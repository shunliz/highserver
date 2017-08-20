#ifndef ZSL_LIB_LOG_H
#define ZSL_LIB_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "Locker.h"
#include <stdarg.h>
#include <time.h>


struct _logger;
typedef struct _logger Logger;
typedef void (*Fatal)(Logger* log,const char * filename, const int line, const char * funcname,char* msg,...);
typedef void (*Error)(Logger* log,const char * filename, const int line, const char * funcname,char* msg,...);
typedef void (*Warn)(Logger* log,const char * filename, const int line, const char * funcname,char* msg,...);
typedef void (*Info)(Logger* log,const char * filename, const int line, const char * funcname,char* msg,...);
typedef void (*Debug)(Logger* log,const char * filename, const int line, const char * funcname,char* msg,...);
typedef void (*LogAll)(Logger* log,const char * filename, const int line, const char * funcname,char* msg,...);
typedef void (*Destroy)(Logger* log);

struct _logger{
	Fatal fatal;
	Error error;
	Debug warn;
	Info info;
	Debug debug;
	LogAll all;
	Destroy destroy;
	FILE* fp;
	int logLevel;
	Locker* lock;
};

#ifdef __cplusplus
}
#endif


#endif