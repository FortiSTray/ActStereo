#include "shuttlecock_recognizer.h"

ShuttleRecognizer::ShuttleRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight) :
	MVStereo(camNameL, argsLeft, camNameR, argsRight)
{
	bgModel = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();
}

void ShuttleRecognizer::BackgroundSubtract()
{
	bgModel->apply(getFrameLeft(), fgMaskLeft, -1);
	bgModel->apply(getFrameRight(), fgMaskRight, -1);

	//Filter
	GaussianBlur(fgMaskLeft, fgMaskLeft, Size(11, 11), 3.5, 3.5);
	threshold(fgMaskLeft, fgMaskLeft, 10, 255, THRESH_BINARY);
	GaussianBlur(fgMaskRight, fgMaskRight, Size(11, 11), 3.5, 3.5);
	threshold(fgMaskRight, fgMaskRight, 10, 255, THRESH_BINARY);

	fgImageLeft = Scalar::all(0);
	getFrameLeft().copyTo(fgImageLeft, fgMaskLeft);
	fgImageRight = Scalar::all(0);
	getFrameRight().copyTo(fgImageRight, fgMaskRight);

	imshow("LeftFg", fgImageLeft);
	imshow("RightFg", fgImageRight);
}