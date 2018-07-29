#include "mv_camera.h"

UINT WINAPI frameGetThreadLeft(LPVOID lpParam)
{
	MindVisionCamera*		pThis = (MindVisionCamera*)lpParam;
	BYTE*					pbyBuffer;
	tSdkFrameHead			sFrameInfo;
	CameraSdkStatus			frameGetStatus = CAMERA_STATUS_FAILED;

	while (!pThis->m_bExit)
	{
		if (CameraGetImageBuffer(pThis->m_hCamera[0], &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			WaitForSingleObject(pThis->hSemaphoreLR, INFINITE);
			{
				//����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[0], pbyBuffer, pThis->m_pFrameBuffer[0], &sFrameInfo);

				//����֡ͷ��Ϣ
				memcpy(&pThis->m_sFrInfo[0], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->hSemaphoreLW, 1, &pThis->semaphoreCount);

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
	MindVisionCamera*		pThis = (MindVisionCamera*)lpParam;
	BYTE*					pbyBuffer;
	tSdkFrameHead			sFrameInfo;
	CameraSdkStatus			frameGetStatus = CAMERA_STATUS_FAILED;

	while (!pThis->m_bExit)
	{
		if (CameraGetImageBuffer(pThis->m_hCamera[1], &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			WaitForSingleObject(pThis->hSemaphoreRR, INFINITE);
			{
				//����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[1], pbyBuffer, pThis->m_pFrameBuffer[1], &sFrameInfo);

				//����֡ͷ��Ϣ
				memcpy(&pThis->m_sFrInfo[1], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->hSemaphoreRW, 1, &pThis->semaphoreCount);

			//�ڳɹ�����CameraGetImageBuffer�󣬱������CameraReleaseImageBuffer���ͷŻ�õ�buffer��
			//�����ٴε���CameraGetImageBufferʱ�����򽫱�����ֱ�������߳��е���CameraReleaseImageBuffer���ͷ���buffer
			CameraReleaseImageBuffer(pThis->m_hCamera[1], pbyBuffer);
		}
	}

	_endthreadex(0);
	return 0;
}

MindVisionCamera::MindVisionCamera(char* camName1, char* camName2)
{
	CameraSdkStatus cameraInitStatus;
	tSdkCameraCapbility sCameraInfo;

	//�����ź���
	hSemaphoreLR = CreateSemaphore(NULL, 1, 1, "semaphoreLeftRead");
	hSemaphoreLW = CreateSemaphore(NULL, 0, 1, "semaphoreLeftWrite");
	hSemaphoreRR = CreateSemaphore(NULL, 1, 1, "semaphoreRightRead");
	hSemaphoreRW = CreateSemaphore(NULL, 0, 1, "semaphoreRightWrite");

	if (CameraEnumerateDeviceEx() == 0)
	{
		printf("W...Where's my camera? QAQ\n");
		exit(-1);
	}

	//���������ʼ��������Զ�����������������������ֵ
	if ((cameraInitStatus = CameraInitEx2(camName1, &m_hCamera[0])) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf(msg, "Failed to init %s! Error code is %d\n", camName1, cameraInitStatus);
		printf(msg);
		printf(CameraGetErrorString(cameraInitStatus));
		exit(-1);
	}
	if ((cameraInitStatus = CameraInitEx2(camName2, &m_hCamera[1])) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf(msg, "Failed to init %s! Error code is %d\n", camName2, cameraInitStatus);
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
	CameraCreateSettingPage(m_hCamera[0], NULL, camName1, NULL, NULL, 0);
	CameraCreateSettingPage(m_hCamera[1], NULL, camName2, NULL, NULL, 0);

	//��������ͷ���ݻ�ȡ�߳�
	m_hFrameGetThread[0] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadLeft, (PVOID)this, 0, &m_threadID[0]);
	m_hFrameGetThread[1] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadRight, (PVOID)this, 0, &m_threadID[1]);

	//���빤��ģʽ��ʼ�ɼ�ͼ��
	CameraPlay(m_hCamera[0]);
	CameraPlay(m_hCamera[1]);

#ifdef IMSHOW_DEBUG_IMAGE

	//TRUE��ʾ������ý��档FALSE�����ء�
	CameraShowSettingPage(m_hCamera[0], TRUE);
	CameraShowSettingPage(m_hCamera[1], TRUE);

#else

	//TRUE��ʾ������ý��档FALSE�����ء�
	CameraShowSettingPage(m_hCamera[0], FALSE);
	CameraShowSettingPage(m_hCamera[1], FALSE);

#endif //IMSHOW_DEBUG_IMAGE

}

MindVisionCamera::~MindVisionCamera()
{
	CameraUnInit(m_hCamera[0]);
	CameraUnInit(m_hCamera[1]);

	CameraAlignFree(m_pFrameBuffer[0]);
	CameraAlignFree(m_pFrameBuffer[1]);
}