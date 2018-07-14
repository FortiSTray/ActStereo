#include "camera_module.h"

CameraModule::CameraModule(char cameraId)
{
	cameraCapture.open(cameraId);

	if (!cameraCapture.isOpened())
	{
		cout << "Open Camera failed!" << endl;
		exit(-1);
	}
	else
	{
		cout << "Camera Init Done." << endl;
		
		cameraCapture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
		cameraCapture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		
		cameraCapture.set(CV_CAP_PROP_EXPOSURE, -6.0);

		cameraCapture >> tempImage;
		rows = tempImage.rows;
		cols = tempImage.cols;
		ROIRect = { 0, 0, cols, rows };
	}
}

void CameraModule::updateFrame()
{
	cameraCapture >> tempImage;
	srcImage = tempImage.clone();

	for (auto i = 0; i < tempImage.rows; i++)
		for (auto j = 0; j < tempImage.cols; j++)
		{
			srcImage.ptr<Vec3b>(i)[j] = tempImage.ptr<Vec3b>(tempImage.rows - i - 1)[tempImage.cols - j - 1];
		}

	srcImage = srcImage.clone();
}