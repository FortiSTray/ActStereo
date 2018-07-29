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
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[0], pbyBuffer, pThis->m_pFrameBuffer[0], &sFrameInfo);

				//复制帧头信息
				memcpy(&pThis->m_sFrInfo[0], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->hSemaphoreLW, 1, &pThis->semaphoreCount);

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
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				frameGetStatus = CameraImageProcess(pThis->m_hCamera[1], pbyBuffer, pThis->m_pFrameBuffer[1], &sFrameInfo);

				//复制帧头信息
				memcpy(&pThis->m_sFrInfo[1], &sFrameInfo, sizeof(tSdkFrameHead));
			}
			ReleaseSemaphore(pThis->hSemaphoreRW, 1, &pThis->semaphoreCount);

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
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

	//创建信号量
	hSemaphoreLR = CreateSemaphore(NULL, 1, 1, "semaphoreLeftRead");
	hSemaphoreLW = CreateSemaphore(NULL, 0, 1, "semaphoreLeftWrite");
	hSemaphoreRR = CreateSemaphore(NULL, 1, 1, "semaphoreRightRead");
	hSemaphoreRW = CreateSemaphore(NULL, 0, 1, "semaphoreRightWrite");

	if (CameraEnumerateDeviceEx() == 0)
	{
		printf("W...Where's my camera? QAQ\n");
		exit(-1);
	}

	//用相机名初始化近焦和远焦相机，并给两个相机句柄赋值
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
	CameraCreateSettingPage(m_hCamera[0], NULL, camName1, NULL, NULL, 0);
	CameraCreateSettingPage(m_hCamera[1], NULL, camName2, NULL, NULL, 0);

	//开启摄像头数据获取线程
	m_hFrameGetThread[0] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadLeft, (PVOID)this, 0, &m_threadID[0]);
	m_hFrameGetThread[1] = (HANDLE)_beginthreadex(NULL, 0, &frameGetThreadRight, (PVOID)this, 0, &m_threadID[1]);

	//进入工作模式开始采集图像
	CameraPlay(m_hCamera[0]);
	CameraPlay(m_hCamera[1]);

#ifdef IMSHOW_DEBUG_IMAGE

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCamera[0], TRUE);
	CameraShowSettingPage(m_hCamera[1], TRUE);

#else

	//TRUE显示相机配置界面。FALSE则隐藏。
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