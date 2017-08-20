#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "../log.h"
#include "../Locker.h"
#define MAXBUF 1024

void* thread_func(void* arg) {
	int ssock;
	int clen;
	int times = 0;
	int writelen;
	struct sockaddr_in server_addr;
	char buf[MAXBUF];
	Logger* log = NULL;
	Locker* locker = locker_pthread_create();
	log = create_log("client.log", locker, 6);

	if ((ssock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0) {
		log->error(log,__FILE__ , __LINE__,__FUNCTION__,"create simple sockstream error.");
		exit(1);
	}
	clen = sizeof(server_addr);
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family =AF_INET;
	server_addr.sin_addr.s_addr=inet_addr("192.168.0.10");
	server_addr.sin_port =htons(3389);

	log->debug(log,__FILE__,__LINE__,__FUNCTION__,"thread %d connecting to the server.............",*((int*)arg));

	if(connect(ssock,(struct sockaddr *)&server_addr,clen)<0) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"connect to server error.");
		exit(1);
	}
	char* serverip = inet_ntoa(server_addr.sin_addr);
	log->debug(log,__FILE__,__LINE__,__FUNCTION__,"connected to the server:%s.",serverip);

	memset(buf,0,MAXBUF);
	//snprintf(buf,"the %d client send data %s\n",*((int*)arg),"test send data");
	
	while(times <2){
		writelen=write(ssock,"test data",strlen("test data"));
		if(writelen != strlen("test data")) {
			log->error(log,__FILE__,__LINE__,__FUNCTION__,"write to socket:%d:%s error.",ssock,"test data");
			exit(1);
		}

		log->debug(log,__FILE__,__LINE__,__FUNCTION__,"write to the server %s",buf);

		if(read(ssock,buf,MAXBUF)<=0)
		{
			log->error(log,__FILE__,__LINE__,__FUNCTION__,"read from socket error.");
			exit(1);
		}
		log->debug(log,__FILE__,__LINE__,__FUNCTION__,"read from server data:%s ",buf);
		//printf("\nread: %s\n",buf);
		times++;
		sleep(3);
	}
	close(ssock);

	log->debug(log,__FILE__,__LINE__,__FUNCTION__,"client %dth work done.",*((int*)arg));
}

int main() {
	int i;
	void* tret[5000];
	pthread_t thread[5000];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	for (i=0; i<5000; i++) {
		pthread_create(&thread[i], &attr, thread_func, (void *)( &i ));
	}
	for (i=0; i<5000; i++) {
			pthread_join(thread[i],&tret[i]);
	}
	for (i=0; i<5000; i++) {
			printf("thread[%d] ret with code=%d",i,(int)(tret[i]));
	}
	
	pthread_attr_destroy(&attr);

	return 0;
}

