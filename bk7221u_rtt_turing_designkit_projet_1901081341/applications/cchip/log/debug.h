
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <string.h>

#include "log.h"

#define SHOW_LOG

//#define TURING_DEBUG  

//extern int showlog;

#ifdef TURING_DEBUG  
#define DEBUG_ERR(fmt, args...)    do { fprintf(stderr,"[%s:%d]"#fmt"\r\n", __func__, __LINE__, ##args); } while(0) 
#define DEBUG_INFO(fmt, args...)   do { printf("[%s:%d]"#fmt"\r\n", __func__, __LINE__, ##args); } while(0) 
#else
#define DEBUG_ERR(fmt, args...)   do {} while(0)  
#define DEBUG_INFO(fmt, args...)  do {} while(0)  
#endif


#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)


#ifdef SHOW_LOG

#ifndef NO_ERROR
//#define error(...) do {if(showlog) log_log(LOG_ERROR, __FILE__, __func__, __LINE__, __VA_ARGS__); }while(0)
#define error(...) do {log_log(LOG_ERROR, __FILENAME__, __func__, __LINE__, __VA_ARGS__); }while(0)

#else
#define error(...) 
#endif


#define      DEBUG			
#ifdef DEBUG
//#define debug(...)      do {if(showlog) log_log(LOG_DEBUG, __FILE__, __func__, __LINE__, __VA_ARGS__); }while(0)
#define debug(...)      do {log_log(LOG_DEBUG, __FILENAME__, __func__, __LINE__, __VA_ARGS__); }while(0)
#else 
#define debug(...) 
#endif


#ifndef NO_WARNING
//#define warning(...)      do {if(showlog) log_log(LOG_WARN, __FILE__, __func__, __LINE__, __VA_ARGS__); }while(0)
#define warning(...)      do { log_log(LOG_WARN, __FILENAME__, __func__, __LINE__, __VA_ARGS__); }while(0)

#else
#define warning(...) 
#endif

#define      INFO
#ifdef INFO
//#define info(...) do {if(showlog) log_log(LOG_INFO, __FILE__, __func__, __LINE__, __VA_ARGS__); }while(0)
#define info(...) do { log_log(LOG_INFO, __FILENAME__, __func__, __LINE__, __VA_ARGS__); }while(0)

#else
#define info(...) 
#endif

#ifdef TRACE
//#define trace(...)   do {if(showlog) log_log(LOG_TRACE, __FILE__, __func__, __LINE__, __VA_ARGS__); }while(0)
#define trace(...)   do { log_log(LOG_TRACE, __FILENAME__, __func__, __LINE__, __VA_ARGS__); }while(0)

#else
#define trace(...) 
#endif

#ifndef FATAL
//#define fatal(...)   do {if(showlog) log_log(LOG_FATAL, __FILE__, __func__, __LINE__, __VA_ARGS__); }while(0)
#define fatal(...)   do { log_log(LOG_FATAL, __FILENAME__, __func__, __LINE__, __VA_ARGS__); }while(0)
#else
#define fatal(...) 
#endif

#else

#define error(...) 
#define debug(...) 
#define warning(...) 
#define info(...) 
#define trace(...)

#endif

#endif

