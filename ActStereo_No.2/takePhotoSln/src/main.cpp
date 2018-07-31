#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "mv_camera.h"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


int main(int argc, char* argv[])
{
	MindVisionCamera mvCamera("CameraLeft", "CameraRight");

	int keyStatus = 0;
	int counter = 0;
	const int photoNum = 30;
	char fileNameL[25];
	char fileNameR[25];

	Mat srcLeft;
	Mat srcRight;

	namedWindow("Left");
	namedWindow("Right");
	//namedWindow("LeftT");
	//namedWindow("RightT");

	while (!mvCamera.m_bExit)
	{
		mvCamera.syncRead();

		srcLeft = mvCamera.getSrcImageLeft();
		srcRight = mvCamera.getSrcImageRight();


		imshow("Left", srcLeft);
		imshow("Right", srcRight);
		
		//延时及检测按键事件
		keyStatus = waitKey(1);

		if (keyStatus == 0x20)
		{
			counter++;
			sprintf(fileNameL, "./Stereo/Left%d.png", counter);
			sprintf(fileNameR, "./Stereo/Right%d.png", counter);
			imwrite(fileNameL, srcLeft);
			//imshow("TLeft", srcLeft);
			imwrite(fileNameR, srcRight);
			//imshow("TRight", srcRight);

			if (counter == photoNum) { break; }
		}
		else if (keyStatus == 27)
		{
			mvCamera.m_bExit = TRUE;
			break;
		}

		mvCamera.releaseReadSemaphore();
	}

	return 0;
}
