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

	CameraHandle		m_hCamera[2];				//˫�������ģʽ��������
	UINT				m_threadID[2];				//ͼ��ץȡ�̵߳�ID
	HANDLE				m_hFrameGetThread[2];	    //ͼ��ץȡ�̵߳ľ��
	BOOL				m_bExit = FALSE;			//����֪ͨͼ��ץȡ�߳̽���
	BYTE*			    m_pFrameBuffer[2];			//���ڽ�ԭʼͼ������ת��ΪRGB�Ļ�����
	tSdkFrameHead		m_sFrInfo[2];		        //���ڱ��浱ǰͼ��֡��֡ͷ��Ϣ

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