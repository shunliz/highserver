/*
先是在accept上阻塞
然后当有新链接过来就会fork
当然这样一个结构并不比多进程阻塞版本好太多
因为每个进程虽然使用epoll来处理
但是一个进程内只有一个accept_fd在被处理
所以效果是一样的
在下一个版本中
我们将在一个进程使用epoll处理多个accept
*/


#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h>
#include <sys/epoll.h>

int main(){
	int listen_fd,accept_fd,flag;
	struct sockaddr_in my_addr,remote_addr;
	if ( (listen_fd = socket( AF_INET,SOCK_STREAM,0 )) == -1 ){
		perror("create socket error");
		exit(1);
	}
	if ( setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&flag,sizeof(flag)) == -1 ){
		perror("setsockopt error");
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(3389);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if ( bind( listen_fd, (struct sockaddr *)&my_addr,sizeof(struct sockaddr_in)) == -1 ) {
			perror("bind error");
			exit(1);
	}
	if ( listen( listen_fd,1 ) == -1 ){
		perror("listen error");
		exit(1);
	}
	for(;;){
		int addr_len = sizeof( struct sockaddr_in );
		accept_fd = accept( listen_fd,(struct sockaddr *)&remote_addr,&addr_len );
		int flags = fcntl(accept_fd, F_GETFL, 0);
		fcntl(accept_fd, F_SETFL, flags|O_NONBLOCK);

		int pid = fork();
		if( pid == 0 ){
			struct epoll_event ev,events[20];
			int epfd = epoll_create(256);
			int ev_s=0;

			ev.data.fd=accept_fd;
			ev.events=EPOLLIN|EPOLLET;
			epoll_ctl(epfd,EPOLL_CTL_ADD,accept_fd,&ev);
			for(;;){
				ev_s = epoll_wait( epfd,events,20,500 );
				int i=0;
				for(i=0; i<ev_s;i++){
					if(events[i].events&EPOLLIN){
						printf("accept event :%d\n",i);
						char in_buf[1024];
						memset(in_buf, 0, 1024);
						recv( events[i].data.fd ,&in_buf ,1024 ,0 );
						printf( "accept:%s\n", in_buf );
					}
			}
		}
	}
	else{
		//do nothing
	}

}
return 0;
}