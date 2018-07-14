#ifndef _CAMERA_MODULE_H
#define _CAMERA_MODULE_H

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

	int rows = 0;
	int cols = 0;

	int ROIRows = 0;
	int ROICols = 0;

private:

	VideoCapture cameraCapture;

	Mat srcImage;
	Mat tempImage;
	Mat testImage;

	Rect ROIRect;
	
};

#endif//_CAMERA_MODULE_H