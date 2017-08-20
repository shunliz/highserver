#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 

int main(){
	int listen_fd,accept_fd,flag,cnt;
	struct sockaddr_in my_addr,remote_addr;
	if ( (listen_fd = socket( AF_INET,SOCK_STREAM,0 )) == -1 ){
		perror("create socket error!");
		exit(1);
	}
	if ( setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&flag,sizeof(flag)) == -1 ){
		perror("setsockopt error");
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(3389);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if ( bind( listen_fd, (struct sockaddr *)&my_addr,
		sizeof(struct sockaddr_in)) == -1 ) {
			perror("bind error");
			exit(1);
	}
	if ( listen( listen_fd,1 ) == -1 ){
		perror("listen error");
		exit(1);
	}
	int pid;
	int addr_len = sizeof( struct sockaddr_in );
	for(;;){
		accept_fd = accept( listen_fd,
			(struct sockaddr *)&remote_addr,&addr_len );
		
		if(accept_fd == -1)
			break; //接收的连接不正确，重新接收。
		pid = fork();
		if(pid == 0){//子进程处理进来的连接
			for(;;){
				//由于采用多个进程处理，这个循环在子进程中处理。再有新连接进来，
				//再生成新进程处理。可以同时处理多个客户端连接。
				char in_buf[1024];
				memset(in_buf, 0, 1024);
				int ret ;
				ret = recv( accept_fd ,&in_buf ,1024 ,0 );//客户端不发送数据也要阻塞等待
				if(ret <0){//接收数据出错
					break;
				}else if(ret == 0){//客户端已经断开连接。如果不加这个判断,再次recv的数据没有意义，接收 不到数据。
					close(accept_fd);
					break;
				}else{
					printf("accept:%s\n", in_buf );
					if(strcmp(in_buf,"bye") == 0){
						close(accept_fd);
						break;
					}
				}
			}
		}else if(pid == -1){//fork 出错
			perror("fork new process error!");
			break;
		}else{
			//主进程可以做一些管理的操作。比如如果那个子进程干活不力，可以kill掉。
		}
		
	}
	return 0;
}
