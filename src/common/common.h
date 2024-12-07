//common includes and defines
// also C_Mutex, C_Thread, C_Event

#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif

#endif
#include <windows.h>
#include <winbase.h>
#include <time.h>
#include <process.h>
 #if defined(_MSC_VER) && (_MSC_VER < 1600) //(1600 = Visual Studio 2010)
  typedef long long          int64_t;
  typedef unsigned long long uint64_t;
  typedef int                int32_t;
  typedef unsigned int       uint32_t;
  typedef short              int16_t;
  typedef unsigned short     uint16_t;
  typedef char               int8_t;
  typedef unsigned char      uint8_t;
 #else
  #include <stdint.h>
 #endif

#define mssleep(x) Sleep(x)

#else
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdint.h>
#include <limits.h>

struct POINT
{
	int32_t x;
	int32_t y;
};

#include <strings.h>
#define _stricmp strcasecmp

#define mssleep(x) usleep((x)*1000)

#define MAX_PATH 2048

#define INFINITE 0xFFFFFFFF

#endif

////////////////////////////////////////////////////////////////////////////////

#define PI 3.1415926535

union AllPtrType
{
	uint8_t  *u8ptr;
	int8_t   *s8ptr;
	uint16_t *u16ptr;
	int16_t  *s16ptr;
	uint32_t *u32ptr;
	int32_t  *s32ptr;
	uint64_t *u64ptr;
	int64_t  *s64ptr;
	void     *vptr;
};

typedef struct
{
	double x, y;
} VECTOR, POINTD;

struct S_Rect
{
	int x, y;
	int width, height;
};

//"constructor" for S_Rect
inline S_Rect S_RECTMAKE(const int x, const int y, const int w, const int h)
{
	S_Rect result;
	result.x = x;
	result.y = y;
	result.width  = w;
	result.height = h;
	return result;
}

//color red, green, blue, alpha (0..255)
struct S_Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

//"constructor" for S_Color
inline S_Color S_COLORMAKE(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	S_Color result;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = a;
	return result;
}

//color red, green, blue, alpha (0.0f..1.0f)
struct S_FColor
{
	float r;
	float g;
	float b;
	float a;
};

//"constructor" for S_FColor
inline S_FColor S_FCOLORMAKE(const float r, const float g, const float b, const float a)
{
	S_FColor result;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = a;
	return result;
}

////////////////////////////////////////////////////////////////////////////////
class C_Mutex
{
public:
	C_Mutex();
	~C_Mutex();
	void Enter();
	void Leave();
	//bool TryEnter();
private:
#ifdef WIN32
	CRITICAL_SECTION m_hCS;
	//HANDLE m_hMutex;
#else
	pthread_mutex_t m_hMutex;
#endif
};
//this class supports multiple threads

////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define THREAD_RET_TYPE DWORD
#define THREAD_RET DWORD WINAPI
#define THREAD_RET_0 0
typedef DWORD ((WINAPI *THREADTYPE)(void *pParam));
#else
#define THREAD_RET_TYPE void*
#define THREAD_RET void*
#define THREAD_RET_0 NULL
typedef void*((*THREADTYPE)(void *pParam));
#endif

class C_Thread
{
public:
	C_Thread(THREADTYPE pThread, void *pParam);
	~C_Thread();
	bool WaitForThreadExit(int iTimeout = -1);

	static int NumberOfCPUs();
private:
#ifdef WIN32
	HANDLE m_hThread;
#else
	pthread_t m_hThread;
#endif
};
//currently this class supports one thread waiting

////////////////////////////////////////////////////////////////////////////////
class C_Event
{
public:
	C_Event();
	~C_Event();
	void Signal();
	bool Wait(int iTimeout = -1);
private:
#ifdef WIN32
	HANDLE m_hEvent;
#else
	pthread_mutex_t m_hMutex;
	pthread_cond_t  m_hCond;
	volatile bool m_bSignaled;
#endif
};
//currently this class supports one thread signaling and one thread waiting

#endif
