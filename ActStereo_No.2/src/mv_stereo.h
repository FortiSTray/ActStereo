#define _CRT_SECURE_NO_WARNINGS

#ifndef _MV_STEREO_H
#define _MV_STEREO_H

//Source Image Size
#define SRC_ROWS			480
#define SRC_COLS			640

//Exposure
#define EXPOSURE_VALUE		-6

//Arguments of left camera
#define CAMERA_ARGS_LEFT  { 529.6793f, 530.9351f,    /*Focal Length*/ \
							325.8743f, 215.4840f,    /*Principal Point*/ \
							-1.3523f,    /*Skew*/ \
							-0.4310f, 0.2169f,   /*Radial Distortion*/ \
							-0.0016f, 8.1329e-05f   /*Tangential Distortion*/ }

//Arguments of right camera
#define CAMERA_ARGS_RIGHT { 535.0516f, 536.7354f,    /*Focal Length*/ \
							293.0789f, 239.2839f,    /*Principal Point*/ \
							-1.1520f,    /*Skew*/ \
							-0.4341f, 0.2194f,   /*Radial Distortion*/ \
							-0.0013f, -6.8871e-04f   /*Tangential Distortion*/ }

//Rotation Matrix and Translation Matrix
#define ROTATION_MATRIX		 (Mat_<double>(3, 3) << 1.0000f, -0.0063f, 0.0068f, \
													0.0063f, 1.0000f, 0.0004f, \
													-0.0068f, -0.0004f, 1.0000f)
#define TRANSLATION_MATRIX	 (Mat_<double>(3, 1) << -128.5457f, -0.9355f, 0.7150f)

#include <iostream>
#include "windows.h"
#include "process.h"
#include "CameraApi.h"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

//Struct of Camera Arguments
typedef struct
{
	//inner arguments
	float fx;
	float fy;
	float cx;
	float cy;
	float skew;

	//distortion arguments
	float k1;
	float k2;
	float p1;
	float p2;

} CameraArguments;

//Struct of MindVision Camera
typedef struct
{
	int rows;
	int cols;

	Mat intrinsicMatrix;
	Mat distortionCoeff;

} MVCamera;

class MVStereo
{
public:

	MVStereo() {}
	explicit MVStereo(char* camNameL, CameraArguments argsL, char* camNameR, CameraArguments argsR);
	MVStereo(const MVStereo&) = delete;
	MVStereo& operator=(const MVStereo&) = delete;
	~MVStereo();

	void stereoInit();

	void syncUpdate();

	//ģ���ຯ��
	Vec4d simulatedLocating(int yLeft, int xLeft, int yRight, int xRight);

	Mat getFrameLeft() { return frameLeft; }
	Mat getFrameRight() { return frameRight; }

public:

	CameraHandle		m_hCamera[2];				//˫�������ģʽ��������
	UINT				m_uThreadID[2];				//ͼ��ץȡ�̵߳�ID
	HANDLE				m_hFrameGetThread[2];	    //ͼ��ץȡ�̵߳ľ��
	BYTE*			    m_pFrameBuffer[2];			//���ڽ�ԭʼͼ������ת��ΪRGB�Ļ�����
	tSdkFrameHead		m_sFrInfo[2];		        //���ڱ��浱ǰͼ��֡��֡ͷ��Ϣ
	BOOL				m_bExit = FALSE;			//����֪ͨͼ��ץȡ�߳̽���

	HANDLE	m_hSemaphoreLR;
	HANDLE	m_hSemaphoreLW;
	HANDLE	m_hSemaphoreRR;
	HANDLE	m_hSemaphoreRW;
	long	m_lSemaphoreCount;

private:

	MVCamera cameraL;
	MVCamera cameraR;

	Mat srcImageLeft;
	Mat srcImageRight;

	Mat frameLeft;
	Mat frameRight;

	Mat rotationMatrix;
	Mat translationMatrix;

	Mat RLeft;				//Rotation Matrix of Left Camera
	Mat RRight;				//Rotation Matrix of Right Camera
	Mat PLeft;				//Projection Matrix of Left Camera
	Mat PRight;				//Projection Matrix of Right Camera
	Mat Q;					//Disparity-to-Depth Mapping Matrix
	Rect validROILeft;
	Rect validROIRight;

	Mat mapLx;
	Mat mapLy;
	Mat mapRx;
	Mat mapRy;
};

#endif //_MV_STEREO_H