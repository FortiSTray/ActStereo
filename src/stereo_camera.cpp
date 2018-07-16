#include "stereo_camera.h"

StereoCamera::StereoCamera(char idLeft, CameraArguments argsLeft, char idRight, CameraArguments argsRight)
{
	cameraArgsLeft = CAMERA_ARGS_LEFT;
	cameraArgsRight = CAMERA_ARGS_RIGHT;

	cameraLeft(2, cameraArgsLeft);
	cameraRight(1, cameraArgsRight);

	rotationMatrix = ROTATION_MATRIX;
	translationMatrix = TRANSLATION_MATRIX;
	
	stereoRectify(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, cameraRight.intrinsicMatrix, cameraRight.distortionCoeff,
		Size(640, 480), rotationMatrix, translationMatrix, rotationMatrixLeft, rotationMatrixRight, newMatrixLeft, newMatrixRight, 
		reprojectionMatrix, CALIB_ZERO_DISPARITY, 0, Size(640, 480), &validROIL, &validROIR);

	initUndistortRectifyMap(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, rotationMatrixLeft, newMatrixLeft, 
		Size(640, 480), CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraRight.intrinsicMatrix, cameraRight.distortionCoeff, rotationMatrixRight, newMatrixRight, 
		Size(640, 480), CV_32FC1, mapRx, mapRy);
}

void StereoCamera::updateFrame()
{
	//test
	int blockSize = 5, uniquenessRatio = 10, numDisparities = 5;
	Ptr<StereoBM> bm = StereoBM::create(16, 9);
	Mat frameGrayL;
	Mat frameGrayR;
	Mat xyz;              //三维坐标

	cameraLeft.updateFrame();
	cameraRight.updateFrame();

	frameLeft = cameraLeft.getFrame().clone();
	frameLeft = cameraRight.getFrame().clone();

	remap(cameraLeft.getFrame(), frameLeft, mapLx, mapLy, INTER_LINEAR);
	remap(cameraRight.getFrame(), frameRight, mapRx, mapRy, INTER_LINEAR);

	for (int i = 1; i < 10; i++)
	{
		line(frameLeft , cvPoint(0, 48 * i), cvPoint(640 - 1, 48 * i), cvScalar(0, 255, 0));
		line(frameRight, cvPoint(0, 48 * i), cvPoint(640 - 1, 48 * i), cvScalar(0, 255, 0));
	}

	cvtColor(frameLeft, frameGrayL, CV_BGR2GRAY);
	cvtColor(frameRight, frameGrayR, CV_BGR2GRAY);
	bm->setBlockSize(2 * blockSize + 5);     //SAD窗口大小，5~21之间为宜
	bm->setROI1(validROIL);
	bm->setROI2(validROIR);
	bm->setPreFilterCap(31);
	bm->setMinDisparity(0);  //最小视差，默认值为0, 可以是负值，int型
	bm->setNumDisparities(numDisparities * 16 + 16);//视差窗口，即最大视差值与最小视差值之差,窗口大小必须是16的整数倍，int型
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(uniquenessRatio);//uniquenessRatio主要可以防止误匹配
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(-1);
	Mat disp, disp8;
	bm->compute(frameGrayL, frameGrayR, disp);//输入图像必须为灰度图
	disp.convertTo(disp8, CV_8U, 255.0f / ((numDisparities * 16 + 16) * 16.));//计算出的视差是CV_16S格式
	reprojectImageTo3D(disp, xyz, reprojectionMatrix, true); //在实际求距离时，ReprojectTo3D出来的X / W, Y / W, Z / W都要乘以16(也就是W除以16)，才能得到正确的三维坐标信息。
	xyz = xyz * 16;
	imshow("disparity", disp8);
}