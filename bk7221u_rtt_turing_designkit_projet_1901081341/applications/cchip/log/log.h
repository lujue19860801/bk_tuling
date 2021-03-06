/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "0.1.0"
#define LOG_USE_COLOR

typedef void (*log_LockFn)(void *udata, int lock);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#ifdef USE_LOG
#define log_trace(...) log_log(LOG_TRACE, __func__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __func__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,__func__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,__func__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR,  __func__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __func__, __LINE__, __VA_ARGS__)
#else
#define log_trace(...) 
#define log_debug(...) 
#define log_info(...)  
#define log_warn(...)  
#define log_error(...) 
#define log_fatal(...)
#endif



void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);

void log_log(int level, const char *file,const char *func, int line, const char *fmt, ...);
void filelog_deinit();
int filelog_init(char *file_name);
#endif
