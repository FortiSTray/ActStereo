#include "camera_module.h"

CameraModule::CameraModule(char cameraId, CameraArguments cameraArgs)
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

		//相机校正矩阵赋值
		args = cameraArgs;
		intrinsicMatrix = (Mat_<float>(3, 3) << args.fx, args.skew, args.cx, \
												0.0f   , args.fy  , args.cy, \
												0.0f   , 0.0f     , 1.0f    );
		distortionCoeff = (Mat_<float>(5, 1) << args.k1, args.k2, args.p1, args.p2, 0.0f);
	}
}

void CameraModule::operator()(char cameraId, CameraArguments cameraArgs)
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

		//相机校正矩阵赋值
		args = cameraArgs;
		intrinsicMatrix = (Mat_<float>(3, 3) << args.fx, args.skew, args.cx, \
			0.0f, args.fy, args.cy, \
			0.0f, 0.0f, 1.0f);
		distortionCoeff = (Mat_<float>(5, 1) << args.k1, args.k2, args.p1, args.p2, 0.0f);
	}
}

void CameraModule::updateFrame()
{
	cameraCapture >> tempImage;
	/*undistort(tempImage, srcImage, intrinsicMatrix, distortionCoeff);
	tempImage = srcImage.clone();*/
	srcImage = tempImage.clone();

	/*for (auto i = 0; i < tempImage.rows; i++)
		for (auto j = 0; j < tempImage.cols; j++)
		{
			srcImage.ptr<Vec3b>(i)[j] = tempImage.ptr<Vec3b>(tempImage.rows - i - 1)[tempImage.cols - j - 1];
		}

	srcImage = srcImage(ROIRect).clone();*/
}