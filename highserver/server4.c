#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "Locker.h"
//#include "os.h"

//#if defined(OS_UNIX)
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "locker_pthread.h"
//#endif

//#if defined(OS_WINDOWS)
//#include "win_semaphor_thread.h"
//#endif

#define MAXLINE 5
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

void handle_write_error(int fd);
void handle_accept_error();

//whole system log
Logger* log = NULL;

void getopt(){
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"Add your command args process here.");
}

void deamon_init(){
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"Add your deamon init code here.");
}

void setnonblocking(int sock) {
	int opts;
	opts=fcntl(sock, F_GETFL);
	if (opts<0) {
		perror("fcntl(sock,GETFL)");
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"fcntl(sock=%d,GETFL) error.",sock);
		exit(1);
	}
	opts = opts|O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts)<0) {
		perror("fcntl(sock,SETFL,opts)");
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"fcntl(sock=%d,GETFL) error.",sock);
		exit(1);
	}
}


int main() {

	//init the thread safe log
	//#if define(OS_UNIX)
	Locker* locker = locker_pthread_create();
	//#endif
	//#if define(OS_WINDOWS)
	//Locker* locker = locker_win_create();
	//#endif
	log = create_log("server.log", locker, 4);
	
	getopt();
	
#if defined(_DEAMON)
	deamon_init();
#endif

	int listen_fd, accept_fd, flag;
	struct sockaddr_in my_addr, remote_addr;
	if ( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("create socket error");
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"create simple sockstream error.");
        exit(1);
    }
    if ( setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&flag,sizeof(flag)) == -1 ){
        perror("setsockopt:SO_REUSEADDR error");
        log->error(log,__FILE__,__LINE__,__FUNCTION__,"setsockopt:SO_REUSEADDR error.");
    }
    
    setnonblocking(listen_fd);
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3389);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);
    if ( bind( listen_fd, (struct sockaddr *)&my_addr,
                sizeof(struct sockaddr_in)) == -1 ) {
        perror("bind error");
        log->error(log,__FILE__,__LINE__,__FUNCTION__,"bind port:3389 & addr:INADDR_ANY error.");
        exit(1);
    }
    
    log->debug(log, __FILE__,__LINE__,__FUNCTION__,"bind port:3389 & addr:INADDR_ANY successful.");
    
    if ( listen( listen_fd,1 ) == -1 ){
        perror("listen error");
        log->error(log, __FILE__, __LINE__,__FUNCTION__, "listen error.");
        exit(1);
    }
    log->info(log,__FILE__,__LINE__, __FUNCTION__,"listening.........................");
    struct epoll_event ev,events[5000];
    int epfd = epoll_create(256);
    int ev_s=0;
    ev.data.fd=listen_fd;
    ev.events=EPOLLIN|EPOLLET;
    epoll_ctl(epfd,EPOLL_CTL_ADD,listen_fd,&ev);
    for(;;){
        ev_s = epoll_wait( epfd,events,5000,500 );
        int i=0;
        for(i=0; i<ev_s; i++){
            char in_buf[1024];
            if(events[i].data.fd==listen_fd){
                log->debug(log,__FILE__,__LINE__,__FUNCTION__,"New connection come on %d.",listen_fd);
                int addr_len = sizeof( struct sockaddr_in );
                accept_fd = accept( listen_fd,(struct sockaddr *)&remote_addr,&addr_len );
                if(accept_fd<0){
                	log->error(log, __FILE__, __LINE__, __FUNCTION__,"error accept_fd=%d<0.",accept_fd);
                	handle_accept_error(listen_fd);
			//if one error happen we should exit(1),not continue and try again.when create core file ,my harddisk full and server restart because there is a too big coredump file.
	                //if all connection not working,we definitely know what happen to server.
		        exit(1);                	
			//continue;
                }
                char* clientip = inet_ntoa(remote_addr.sin_addr);                        
                log->info(log,__FILE__, __LINE__,__FUNCTION__,"New connection from %s accepted. accepted fd:%d.",clientip,accept_fd);
                int flags = fcntl(accept_fd, F_GETFL, 0);
                fcntl(accept_fd, F_SETFL, flags|O_NONBLOCK);
                ev.data.fd=accept_fd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,accept_fd,&ev);
            }
            else if(events[i].events&EPOLLIN){
                unsigned int n = 0; 
                int fd = events[i].data.fd;                       
                log->debug(log,__FILE__,__LINE__,__FUNCTION__,"new events of EPOLLIN ocured.");
                
                memset(in_buf, 0, 1024);
                n = read( events[i].data.fd ,&in_buf ,1024 ,0 );
                
                log->debug(log,__FILE__,__LINE__,__FUNCTION__,"read:%s from the server", in_buf );
	          
                if(n<0){
                	if(errno == ECONNRESET){
                		close(fd);
                		log->debug(log,__FILE__,__LINE__,__FUNCTION__,"connection fd=%d reset.",fd);
                	}else{
                		log->error(log,__FILE__,__LINE__,__FUNCTION__,"read fd=%d error.",fd);
                	}
                }else {
                	if(n==0){
                		close(fd);
                		log->info(log,__FILE__,__LINE__,__FUNCTION__,"client connection closed.");
                	} else {
                		log->debug(log,__FILE__,__LINE__,__FUNCTION__,"add your recived packet handle function here.");
                		log->debug(log,__FILE__,__LINE__,__FUNCTION__,"processing the data packet....................");
                		log->debug(log,__FILE__,__LINE__,__FUNCTION__,"set your response packet handle function here.");
						ev.data.ptr=in_buf;
						ev.events=EPOLLOUT|EPOLLET;
						ev.data.fd = fd;
						epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
						log->debug(log,__FILE__,__LINE__,__FUNCTION__,"read ok.modify the fd=%d status to EPOLLOUT.", fd );
					}
                }
            }
            else if(events[i].events&EPOLLOUT){
            	
            	int writelen;
            	log->debug(log,__FILE__,__LINE__,__FUNCTION__,"Fd=%d can write now.",events[i].data.fd);
            	//fixme:get the data len by the actual size not only for string
            	writelen=write(events[i].data.fd,/*events[i].data.ptr*/"test reviced",/*strlen(events[i].data.ptr)*/strlen("test reviceds"));
            	if(writelen == -1){
            		log->debug(log, __FILE__,__LINE__, __FUNCTION__,"write to fd=%d %s fails.",events[i].data.fd,/*events[i].data.ptr*/"test reviced");
            		close(events[i].data.fd);
            		handle_write_error(events[i].data.fd);
            		continue;
            	}
            	log->debug(log, __FILE__, __LINE__, __FUNCTION__,"write to fd=%d %s.",events[i].data.fd,"test reviced");
	          
            	ev.events=EPOLLIN|EPOLLET;
            	ev.data.fd = events[i].data.fd;
            	epoll_ctl(epfd,EPOLL_CTL_MOD,events[i].data.fd,&ev);
    		
            	log->debug(log,__FILE__,__LINE__,__FUNCTION__,"write ok.modify the fd=%d status to EPOLLIN.", events[i].data.fd );  
            }
            else if(events[i].events&EPOLLPRI){
            	log->debug(log,__FILE__, __LINE__, __FUNCTION__,"Fd=%d has urgent data come.",events[i].data.fd);
            }
            else if(events[i].events&EPOLLERR){
            	log->debug(log,__FILE__, __LINE__, __FUNCTION__,"Fd=%d file descriptor error.",events[i].data.fd);
            }
            else if(events[i].events&EPOLLHUP){
            	log->debug(log,__FILE__, __LINE__,__FUNCTION__,"Fd=%d file descriptor hup.",events[i].data.fd);
            }
            else {
            	log->debug(log, __FILE__, __LINE__,__FUNCTION__,"Fd=%d other events occur.",events[i].data.fd);
            }
        }
   }
    return 0;
}


