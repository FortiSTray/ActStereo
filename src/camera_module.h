#ifndef _CAMERA_MODULE_H
#define _CAMERA_MODULE_H

//Arguments of left camera
#define CAMERA_ARGS_LEFT  { 527.4714f, 529.0205f,    /*Focal Length*/ \
							313.2706f, 267.7183f,    /*Principal Point*/ \
							-1.7437f,    /*Skew*/ \
							-0.4203f, 0.1865f,   /*Radial Distortion*/ \
							0.0010f, -2.9267e-05f   /*Tangential Distortion*/ }

//Arguments of right camera
#define CAMERA_ARGS_RIGHT { 533.6941f, 535.3964f,    /*Focal Length*/ \
							345.5350f, 244.8390f,    /*Principal Point*/ \
							-2.0714f,    /*Skew*/ \
							-0.4248f, 0.1999f,   /*Radial Distortion*/ \
							2.4476e-04f, 2.6631e-04f   /*Tangential Distortion*/ }

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

	void setROIRect(Rect& r)
	{
		ROIRect = r;
		ROIRows = r.height;
		ROICols = r.width;
	}

	bool isOpened() { return !cameraCapture.isOpened(); }

	void updateFrame();

	Mat getSrcImage() { return srcImage; }

	int rows;
	int cols;

	int ROIRows;
	int ROICols;

private:

	VideoCapture cameraCapture;

	CameraArguments args;

	Mat srcImage;
	Mat tempImage;
	Mat testImage;

	Mat intrinsicMatrix;
	Mat distortionCoeff;

	Rect ROIRect;
	
};


#endif//_CAMERA_MODULE_H