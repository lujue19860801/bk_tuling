#ifndef __TURING_UPLOADFILE_H__
#define __TURING_UPLOADFILE_H__

#include "TuringCommon.h"

#define TUPLOAD_DEBUG_PRTF                     1

#define TUPLOAD_FATAL_PRINTF                   rt_kprintf
#define TUPLOAD_WARNING_PRINTF                 rt_kprintf
#define TUPLOAD_LOG_PRINTF(...)

#if TUPLOAD_DEBUG_PRTF
#define TUPLOAD_PRINTF                         rt_kprintf
#else
#define TUPLOAD_PRINTF(...)
#endif //TUPLOAD_DEBUG_PRTF

int TuringHttpUploadFile(char * , int);
#endif
