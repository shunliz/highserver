#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include <string.h>
#include "log.h"
#include "Locker.h"
#include "locker_pthread.h"


#define MAXBUF 1024


//whole system log
Logger* log = NULL;

int main(int argc ,char** argv) {

	Locker* locker = locker_pthread_create();
	log = create_log("client9.log", locker, 6);

	int i =3;
	int ssock;
	int clen;
	int writelen;
	struct sockaddr_in server_addr;
	char buf[MAXBUF];
	char end[2]={0x10,0x13};
	
	if(argc<3)
		return 1;

	if ((ssock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0) {
		perror("create simple sockstream error.");
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"create simple sockstream error.");
		exit(1);
	}
	clen = sizeof(server_addr);
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family =AF_INET;
	server_addr.sin_addr.s_addr=inet_addr(argv[1]);
	server_addr.sin_port =htons(atoi(argv[2]));


	if(connect(ssock,(struct sockaddr *)&server_addr,clen)<0) {
		perror("connect to server error.");
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"connect to server error.");
		exit(1);
	}
	char* serverip = inet_ntoa(server_addr.sin_addr);
	

	memset(buf,0,MAXBUF);
	
	writelen=write(ssock,argv[3],strlen(argv[3]));
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"write data:\%s",argv[3]);
	if(writelen != strlen(argv[3])) {
		perror("write to socket error.");
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"write to socket error.");
		exit(1);
	}
	
	writelen=write(ssock,end,2);
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"write data:\%s",end);
	if(writelen != 2) {
		perror("write end to socket error.");
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"write end to socket error.");
		exit(1);
	}
	
	if(read(ssock,buf,MAXBUF)<=0)
	{
		perror("read from socket error.");
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"read from socket error.");
		exit(1);
	}
	
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"read from socket data:%s.",buf);
	
	close(ssock);



}