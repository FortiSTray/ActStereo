#include "stereo_camera.h"

StereoCamera::StereoCamera(char idLeft, CameraArguments argsLeft, char idRight, CameraArguments argsRight)
{
	cameraArgsLeft = CAMERA_ARGS_LEFT;
	cameraArgsRight = CAMERA_ARGS_RIGHT;

	cameraLeft(idLeft, cameraArgsLeft);
	cameraRight(idRight, cameraArgsRight);

	rotationMatrix = ROTATION_MATRIX;
	translationMatrix = TRANSLATION_MATRIX;
	
	//����У��
	stereoRectify(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, cameraRight.intrinsicMatrix, cameraRight.distortionCoeff,
		Size(cameraLeft.cols, cameraLeft.rows), rotationMatrix, translationMatrix, RLeft, RRight, PLeft, PRight, Q, 
		CALIB_ZERO_DISPARITY, 0, Size(cameraLeft.cols, cameraLeft.rows), &validROILeft, &validROIRight);

	//��ȡ���������ӳ�����
	initUndistortRectifyMap(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, RLeft, PLeft, 
		Size(cameraLeft.cols, cameraLeft.rows), CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraRight.intrinsicMatrix, cameraRight.distortionCoeff, RRight, PRight, 
		Size(cameraLeft.cols, cameraLeft.rows), CV_32FC1, mapRx, mapRy);
}

void StereoCamera::updateFrame()
{
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
}

Vec4d StereoCamera::simulatedLocating(int yLeft, int xLeft, int yRight, int xRight)
{
	circle(frameLeft, Point(xLeft, yLeft), 3, Scalar(255, 0, 255), 2);
	circle(frameRight, Point(xRight, yRight), 3, Scalar(255, 0, 255), 2);

	//x, y, z and distance
	Vec4d worldPoint;

	Mat reprojectMatrix;
	Q.convertTo(reprojectMatrix, CV_64FC1);
	Mat srcVector = (Mat_<double>(4, 1) << xLeft, yLeft, xLeft - xRight, 1.0f);
	Mat dstVector = Q * srcVector;

	worldPoint[0] = dstVector.ptr<double>(0)[0] / dstVector.ptr<double>(3)[0];
	worldPoint[1] = dstVector.ptr<double>(1)[0] / dstVector.ptr<double>(3)[0];
	worldPoint[2] = dstVector.ptr<double>(2)[0] / dstVector.ptr<double>(3)[0];
	worldPoint[3] = sqrt(pow(worldPoint[0], 2) + pow(worldPoint[1], 2) + pow(worldPoint[2], 2));

	return worldPoint;
}

//OpenCV���ú�����ȡ���ͼ
void StereoCamera::outputDepthImage()
{
	int blockSize = 5, uniquenessRatio = 10, numDisparities = 5;
	Ptr<StereoBM> bm = StereoBM::create(16, 9);
	Mat frameGrayL;
	Mat frameGrayR;
	Mat xyz;              //��ά����
	
	cvtColor(frameLeft, frameGrayL, CV_BGR2GRAY);
	cvtColor(frameRight, frameGrayR, CV_BGR2GRAY);
	bm->setBlockSize(2 * blockSize + 5);     //SAD���ڴ�С��5~21֮��Ϊ��
	bm->setROI1(validROILeft);
	bm->setROI2(validROIRight);
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
	reprojectImageTo3D(disp, xyz, Q, true); //��ʵ�������ʱ��ReprojectTo3D������X / W, Y / W, Z / W��Ҫ����16(Ҳ����W����16)�����ܵõ���ȷ����ά������Ϣ��
	xyz = xyz * 16;
	imshow("disparity", disp8);
}