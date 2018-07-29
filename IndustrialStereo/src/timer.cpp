#include "timer.h"
#include <Windows.h>

Timer::Timer()
{
	initStatus = QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
}

bool Timer::begin()
{
	if (!initStatus) { return 0; }

	return QueryPerformanceCounter((LARGE_INTEGER*)&beginTime);
}

double Timer::end()
{
	if (!initStatus) { return 0; }

	__int64 endtime;

	QueryPerformanceCounter((LARGE_INTEGER*)&endtime);

	__int64 elapsed = endtime - beginTime;

	return ((double)elapsed / (double)frequency) * 1000.0f;  //µ•Œª∫¡√Î
}

bool Timer::isAvaliable()
{
	return initStatus;
}
