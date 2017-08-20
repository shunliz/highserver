
/*
整最终的形态基本上在上次的结构中就定型了
当然有些细节需要完善
不过基本上用这个结构来写service已经是OK了
那么现在就是继续细化这个结构.用来写个比较靠近实际的应用

有了链接的管理
接下来就是对通讯协议的实现
由于是从头开始写
所以协议也由我们自己来实现
先是对请求的解析
从客户端telnet传送过来的数据
回行是用/r/n结尾的
所以我们不停的接受数据
然后判断数据的最后是否是/r/n
如果是的话.就把它和以前的数据一起拼接起来
然后调用请求分析来解析指令

在event_handle结构中
我们加入了command数组
用来存放每次传输过来的数据
直至遇到以/r/n结尾的数据.然后拼接起来,输出,再清空这个数组
从头再接受新的指令

由于使用了epoll和非阻塞accept_fd
所以每次接受到的数据是零散的
需要将每次recv的数据连续的拼接到一个变量中
这就是command数组存在的理由
而command_pos用来保存的是每次拼接后数组的实际存放数据的量
也可以认为是最后一个数据所在数组中的位置
便于下次拼接
*/


#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h>
#include <sys/epoll.h>


typedef struct event_handle{
	int fd;
	int ( * handle )( struct event_handle * ev );
	char command[1024];
	int command_pos;
} EV,* EH;
typedef int ( * EVENT_HANDLE )( struct event_handle * ev );

int create_listen_fd( int port ){
	int listen_fd;
	struct sockaddr_in my_addr;
	if ( ( listen_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ){
		perror( "create socket error");
		exit(1);
	}
	int flag;
	if ( setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR,( char * )&flag, sizeof( flag ) ) == -1 ){
		perror( "setsockopt error" );
	}
	int flags = fcntl( listen_fd, F_GETFL, 0 );
	fcntl( listen_fd, F_SETFL, flags|O_NONBLOCK );
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons( port );
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero( &( my_addr.sin_zero ), 8 );
	if ( bind( listen_fd, ( struct sockaddr * )&my_addr,sizeof( struct sockaddr_in ) ) == -1 ) {
		perror( "bind error" );
		exit( 1 );
	}
	if ( listen( listen_fd, 1 ) == -1 ){
		perror( "listen error" );
		exit( 1 );
	}
	return listen_fd;
}

int create_accept_fd( int listen_fd ){
	int addr_len = sizeof( struct sockaddr_in );
	struct sockaddr_in remote_addr;
	int accept_fd = accept( listen_fd,( struct sockaddr * )&remote_addr, &addr_len );
	int flags = fcntl( accept_fd, F_GETFL, 0 );
	fcntl( accept_fd, F_SETFL, flags|O_NONBLOCK );
	return accept_fd;
}

int fork_process( int process_num ){
	int i;
	int pid=-1;
	for( i = 0; i < process_num; i++ ){
		if( pid != 0 ){
			pid = fork();
		}
	}
	return pid;
}

int handle_hook_v2( EH ev ){
	char in_buf[1024];
	memset( in_buf, 0, 1024 );
	int recv_num = recv( ev->fd, &in_buf, 1024, 0 );
	if( recv_num ==0 ){
		printf( "ProcessID:%d, EPOLLIN, fd:%d, closed\n", getpid(), ev->fd );
		printf( "recved:%s\n", ev->command );
		close( ev->fd );
	}
	else{
		printf( "ProcessID:%d, EPOLLIN, fd:%d, recv_num:%d;recv:", getpid(), ev->fd, recv_num );
		int i;
		for( i = 0; i<recv_num;i++){
			printf( "%02x ", in_buf[i] );
		}
		printf( "\n" );
		memcpy( ev->command + ev->command_pos, in_buf, recv_num );
		ev->command_pos += recv_num;
		if( recv_num == 2 && ( !memcmp( &in_buf[recv_num-2], "\r\n", 2 ) ) ){
			printf( "recved:%s\n", ev->command );
			memset( ev->command, 0, 1024 );
			ev->command_pos = 0;
		}
	}
	return recv_num;
}

int main(){
	int listen_fd = create_listen_fd( 3389 );
	int pid = fork_process( 3 );
	if( pid == 0 ){
		int accept_handles = 0;
		struct epoll_event ev, events[20];
		int epfd = epoll_create( 256 );
		int ev_s = 0;

		ev.data.fd = listen_fd;
		ev.events = EPOLLIN|EPOLLET;
		epoll_ctl( epfd, EPOLL_CTL_ADD, listen_fd, &ev );
		struct event_handle ev_handles[256];
		for( ;; ){
			ev_s = epoll_wait( epfd, events, 20, 500 );
			int i = 0;
			for( i = 0; i<ev_s;i++){
				if( events[i].data.fd == listen_fd ){
					int max_process_accept = 3;
					if( accept_handles < max_process_accept ){
						accept_handles++;
						int accept_fd = create_accept_fd( listen_fd );
						ev_handles[accept_handles].fd = accept_fd;
						ev_handles[accept_handles].handle = handle_hook_v2;
						ev_handles[accept_handles].command_pos = 0;
						memset( ev_handles[accept_handles].command, 0, 1024 );
						ev.data.ptr = &ev_handles[accept_handles];
						ev.events = EPOLLIN|EPOLLET;
						epoll_ctl( epfd, EPOLL_CTL_ADD, accept_fd, &ev );
						printf( "ProcessID:%d, EPOLLIN, fd:%d, accept:%d\n", getpid(), listen_fd, accept_fd );
					}
				}
				else if( events[i].events&EPOLLIN ){
					EVENT_HANDLE current_handle = ( ( EH )( events[i].data.ptr ) )->handle;
					EH current_event = ( EH )( events[i].data.ptr );
					if( ( *current_handle )( current_event ) == 0 ){
						accept_handles--;
					}
				}
				else if( events[i].events&EPOLLOUT ){
					//need add write event process
				}
			}
		}
	}
	else{
		//manager the process
		int child_process_status;
		wait( &child_process_status );
	}

	return 0;
}