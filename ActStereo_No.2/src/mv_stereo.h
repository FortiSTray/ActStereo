#define _CRT_SECURE_NO_WARNINGS

#ifndef _MV_STEREO_H
#define _MV_STEREO_H

//Source Image Size
#define SRC_ROWS			480
#define SRC_COLS			640

//Arguments of left camera
#define CAMERA_ARGS_LEFT  { 1046.833f, 1045.123f,    /*Focal Length*/ \
							359.406f, 263.913f,    /*Principal Point*/ \
							0.665f,    /*Skew*/ \
							-0.116f, 0.435f,   /*Radial Distortion*/ \
							0.0012f, 0.0006185f   /*Tangential Distortion*/ }

//Arguments of right camera
#define CAMERA_ARGS_RIGHT { 1131.1036f, 1128.697f,    /*Focal Length*/ \
							319.279f, 284.474f,    /*Principal Point*/ \
							0.8227f,    /*Skew*/ \
							-0.1526f, 0.0391f,   /*Radial Distortion*/ \
							0.000568f, 0.0004046f   /*Tangential Distortion*/ }

//Rotation Matrix(Need Transposing) and Translation Matrix
#define ROTATION_MATRIX		 (Mat_<double>(3, 3) << 0.9991f, 0.0063f, 0.0421f, \
													0.0046f, 0.9999f, -0.0109f, \
													-0.0421f, 0.0111f, 1.0000f)
#define TRANSLATION_MATRIX	 (Mat_<double>(3, 1) << -698.784f, -10.2996f, -5.9408f)

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

	Mat srcImageLeft;
	Mat srcImageRight;

	Mat frameLeft;
	Mat frameRight;
};

#endif //_MV_STEREO_H