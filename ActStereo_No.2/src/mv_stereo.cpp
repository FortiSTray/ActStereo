#include "mv_stereo.h"

UINT WINAPI frameGetThreadLeft(LPVOID lpParam)
{
	MVStereo*				pThis = (MVStereo*)lpParam;
	BYTE*					pbyBuffer;
	tSdkFrameHead			sFrameInfo;
	CameraSdkStatus			frameGetStatus = CAMERA_STATUS_FAILED;

	while (!pThis->m_bExit)
	{
		if (CameraGetImageBuffer(pThis->m_hCamera[0], &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			WaitForSingleObject(pThis->m_hSemaphoreLR, INFINITE);
			{
				//����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[0], pbyBuffer, pThis->m_pFrameBuffer[0], &sFrameInfo);

				//����֡ͷ��Ϣ
				memcpy(&pThis->m_sFrInfo[0], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->m_hSemaphoreLW, 1, &pThis->m_lSemaphoreCount);

			//�ڳɹ�����CameraGetImageBuffer�󣬱������CameraReleaseImageBuffer���ͷŻ�õ�buffer��
			//�����ٴε���CameraGetImageBufferʱ�����򽫱�����ֱ�������߳��е���CameraReleaseImageBuffer���ͷ���buffer
			CameraReleaseImageBuffer(pThis->m_hCamera[0], pbyBuffer);
		}
	}

	_endthreadex(0);
	return 0;
}

UINT WINAPI frameGetThreadRight(LPVOID lpParam)
{
	MVStereo*				pThis = (MVStereo*)lpParam;
	BYTE*					pbyBuffer;
	tSdkFrameHead			sFrameInfo;
	CameraSdkStatus			frameGetStatus = CAMERA_STATUS_FAILED;

	while (!pThis->m_bExit)
	{
		if (CameraGetImageBuffer(pThis->m_hCamera[1], &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			WaitForSingleObject(pThis->m_hSemaphoreRR, INFINITE);
			{
				//����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[1], pbyBuffer, pThis->m_pFrameBuffer[1], &sFrameInfo);

				//����֡ͷ��Ϣ
				memcpy(&pThis->m_sFrInfo[1], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->m_hSemaphoreRW, 1, &pThis->m_lSemaphoreCount);

			//�ڳɹ�����CameraGetImageBuffer�󣬱������CameraReleaseImageBuffer���ͷŻ�õ�buffer��
			//�����ٴε���CameraGetImageBufferʱ�����򽫱�����ֱ�������߳��е���CameraReleaseImageBuffer���ͷ���buffer
			CameraReleaseImageBuffer(pThis->m_hCamera[1], pbyBuffer);
		}
	}

	_endthreadex(0);
	return 0;
}

MVStereo::MVStereo(char* camNameL, CameraArguments argsL, char* camNameR, CameraArguments argsR)
{
	CameraSdkStatus cameraInitStatus;
	tSdkCameraCapbility sCameraInfo;

	//�����ź���
	m_hSemaphoreLR = CreateSemaphore(NULL, 1, 1, "semaphoreLeftRead");
	m_hSemaphoreLW = CreateSemaphore(NULL, 0, 1, "semaphoreLeftWrite");
	m_hSemaphoreRR = CreateSemaphore(NULL, 1, 1, "semaphoreRightRead");
	m_hSemaphoreRW = CreateSemaphore(NULL, 0, 1, "semaphoreRightWrite");

	if (CameraEnumerateDeviceEx() == 0)
	{
		printf("W...Where's my camera? QAQ\n");
		exit(-1);
	}

	//���������ʼ��������Զ�����������������������ֵ
	if ((cameraInitStatus = CameraInitEx2(camNameL, &m_hCamera[0])) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf(msg, "Failed to init %s! Error code is %d\n", camNameL, cameraInitStatus);
		printf(msg);
		printf(CameraGetErrorString(cameraInitStatus));
		exit(-1);
	}
	if ((cameraInitStatus = CameraInitEx2(camNameR, &m_hCamera[1])) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf(msg, "Failed to init %s! Error code is %d\n", camNameR, cameraInitStatus);
		printf(msg);
		printf(CameraGetErrorString(cameraInitStatus));
		exit(-1);
	}

	//���ݰ�װ��ʽ����Դͼ�������
	CameraSetMirror(m_hCamera[0], 0, FALSE);
	CameraSetMirror(m_hCamera[0], 1, TRUE);
	CameraSetMirror(m_hCamera[1], 0, FALSE);
	CameraSetMirror(m_hCamera[1], 1, TRUE);

	//���öԱȶ�
	CameraSetContrast(m_hCamera[0], 100);
	CameraSetContrast(m_hCamera[1], 100);

	//��������������������������ͺż�Ӳ��������ȫ��ͬ������ֻ��Ҫ��ȡһ̨�������Ϣ
	CameraGetCapability(m_hCamera[0], &sCameraInfo);

	m_pFrameBuffer[0] = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iHeightMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);
	m_pFrameBuffer[1] = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iHeightMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);

	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCamera[0], CAMERA_MEDIA_TYPE_RGB8);
		CameraSetIspOutFormat(m_hCamera[1], CAMERA_MEDIA_TYPE_RGB8);
	}

	//֪ͨSDK�ڲ��������������ҳ��
	CameraCreateSettingPage(m_hCamera[0], NULL, camNameL, NULL, NULL, 0);
	CameraCreateSettingPage(m_hCamera[1], NULL, camNameR, NULL, NULL, 0);

	//��������ͷ���ݻ�ȡ�߳�
	m_hFrameGetThread[0] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadLeft , (PVOID)this, 0, &m_uThreadID[0]);
	m_hFrameGetThread[1] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadRight, (PVOID)this, 0, &m_uThreadID[1]);

	//���빤��ģʽ��ʼ�ɼ�ͼ��
	CameraPlay(m_hCamera[0]);
	CameraPlay(m_hCamera[1]);

	//TRUE��ʾ������ý��档FALSE�����ء�
	CameraShowSettingPage(m_hCamera[0], TRUE);
	CameraShowSettingPage(m_hCamera[1], TRUE);

	//���У������ֵ
	cameraL.intrinsicMatrix = (Mat_<float>(3, 3) << argsL.fx, argsL.skew, argsL.cx, \
													0.0f    , argsL.fy  , argsL.cy, \
													0.0f    , 0.0f      , 1.0f      );
	cameraR.intrinsicMatrix = (Mat_<float>(3, 3) << argsR.fx, argsR.skew, argsR.cx, \
													0.0f    , argsR.fy  , argsR.cy, \
													0.0f    , 0.0f      , 1.0f      );
	cameraL.distortionCoeff = (Mat_<float>(5, 1) << argsL.k1, argsL.k2, argsL.p1, argsL.p2, 0.0f);
	cameraR.distortionCoeff = (Mat_<float>(5, 1) << argsR.k1, argsR.k2, argsR.p1, argsR.p2, 0.0f);

	cameraL.rows = SRC_ROWS;
	cameraL.cols = SRC_COLS;
}

