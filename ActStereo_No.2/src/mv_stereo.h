#define _CRT_SECURE_NO_WARNINGS

#ifndef _MV_STEREO_H
#define _MV_STEREO_H

//Source Image Size
#define SRC_ROWS			480
#define SRC_COLS			640

//Arguments of left camera
#define CAMERA_ARGS_LEFT  { 1048.96647747845f, 1047.34210680108f,    /*Focal Length*/ \
							358.316505355369f, 250.414596319905f,    /*Principal Point*/ \
							0.533969287533816f,    /*Skew*/ \
							-0.0870216932789717f, 0.162170702067133f,   /*Radial Distortion*/ \
							-0.00127355847135182f, -0.000335534423369419f   /*Tangential Distortion*/ }

//Arguments of right camera
#define CAMERA_ARGS_RIGHT { 1135.55569966515f, 1133.61162389769f,    /*Focal Length*/ \
							321.278694069469f, 271.861585817153f,    /*Principal Point*/ \
							0.395850223483792f,    /*Skew*/ \
							-0.178275203194529f, 0.424986480722085f,   /*Radial Distortion*/ \
							-0.00100639818615331f, 0.000676255973728689f   /*Tangential Distortion*/ }

//Rotation Matrix(Need Transposing) and Translation Matrix
#define ROTATION_MATRIX		 (Mat_<double>(3, 3) << 0.999191628914595f, -0.00823526575398190f, 0.0393480508406796f, \
													0.00858540984152276f, 0.999924966555566f, -0.00873796296076068f, \
													-0.0392731389737629f, 0.00906871858709015f, 0.999187359256679f)
#define TRANSLATION_MATRIX	 (Mat_<double>(3, 1) << -706.344134499443f, -10.1298529199082f, -2.14161643140913f)

//RT Matrix Convert Camera Coordinate to World Coordinate (World = RT * Camera)
#define RT_MATRIX			 (Mat_<double>(3, 4) << 1.0f, 0.0f, 0.0f, 0.0f, \
													0.0f, 1.0f, 0.0f, 0.0f, \
													0.0f, 0.0f, 1.0f, 0.0f)

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

	//Simulated Locating
	Vec4d simulatedLocating(int yLeft, int xLeft, int yRight, int xRight);

	Mat getFrameLeft() { return frameLeft; }
	Mat getFrameRight() { return frameRight; }

public:

	CameraHandle		m_hCamera[2];
	UINT				m_uThreadID[2];
	HANDLE				m_hFrameGetThread[2];
	BYTE*			    m_pFrameBuffer[2];
	tSdkFrameHead		m_sFrInfo[2];
	BOOL				m_bExit = FALSE;

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
	Mat rtMatrix;

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