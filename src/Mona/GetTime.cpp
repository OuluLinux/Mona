#if 0
/*
    Get time in milliseconds since the initial call.
*/

#include "GetTime.h"
#include <assert.h>

Time gettime() {
	Time t;
	#ifdef WIN32
	static Time base_time = 0;

	if (base_time == 0) {
		base_time = (Time)GetTickCount64();
		return (0);
	}
	else {
		t = (Time)GetTickCount64();
		ASSERT(t >= base_time);
		return (t - base_time);
	}

	#else
	static time_t      base_sec  = 0;
	static suseconds_t base_usec = 0;
	struct timeval     tv;
	gettimeofday(&tv, NULL);

	if (base_sec == 0) {
		base_sec  = tv.tv_sec;
		base_usec = tv.tv_usec;
		return (0);
	}
	else {
		t = (Time)(((tv.tv_sec - base_sec) * 1000) + ((tv.tv_usec - base_usec) / 1000));
		return (t);
	}

	#endif
}
#endif
