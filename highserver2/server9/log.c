#include "stdarg.h"
#include "malloc.h"
#include "time.h"
#include "string.h"
#include "log.h"
#include "Locker.h"

static const int FATAL = 1;
static const int ERROR = 2;
static const int WARN = 3;
static const int INFO = 4;
static const int DEBUG = 5;
static const int ALL = 6;

void fatal(Logger* log,const char * filename, const int line, const char * funcname,char* fmt,...)
{
	char buf[1024];
        time_t t;
        struct tm * now;
        va_list ap;

        time(&t);
        now = localtime(&t);

	log->lock->lock(log->lock);
	if(log->logLevel >= FATAL)
	{
		
		 va_start(ap, fmt);
                 fprintf(log->fp, 
                         "[%04d-%02d-%02d %02d:%02d:%02d -- %s(%d):%s]:", 
                         now -> tm_year + 1900, 
                         now -> tm_mon + 1, 
                         now -> tm_mday, 
                         now -> tm_hour, 
                         now -> tm_min, 
                         now -> tm_sec, 
                         filename, 
                         line, 
                         funcname);
                 vsprintf(buf, fmt, ap);
                 fprintf(log->fp, "%s\n", buf);
                 va_end(ap);
	}
	fflush(log->fp);
	log->lock->unlock(log->lock);
}

void error(Logger* log,const char * filename, const int line, const char * funcname,char* fmt,...)
{
	char buf[1024];
        time_t t;
        struct tm * now;
        va_list ap;

        time(&t);
        now = localtime(&t);

	log->lock->lock(log->lock);
	
	if(log->logLevel >= ERROR)
	{
		 va_start(ap, fmt);
                 fprintf(log->fp, 
                         "[%04d-%02d-%02d %02d:%02d:%02d -- %s(%d):%s]:", 
                         now -> tm_year + 1900, 
                         now -> tm_mon + 1, 
                         now -> tm_mday, 
                         now -> tm_hour, 
                         now -> tm_min, 
                         now -> tm_sec, 
                         filename, 
                         line, 
                         funcname);
                 vsprintf(buf, fmt, ap);
                 fprintf(log->fp, "%s\n", buf);
                 va_end(ap);
	}
	fflush(log->fp);
	log->lock->unlock(log->lock);
}

void warn(Logger* log,const char * filename, const int line, const char * funcname,char* fmt,...)
{
	char buf[1024];
        time_t t;
        struct tm * now;
        va_list ap;

        time(&t);
        now = localtime(&t);

	log->lock->lock(log->lock);
	
	if(log->logLevel >= WARN)
	{
		va_start(ap, fmt);
                 fprintf(log->fp, 
                         "[%04d-%02d-%02d %02d:%02d:%02d -- %s(%d):%s]:", 
                         now -> tm_year + 1900, 
                         now -> tm_mon + 1, 
                         now -> tm_mday, 
                         now -> tm_hour, 
                         now -> tm_min, 
                         now -> tm_sec, 
                         filename, 
                         line, 
                         funcname);
                 vsprintf(buf, fmt, ap);
                 fprintf(log->fp, "%s\n", buf);
                 va_end(ap);
	}
	fflush(log->fp);
	log->lock->unlock(log->lock);
}

void debug(Logger* log,const char * filename, const int line, const char * funcname,char* fmt,...)
{
	char buf[1024];
        time_t t;
        struct tm * now;
        va_list ap;

        time(&t);
        now = localtime(&t);

	log->lock->lock(log->lock);
	
	if(log->logLevel >= DEBUG)
	{
		va_start(ap, fmt);
                 fprintf(log->fp, 
                         "[%04d-%02d-%02d %02d:%02d:%02d -- %s(%d):%s]:", 
                         now -> tm_year + 1900, 
                         now -> tm_mon + 1, 
                         now -> tm_mday, 
                         now -> tm_hour, 
                         now -> tm_min, 
                         now -> tm_sec, 
                         filename, 
                         line, 
                         funcname);
                 vsprintf(buf, fmt, ap);
                 fprintf(log->fp, "%s\n", buf);
                 va_end(ap);
	}
	fflush(log->fp);
	log->lock->unlock(log->lock);
}

void info(Logger* log,const char * filename, const int line, const char * funcname,char* fmt,...)
{
	char buf[1024];
        time_t t;
        struct tm * now;
        va_list ap;

        time(&t);
        now = localtime(&t);

	log->lock->lock(log->lock);
	
	if(log->logLevel >= INFO)
	{
		va_start(ap, fmt);
                 fprintf(log->fp, 
                         "[%04d-%02d-%02d %02d:%02d:%02d -- %s(%d):%s]:", 
                         now -> tm_year + 1900, 
                         now -> tm_mon + 1, 
                         now -> tm_mday, 
                         now -> tm_hour, 
                         now -> tm_min, 
                         now -> tm_sec, 
                         filename, 
                         line, 
                         funcname);
                 vsprintf(buf, fmt, ap);
                 fprintf(log->fp, "%s\n", buf);
                 va_end(ap);
	}
	fflush(log->fp);
	log->lock->unlock(log->lock);
}

void all(Logger* log,const char * filename, const int line, const char * funcname,char* fmt,...)
{
	char buf[1024];
        time_t t;
        struct tm * now;
        va_list ap;

        time(&t);
        now = localtime(&t);

	log->lock->lock(log->lock);
	
	if(log->logLevel >= ALL)
	{
		 va_start(ap, fmt);
                 fprintf(log->fp, 
                         "[%04d-%02d-%02d %02d:%02d:%02d -- %s(%d):%s]:", 
                         now -> tm_year + 1900, 
                         now -> tm_mon + 1, 
                         now -> tm_mday, 
                         now -> tm_hour, 
                         now -> tm_min, 
                         now -> tm_sec, 
                         filename, 
                         line, 
                         funcname);
                 vsprintf(buf, fmt, ap);
                 fprintf(log->fp, "%s\n", buf);
                 va_end(ap);
	}
	fflush(log->fp);
	log->lock->unlock(log->lock);
}

void destroy_log(Logger* log)
{
	fclose(log->fp);
	log->fp = NULL;
	log->lock->destroy(log->lock);

	free(log);
	log = NULL;
}



Logger* create_log(char* filename,Locker* lock,int level){

	Logger* log = (Logger*)malloc(sizeof(Logger));
	if(NULL == log)
	{
		printf("Logger create error,no more mem!\n");
		exit(1);
	}
	log->all = all;
	log->debug = debug;
	log->info = info;
	log->warn=warn;
	log->error = error;
	log->fatal = fatal;
	log->destroy = destroy_log;
	log->fp = fopen(filename,"at");
	if(NULL == log->fp){
		printf("Open log file error!\n");
	}
	log->lock = lock;
	log->logLevel = level;
	if(NULL != log->fp){
		fprintf(log->fp,"Thank you for using ZSL_LIB_LOG.\n");
		fprintf(log->fp,"ZSL_LIB_LOG inited!\n\n\n");
	}
	fflush(log->fp);
	return log;
}

