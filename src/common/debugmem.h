
//must be included first in all files to give correct output when a memory leak is detected
//only works in windows

#ifndef _DEBUG_MEM_H_
#define _DEBUG_MEM_H_

#ifdef WIN32
#ifdef _DEBUG
#include <crtdbg.h>
#define NEW_INLINE_WORKAROUND new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new NEW_INLINE_WORKAROUND 
#endif
#endif

#endif