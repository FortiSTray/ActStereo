#ifndef _CAMERA_MODULE_H
#define _CAMERA_MODULE_H

//Arguments of left camera
#define CAMERA_ARGS_LEFT  { 527.2071f, 529.0991f,    /*Focal Length*/ \
							311.2050f, 266.5736f,    /*Principal Point*/ \
							-1.7056f,    /*Skew*/ \
							-0.4312f, 0.2051f,   /*Radial Distortion*/ \
							0.0011f, -9.2159e-04f   /*Tangential Distortion*/ }

//Arguments of right camera
#define CAMERA_ARGS_RIGHT { 535.5167f, 536.9288f,    /*Focal Length*/ \
							345.9750f, 241.9847f,    /*Principal Point*/ \
							-1.3922f,    /*Skew*/ \
							-0.4289f, 0.2053f,   /*Radial Distortion*/ \
							0.0014f, 9.8522e-05f   /*Tangential Distortion*/ }

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//Struct of camera arguments
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

class CameraModule
{
public:

	CameraModule() {}
	explicit CameraModule(char cameraId, CameraArguments cameraArgs);
	CameraModule(const CameraModule&) = delete;
	CameraModule& operator=(CameraModule&) = delete;
	virtual~CameraModule() {}

	void operator()(char cameraId, CameraArguments cameraArgs);

	void setROIRect(Rect& r)
	{
		ROIRect = r;
		ROIRows = r.height;
		ROICols = r.width;
	}

	bool isOpened() { return !cameraCapture.isOpened(); }

	void updateFrame();

	Mat getFrame() { return frame; }

	int rows;
	int cols;

	int ROIRows;
	int ROICols;

	Mat intrinsicMatrix;
	Mat distortionCoeff;

private:

	VideoCapture cameraCapture;

	CameraArguments args;

	Mat srcImage;
	Mat frame;

	Rect ROIRect;
	
};


#endif//_CAMERA_MODULE_H