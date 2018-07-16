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
		reprojectionMatrix);

	initUndistortRectifyMap(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, rotationMatrixLeft, newMatrixLeft, 
		Size(640, 480), CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraRight.intrinsicMatrix, cameraRight.distortionCoeff, rotationMatrixRight, newMatrixRight, 
		Size(640, 480), CV_32FC1, mapRx, mapRy);
}

void StereoCamera::updateFrame()
{
	cameraLeft.updateFrame();
	cameraRight.updateFrame();

	frameLeft = cameraLeft.getFrame().clone();
	frameLeft = cameraRight.getFrame().clone();

	remap(cameraLeft.getFrame(), frameLeft, mapLx, mapLy, INTER_LINEAR);
	remap(cameraRight.getFrame(), frameRight, mapRx, mapRy, INTER_LINEAR);

	line(frameLeft , cvPoint(0, 48 * 1), cvPoint(640 - 1, 48 * 1), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 2), cvPoint(640 - 1, 48 * 2), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 3), cvPoint(640 - 1, 48 * 3), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 4), cvPoint(640 - 1, 48 * 4), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 5), cvPoint(640 - 1, 48 * 5), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 6), cvPoint(640 - 1, 48 * 6), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 7), cvPoint(640 - 1, 48 * 7), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 8), cvPoint(640 - 1, 48 * 8), cvScalar(255, 0, 0));
	line(frameLeft , cvPoint(0, 48 * 9), cvPoint(640 - 1, 48 * 9), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 1), cvPoint(640 - 1, 48 * 1), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 2), cvPoint(640 - 1, 48 * 2), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 3), cvPoint(640 - 1, 48 * 3), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 4), cvPoint(640 - 1, 48 * 4), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 5), cvPoint(640 - 1, 48 * 5), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 6), cvPoint(640 - 1, 48 * 6), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 7), cvPoint(640 - 1, 48 * 7), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 8), cvPoint(640 - 1, 48 * 8), cvScalar(255, 0, 0));
	line(frameRight, cvPoint(0, 48 * 9), cvPoint(640 - 1, 48 * 9), cvScalar(255, 0, 0));
}