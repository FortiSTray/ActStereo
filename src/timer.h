#ifndef _TIMER_H
#define _TIMER_H

class Timer
{
public:
	explicit Timer();
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;
	virtual ~Timer() {}

	double End();
	bool Begin();
	bool Avaliable();

private:
	int Initialized;
	__int64 Frequency;
	__int64 BeginTime;

};

#endif //_TIMER_H