#include <iostream>
#include "opencv2/opencv.hpp"
#include "shuttlecock_recognizer.h"
#include "timer.h"

using namespace std;
using namespace cv;

#define DETECTION	0
#define TRACKING	1

//-- Global Status
bool status = 0;
int keyStatus;

//-- Timer
Timer periodTimer;

int main(int argc, char* argv[])
{
	ShuttlecockRecognizer actStereoNo_1("CameraLeft", CAMERA_ARGS_LEFT, "CameraRight", CAMERA_ARGS_RIGHT);
	actStereoNo_1.stereoInit();

	while (!actStereoNo_1.m_bExit)
	{
		actStereoNo_1.syncUpdate();

		imshow("LeftSrc", actStereoNo_1.getFrameLeft());
		imshow("RightSrc", actStereoNo_1.getFrameRight());

		actStereoNo_1.backgroundSubtract();
		actStereoNo_1.preprocess();

		//-- Detection & Tracking
		switch (status)
		{
		case DETECTION:
			if (actStereoNo_1.shuttlecockDetection(Size(7, 7), 1, 47) == true) { status = TRACKING; }
			cout << "Detecting" << endl;

			break;

		case TRACKING:
			if (actStereoNo_1.shuttlecockTracking() == false) { status = DETECTION; }
			cout << "Tracking" << endl;
			
			break;
		}

		//-- If key Esc is pushed down, kill the progress
		keyStatus = waitKey(1);
		if (keyStatus == 0x1B)
		{
			break;
		}
	}

	return 0;
}