#include <iostream>
#include <opencv2/opencv.hpp>
#include "camera_module.h"
#include "timer.h"

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	CameraModule cameraLeft(2);
	CameraModule cameraRight(1);

	while (true)
	{
		cameraLeft.updateFrame();
		cameraRight.updateFrame();

		imshow("Camera Left", cameraLeft.getSrcImage());
		imshow("Camera Right", cameraRight.getSrcImage());

		//if push down Esc, kill the progress
		if (waitKey(1) == 0x1B)
		{
			break;
		}
	}

	return 0;
}