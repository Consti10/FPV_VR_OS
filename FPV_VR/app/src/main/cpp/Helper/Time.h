#ifndef TIME_FPV_VR
#define TIME_FPV_VR

#include <stdint.h>
#include <linux/time.h>
#include <time.h>
#include <sys/time.h>


/******************************************
 * use "getSystemTime##() if you want to get the current linux time
 * "getTime##()" returns way smaller values (depending on the time the app was started) that can only be used for comparisons in FPV_VR
 *****************************************/

typedef uint64_t cNanoseconds;
typedef uint64_t cMicroseconds;
typedef uint64_t cMilliseconds;

static cNanoseconds timeBase = 0;

static cNanoseconds getSystemTimeNS(){
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    return (cNanoseconds) ts.tv_sec * 1000ULL * 1000ULL * 1000ULL + ts.tv_nsec;
}

static cMicroseconds getSystemTimeUS(){
    cNanoseconds cN=getSystemTimeNS();
    if(cN==0){
        return 0;
    }
    return (cMicroseconds )(cN/1000ULL);
}

static cNanoseconds getTimeNS(){
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );

	if ( timeBase == 0 )
	{
		timeBase = (cNanoseconds) ts.tv_sec * 1000ULL * 1000ULL * 1000ULL + ts.tv_nsec;
	}
	return (cNanoseconds) ts.tv_sec * 1000ULL * 1000ULL * 1000ULL + ts.tv_nsec - timeBase;
}
static cMicroseconds getTimeUS(){
    cNanoseconds cN=getTimeNS();
    if(cN==0){
        return 0;
    }
    return (cMicroseconds )(cN/1000ULL);
}

static cMilliseconds getTimeMS(){
    cNanoseconds cN=getTimeNS();
    if(cN==0){
        return 0;
    }
    return cN/1000ULL/1000ULL;
}

#endif