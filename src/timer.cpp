#include "timer.h"
#include <Windows.h>

Timer::Timer()
{
	Initialized = QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
}

bool Timer::Begin()
{
	if (!Initialized) { return 0; }

	return QueryPerformanceCounter((LARGE_INTEGER*)&BeginTime);
}

double Timer::End()
{
	if (!Initialized) { return 0; }

	__int64 endtime;

	QueryPerformanceCounter((LARGE_INTEGER*)&endtime);

	__int64 elapsed = endtime - BeginTime;

	return ((double)elapsed / (double)Frequency) * 1000.0f;  //µ•Œª∫¡√Î
}

bool Timer::Avaliable()
{
	return Initialized;
}