void MVStereo::stereoInit()
{
	rotationMatrix = ROTATION_MATRIX;
	translationMatrix = TRANSLATION_MATRIX;

	//����У��
	stereoRectify(cameraL.intrinsicMatrix, cameraL.distortionCoeff, cameraR.intrinsicMatrix, cameraR.distortionCoeff,
		Size(cameraL.cols, cameraL.rows), rotationMatrix, translationMatrix, RLeft, RRight, PLeft, PRight, Q,
		CALIB_ZERO_DISPARITY, 0, Size(cameraL.cols, cameraL.rows), &validROILeft, &validROIRight);

	//��ȡ���������ӳ�����
	initUndistortRectifyMap(cameraL.intrinsicMatrix, cameraL.distortionCoeff, RLeft, PLeft,
		Size(cameraL.cols, cameraL.rows), CV_32FC1, mapLx, mapLy);

	initUndistortRectifyMap(cameraR.intrinsicMatrix, cameraR.distortionCoeff, RRight, PRight,
		Size(cameraL.cols, cameraL.rows), CV_32FC1, mapRx, mapRy);
}

void MVStereo::syncUpdate()
{
	WaitForSingleObject(m_hSemaphoreLW, INFINITE);
	WaitForSingleObject(m_hSemaphoreRW, INFINITE);

	srcImageLeft = Mat(Size(m_sFrInfo[0].iWidth, m_sFrInfo[0].iHeight), CV_8UC3, m_pFrameBuffer[0]);
	srcImageRight = Mat(Size(m_sFrInfo[1].iWidth, m_sFrInfo[1].iHeight), CV_8UC3, m_pFrameBuffer[1]);

	frameLeft = srcImageLeft.clone();
	frameLeft = srcImageRight.clone();

	remap(srcImageLeft , frameLeft , mapLx, mapLy, INTER_LINEAR);
	remap(srcImageRight, frameRight, mapRx, mapRy, INTER_LINEAR);

	for (int i = 1; i < 10; i++)
	{
		line(frameLeft, cvPoint(0, 48 * i), cvPoint(640 - 1, 48 * i), cvScalar(0, 255, 0));
		line(frameRight, cvPoint(0, 48 * i), cvPoint(640 - 1, 48 * i), cvScalar(0, 255, 0));
	}

	imshow("Left", frameLeft);
	imshow("Right", frameRight);

	ReleaseSemaphore(m_hSemaphoreLR, 1, &m_lSemaphoreCount);
	ReleaseSemaphore(m_hSemaphoreRR, 1, &m_lSemaphoreCount);
}