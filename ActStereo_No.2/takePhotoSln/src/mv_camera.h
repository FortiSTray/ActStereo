#define _CRT_SECURE_NO_WARNINGS

#ifndef _MV_CAMERA_H
#define _MV_CAMERA_H

#define IMSHOW_DEBUG_IMAGE

#include <iostream>
#include "windows.h"
#include "process.h"
#include "CameraApi.h"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


class MindVisionCamera
{
public:

	MindVisionCamera() {}
	explicit MindVisionCamera(char* camName1, char* camName2);
	MindVisionCamera(const MindVisionCamera&) = delete;
	MindVisionCamera& operator=(const MindVisionCamera&) = delete;
	~MindVisionCamera();

	void syncRead()
	{
		WaitForSingleObject(m_hSemaphoreLW, INFINITE);
		WaitForSingleObject(m_hSemaphoreRW, INFINITE);
		srcImageLeft = Mat(Size(m_sFrInfo[0].iWidth, m_sFrInfo[0].iHeight), CV_8UC3, m_pFrameBuffer[0]);
		srcImageRight = Mat(Size(m_sFrInfo[1].iWidth, m_sFrInfo[1].iHeight), CV_8UC3, m_pFrameBuffer[1]);
		
	}

	void releaseReadSemaphore()
	{
		ReleaseSemaphore(m_hSemaphoreLR, 1, &m_lSemaphoreCount);
		ReleaseSemaphore(m_hSemaphoreRR, 1, &m_lSemaphoreCount);
	}

	Mat getSrcImageLeft() { return srcImageLeft; }
	Mat getSrcImageRight() { return srcImageRight; }

public:

	CameraHandle		m_hCamera[2];				//双相机比赛模式相机句柄组
	UINT				m_threadID[2];				//图像抓取线程的ID
	HANDLE				m_hFrameGetThread[2];	    //图像抓取线程的句柄
	BOOL				m_bExit = FALSE;			//用来通知图像抓取线程结束
	BYTE*			    m_pFrameBuffer[2];			//用于将原始图像数据转换为RGB的缓冲区
	tSdkFrameHead		m_sFrInfo[2];		        //用于保存当前图像帧的帧头信息

	HANDLE	m_hSemaphoreLR;
	HANDLE	m_hSemaphoreLW;
	HANDLE	m_hSemaphoreRR;
	HANDLE	m_hSemaphoreRW;
	long	m_lSemaphoreCount;

private:

	Mat srcImageLeft;
	Mat srcImageRight;

};

#endif //_MV_CAMERA_H