#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	VideoCapture cameraLeft(2);
	VideoCapture cameraRight(1);

	Mat frameLeft;
	Mat frameRight;
	Mat tempL;
	Mat tempR;

	int counter = 0;
	const int photoNum = 20;
	char fileNameL[25];
	char fileNameR[25];

	int keyFlag = 0;

	while (true)
	{
		cameraLeft >> tempL;
		cameraRight >> tempR;

		frameLeft = tempL.clone();
		frameRight = tempR.clone();

		for (auto i = 0; i < tempL.rows; i++)
			for (auto j = 0; j < tempL.cols; j++)
			{
				frameLeft.ptr<Vec3b>(i)[j] = tempL.ptr<Vec3b>(tempL.rows - i - 1)[tempL.cols - j - 1];
			}

		for (auto i = 0; i < tempR.rows; i++)
			for (auto j = 0; j < tempR.cols; j++)
			{
				frameRight.ptr<Vec3b>(i)[j] = tempR.ptr<Vec3b>(tempR.rows - i - 1)[tempR.cols - j - 1];
			}

		imshow("Left", frameLeft);
		imshow("Right", frameRight);
		
		keyFlag = waitKey(1);
		if (keyFlag == 0x20)
		{
			counter++;
			sprintf_s(fileNameL, "./Stereo/Left%d.png", counter);
			sprintf_s(fileNameR, "./Stereo/Right%d.png", counter);
			imwrite(fileNameL, frameLeft);
			imwrite(fileNameR, frameRight);
			
			if (counter == photoNum) { break; }
		}
		else if (keyFlag == 0x1B)
		{
			break;
		}
	}

	return 0;
}