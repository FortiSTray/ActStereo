#include <iostream>
#include <opencv2/opencv.hpp>
#include "camera_module.h"
#include "stereo_camera.h"
#include "timer.h"

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	StereoCamera stStereoNo_1(2, CAMERA_ARGS_LEFT, 1, CAMERA_ARGS_RIGHT);

	while (true)
	{
		stStereoNo_1.updateFrame();

		//if push down Esc, kill the progress
		if (waitKey(1) == 0x1B)
		{
			break;
		}
	}

	return 0;
}