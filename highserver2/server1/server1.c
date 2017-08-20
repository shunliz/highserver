#include <netinet/in.h> 
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
	for(;;){
		int addr_len = sizeof( struct sockaddr_in );
		accept_fd = accept( listen_fd,
			(struct sockaddr *)&remote_addr,&addr_len );
		cnt = 0;
		if(accept_fd == -1)
			break; //接收的连接不正确，重新接收。
		for(;;){
			//在这个循环中如果再有连接进来，必须等待这个循环处理完成才能处理下一个连接（accept后也阻塞）。
			cnt++;
			char in_buf[1024];
			memset(in_buf, 0, 1024);
			recv( accept_fd ,&in_buf ,1024 ,0 );//客户端不发送数据也要阻塞等待
			printf("accept:%s\n", in_buf );
			if(cnt >5){
				close(accept_fd);
				break;
			}
		}
	}
	return 0;
}
