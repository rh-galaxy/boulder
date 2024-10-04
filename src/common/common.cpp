// C_Mutex, C_Thread, C_Event

//classes for OS specific execution controlling functions.
// - thread
// - mutex
// - event

#include "common.h"

////////////////////////////////////////////////////////////////////////////////
C_Mutex::C_Mutex()
{
#ifdef WIN32
	//m_hMutex = CreateMutex(NULL, FALSE, NULL);
	InitializeCriticalSection(&m_hCS);
#else
	pthread_mutex_init(&m_hMutex, NULL);
#endif
}

C_Mutex::~C_Mutex()
{
#ifdef WIN32
	//CloseHandle(m_hMutex);
#else
	pthread_mutex_destroy(&m_hMutex);
#endif
}

void C_Mutex::Enter()
{
	//lock
#ifdef WIN32
	//WaitForSingleObject(m_hMutex, INFINITE);
	EnterCriticalSection(&m_hCS);
#else
	pthread_mutex_lock(&m_hMutex);
#endif
}

//commented to be able to use CriticalSection instead of mutex for windows (more efficient).
/*bool C_Mutex::TryEnter()
{
	//try lock, returns true if the mutex can be locked immediately
#ifdef WIN32
	return (WaitForSingleObject(m_hMutex, 0)==WAIT_OBJECT_0);
#else
	return (pthread_mutex_trylock(&m_hMutex)==0);
#endif
}*/

void C_Mutex::Leave()
{
	//unlock
#ifdef WIN32
	//ReleaseMutex(m_hMutex);
	LeaveCriticalSection(&m_hCS);
#else
	pthread_mutex_unlock(&m_hMutex);
#endif
}

////////////////////////////////////////////////////////////////////////////////
C_Thread::C_Thread(THREADTYPE pThread, void *pParam)
{
#ifdef WIN32
	m_hThread = CreateThread(NULL, NULL, pThread, pParam, NULL, NULL);
	SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL);
#else
	pthread_attr_t stAttr;
	pthread_attr_init(&stAttr);
	pthread_create(&m_hThread, &stAttr, pThread, pParam);
	pthread_attr_destroy(&stAttr);
#endif
}

bool C_Thread::WaitForThreadExit(int iTimeout)
{
#ifdef WIN32
	DWORD dwError = WaitForSingleObject(m_hThread, iTimeout==-1? INFINITE : iTimeout);
	return (dwError==WAIT_OBJECT_0);
#else
	//no timeout can be used?
	return (pthread_join(m_hThread, NULL)==0);
#endif
}

C_Thread::~C_Thread()
{
#ifdef WIN32
	CloseHandle(m_hThread);
#else
	//?
#endif
}

int C_Thread::NumberOfCPUs()
{
#ifdef WIN32
	SYSTEM_INFO stSysinfo;
	GetSystemInfo(&stSysinfo);
	return (int)stSysinfo.dwNumberOfProcessors;
#else
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

////////////////////////////////////////////////////////////////////////////////
C_Event::C_Event()
{
#ifdef WIN32
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	pthread_mutex_init(&m_hMutex, NULL);
	pthread_cond_init(&m_hCond, NULL);
	m_bSignaled = false;
#endif
}

C_Event::~C_Event()
{
#ifdef WIN32
	CloseHandle(m_hEvent);
#else
	pthread_mutex_lock(&m_hMutex);
	pthread_cond_broadcast(&m_hCond);
	pthread_mutex_unlock(&m_hMutex);
	pthread_cond_destroy(&m_hCond);
	pthread_mutex_destroy(&m_hMutex);
#endif
}

void C_Event::Signal()
{
#ifdef WIN32
	SetEvent(m_hEvent);
#else
	pthread_mutex_lock(&m_hMutex);
	m_bSignaled = true;
	pthread_cond_signal(&m_hCond);
	pthread_mutex_unlock(&m_hMutex);
#endif
}

bool C_Event::Wait(int iTimeout)
{
#ifdef WIN32
	DWORD dwError = WaitForSingleObject(m_hEvent, iTimeout==-1? INFINITE : iTimeout);
	return (dwError==WAIT_OBJECT_0);
#else
	int ret = 0;
	timespec stTimeSpec;
	if(iTimeout!=-1) {
		timeval  stTimeVal;
		gettimeofday(&stTimeVal, NULL);
		stTimeSpec.tv_sec = stTimeVal.tv_sec + (iTimeout/1000);
		stTimeSpec.tv_nsec = stTimeVal.tv_usec*1000 + ((iTimeout%1000)*1000000);
		if(stTimeSpec.tv_nsec>=1000000000) {
			stTimeSpec.tv_sec += 1;
			stTimeSpec.tv_nsec -= 1000000000;
		}
	}
	pthread_mutex_lock(&m_hMutex);
	//m_bSignaled = false; //reset
	while(!m_bSignaled && ret==0) {
		if(iTimeout!=-1) ret=pthread_cond_timedwait(&m_hCond, &m_hMutex, &stTimeSpec);
		else ret=pthread_cond_wait(&m_hCond, &m_hMutex);
	}
	m_bSignaled = false; //reset
	pthread_mutex_unlock(&m_hMutex);
	return (ret==0);
#endif
}
