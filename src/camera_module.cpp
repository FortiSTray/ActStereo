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

		cameraCapture >> srcImage;
		rows = srcImage.rows;
		cols = srcImage.cols;
		ROIRect = { 0, 0, cols, rows };

		//���У������ֵ
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

		cameraCapture >> srcImage;
		rows = srcImage.rows;
		cols = srcImage.cols;
		ROIRect = { 0, 0, cols, rows };

		//���У������ֵ
		args = cameraArgs;
		intrinsicMatrix = (Mat_<float>(3, 3) << args.fx, args.skew, args.cx, \
			0.0f, args.fy, args.cy, \
			0.0f, 0.0f, 1.0f);
		distortionCoeff = (Mat_<float>(5, 1) << args.k1, args.k2, args.p1, args.p2, 0.0f);
	}
}

void CameraModule::updateFrame()
{
	cameraCapture >> srcImage;
	frame = Mat(srcImage.rows, srcImage.cols, CV_8UC3);

	for (auto i = 0; i < srcImage.rows; i++)
		for (auto j = 0; j < srcImage.cols; j++)
		{
			frame.ptr<Vec3b>(i)[j] = srcImage.ptr<Vec3b>(srcImage.rows - i - 1)[srcImage.cols - j - 1];
		}

	frame = frame(ROIRect).clone();
}