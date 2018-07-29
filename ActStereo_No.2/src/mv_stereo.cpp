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
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[0], pbyBuffer, pThis->m_pFrameBuffer[0], &sFrameInfo);

				//复制帧头信息
				memcpy(&pThis->m_sFrInfo[0], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->m_hSemaphoreLW, 1, &pThis->m_lSemaphoreCount);

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
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
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[1], pbyBuffer, pThis->m_pFrameBuffer[1], &sFrameInfo);

				//复制帧头信息
				memcpy(&pThis->m_sFrInfo[1], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->m_hSemaphoreRW, 1, &pThis->m_lSemaphoreCount);

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
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

	//创建信号量
	m_hSemaphoreLR = CreateSemaphore(NULL, 1, 1, "semaphoreLeftRead");
	m_hSemaphoreLW = CreateSemaphore(NULL, 0, 1, "semaphoreLeftWrite");
	m_hSemaphoreRR = CreateSemaphore(NULL, 1, 1, "semaphoreRightRead");
	m_hSemaphoreRW = CreateSemaphore(NULL, 0, 1, "semaphoreRightWrite");

	if (CameraEnumerateDeviceEx() == 0)
	{
		printf("W...Where's my camera? QAQ\n");
		exit(-1);
	}

	//用相机名初始化近焦和远焦相机，并给两个相机句柄赋值
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

	//根据安装方式设置源图像镜像操作
	CameraSetMirror(m_hCamera[0], 0, FALSE);
	CameraSetMirror(m_hCamera[0], 1, TRUE);
	CameraSetMirror(m_hCamera[1], 0, FALSE);
	CameraSetMirror(m_hCamera[1], 1, TRUE);

	//设置对比度
	CameraSetContrast(m_hCamera[0], 100);
	CameraSetContrast(m_hCamera[1], 100);

	//获得相机的特性描述，两个相机型号及硬件设置完全相同，所以只需要获取一台相机的信息
	CameraGetCapability(m_hCamera[0], &sCameraInfo);

	m_pFrameBuffer[0] = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iHeightMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);
	m_pFrameBuffer[1] = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iHeightMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);

	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCamera[0], CAMERA_MEDIA_TYPE_RGB8);
		CameraSetIspOutFormat(m_hCamera[1], CAMERA_MEDIA_TYPE_RGB8);
	}

	//通知SDK内部建该相机的属性页面
	CameraCreateSettingPage(m_hCamera[0], NULL, camNameL, NULL, NULL, 0);
	CameraCreateSettingPage(m_hCamera[1], NULL, camNameR, NULL, NULL, 0);

	//开启摄像头数据获取线程
	m_hFrameGetThread[0] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadLeft , (PVOID)this, 0, &m_uThreadID[0]);
	m_hFrameGetThread[1] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadRight, (PVOID)this, 0, &m_uThreadID[1]);

	//进入工作模式开始采集图像
	CameraPlay(m_hCamera[0]);
	CameraPlay(m_hCamera[1]);

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCamera[0], TRUE);
	CameraShowSettingPage(m_hCamera[1], TRUE);

	//相机校正矩阵赋值
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

	//立体校正
	stereoRectify(cameraL.intrinsicMatrix, cameraL.distortionCoeff, cameraR.intrinsicMatrix, cameraR.distortionCoeff,
		Size(cameraL.cols, cameraL.rows), rotationMatrix, translationMatrix, RLeft, RRight, PLeft, PRight, Q,
		CALIB_ZERO_DISPARITY, 0, Size(cameraL.cols, cameraL.rows), &validROILeft, &validROIRight);

	//获取左右相机的映射矩阵
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