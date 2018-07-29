#ifndef _SHUTTLECOCK_RECOGNIZER_H
#define _SHUTTLECOCK_RECOGNIZER_H

#include "mv_stereo.h"

class ShuttleRecognizer : public MVStereo
{
public:
	ShuttleRecognizer() {}
	explicit ShuttleRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight);
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