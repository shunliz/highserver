#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h>

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
	int flags = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, flags|O_NONBLOCK);
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
	fd_set fd_sets;

	int max_fd = listen_fd ;
	int events=0;
	int accept_fds[1024];
	int i = 0;

	for(;;){
		FD_ZERO( &fd_sets );
		FD_SET(listen_fd,&fd_sets);
		int k=0;
		for(k=0; k<=i; k++){
			FD_SET(accept_fds[k],&fd_sets);
		}
		events = select( max_fd + 1, &fd_sets, NULL, NULL, NULL );
		if( events ){
			printf("events:%d\n",events);
			if( FD_ISSET(listen_fd,&fd_sets) ){
				printf("listen event :1\n");
				int addr_len = sizeof( struct sockaddr_in );
				accept_fd = accept( listen_fd,
					(struct sockaddr *)&remote_addr,&addr_len );
				int flags = fcntl(accept_fd, F_GETFL, 0);
				fcntl(accept_fd, F_SETFL, flags|O_NONBLOCK);
				accept_fds[i] = accept_fd;
				i++;
				max_fd = accept_fd ;
				printf("new accept fd:%d\n",accept_fd);
			}
			int j=0;
			for( j=0; j<=i; j++ ){
				if( FD_ISSET(accept_fds[j],&fd_sets) ){
					printf("accept event :%d\n",j);
					char in_buf[1024];
					memset(in_buf, 0, 1024);
					recv( accept_fds[j] ,&in_buf ,1024 ,0 );
					printf( "accept:%s\n", in_buf );
				}
			}
		}
	}
	return 0;
}
