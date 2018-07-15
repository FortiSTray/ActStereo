#ifndef _CAMERA_MODULE_H
#define _CAMERA_MODULE_H

#define INTRINSIC_MATRIX_L { 1.1721e+03f, -7.3672f  , 933.3299f, \
							 0.0f     , 1.1827e+03f, 608.4042f, \
							 0.0f     , 0.0f      , 1.0f      }

#define DISTORTION_COEFF_L { -0.3828, 0.1174, -8.2614e-04, 0.0011 }

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class CameraModule
{
public:

	CameraModule() {}
	explicit CameraModule(char cameraId);
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

	Mat srcImage;
	Mat tempImage;
	Mat testImage;

	double intrinsicMatrixArray[3][3] = INTRINSIC_MATRIX_L;
	double distortionCoeffArray[5] = DISTORTION_COEFF_L;
	Mat intrinsicMatrix;
	Mat distortionCoeff;

	Rect ROIRect;
	
};


#endif//_CAMERA_MODULE_H