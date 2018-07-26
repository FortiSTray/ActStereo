#ifndef _SHUTTLECOCK_RECOGNIZER_H
#define _SHUTTLECOCK_RECOGNIZER_H

#include "stereo_camera.h"

class ShuttleRecognizer : public StereoCamera
{
public:
	ShuttleRecognizer() {}
	explicit ShuttleRecognizer(char idLeft, CameraArguments argsLeft, char idRight, CameraArguments argsRight);
	ShuttleRecognizer(const ShuttleRecognizer&) = delete;
	ShuttleRecognizer& operator=(const ShuttleRecognizer&) = delete;
	virtual ~ShuttleRecognizer() {}

	void BackgroundSubtract();

private:
	Mat fgMaskLeft;
	Mat fgMaskRight;
	Mat fgImageLeft;
	Mat fgImageRight;

	Ptr<BackgroundSubtractor> bgModel;

};

#endif //_SHUTTLECOCK_RECOGNIZER_H