void handle_write_error(int fd) {
	if (errno == EINTR) {
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"invocation is interrupted");
	} else if(errno == EAGAIN) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"no data can write to fd=%d,blocking.",fd);
	} else if((errno == EBADF)||(errno == EINVAL)) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"Invalid fd=%d",fd);
	} else if(errno == EFAULT) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"buffer outof space.");
	} else if(errno == EFBIG) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"Data write is too big。");
	} else if(errno == EIO) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"low level io error。");
	} else if(errno == ENOSPC) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"file has no space to write.");
	} else if(errno == EPIPE) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"reading end has closed。");
	}
}


void handle_accept_error(int fd) {
	if (errno == EINTR) {
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"invocation is interrupted");
	} else if((errno == EAGAIN)||(errno == EWOULDBLOCK)) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"blocking.............try again?????");
	} else if((errno == EBADF)) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"Invalid fd=%d",fd);
	} else if(errno == EINVAL) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"socket fd=%d is not listening",fd);
	} else if(errno == EFAULT) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"accpet addr not write able.");
	} else if(errno == ECONNABORTED) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"connection aborted.");
	} else if(errno == ENFILE) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"max open file limited");
	} else if(errno == EINVAL) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"client not listening.");
	} else if((errno == ENOBUFS)||(errno == ENOMEM)) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"server socket buffer memory not enough。");
	}else if(errno == EPERM) {
		log->error(log,__FILE__,__LINE__,__FUNCTION__,"fire wall forbiden connection。");
	}
}
