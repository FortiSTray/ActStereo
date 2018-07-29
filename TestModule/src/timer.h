#ifndef _TIMER_H
#define _TIMER_H

class Timer
{
public:
	explicit Timer();
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;
	virtual ~Timer() {}

	double end();
	bool begin();
	bool isAvaliable();

private:
	int initStatus;
	__int64 frequency;
	__int64 beginTime;

};

#endif //_TIMER_H