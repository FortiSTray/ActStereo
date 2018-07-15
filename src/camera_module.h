#ifndef _CAMERA_MODULE_H
#define _CAMERA_MODULE_H

//Arguments of left camera
#define CAMERA_ARGS_LEFT  { 1.1721e+03f, 1.1827e+03f,    /*fx fy*/ \
							933.3299f, 608.4042f,    /*cx cy*/ \
							-7.3672f,    /*skew*/ \
							-0.3828f, 0.1174f,   /*k1 k2*/ \
							-8.2614e-04f, 0.0011f   /*p1 p2*/ }

//Arguments of right camera
#define CAMERA_ARGS_RIGHT { 0.0f, 0.0f, \
							0.0f, 0.0f, \
							0.0f, \
							0.0f, 0.0f, \
							0.0f, 0.0f, }

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