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
	Mat xyz;              //��ά����

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
	bm->setBlockSize(2 * blockSize + 5);     //SAD���ڴ�С��5~21֮��Ϊ��
	bm->setROI1(validROIL);
	bm->setROI2(validROIR);
	bm->setPreFilterCap(31);
	bm->setMinDisparity(0);  //��С�ӲĬ��ֵΪ0, �����Ǹ�ֵ��int��
	bm->setNumDisparities(numDisparities * 16 + 16);//�Ӳ�ڣ�������Ӳ�ֵ����С�Ӳ�ֵ֮��,���ڴ�С������16����������int��
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(uniquenessRatio);//uniquenessRatio��Ҫ���Է�ֹ��ƥ��
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(-1);
	Mat disp, disp8;
	bm->compute(frameGrayL, frameGrayR, disp);//����ͼ�����Ϊ�Ҷ�ͼ
	disp.convertTo(disp8, CV_8U, 255.0f / ((numDisparities * 16 + 16) * 16.));//��������Ӳ���CV_16S��ʽ
	reprojectImageTo3D(disp, xyz, reprojectionMatrix, true); //��ʵ�������ʱ��ReprojectTo3D������X / W, Y / W, Z / W��Ҫ����16(Ҳ����W����16)�����ܵõ���ȷ����ά������Ϣ��
	xyz = xyz * 16;
	imshow("disparity", disp8);
}