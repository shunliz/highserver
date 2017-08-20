/*
整个基础结构已经基本确定了
接下来做一些细节工作
首先把一些函数抽取出来.
例如prefork独立出来.socket->bind->listen独立出来

这里我们引入一个新的思路
原先由统一的函数在epoll_wait之后对events里面的fd进行处理
但是每个fd可能需要处理的方式都不同.
怎么样针对不同的fd来调用特定的函数呢?

首先在epoll_event结构中有data成员
而data的定义如下

typedef union epoll_data {
void *ptr;
int fd;
__uint32_t u32;
__uint64_t u64;
} epoll_data_t;

struct epoll_event {
__uint32_t events;      // Epoll events 
epoll_data_t data;      // User data variable 
};
可见既可以在events里面放data.fd
也可以使用data.ptr来指向一个指针
当fd有消息时内核将对应的ev变量塞入events数组的时候
如果我们只是用fd来指向注册的,那么获取数据的时候只能得到对应的fd
这样使用什么函数来处理这个fd就需要另行判断

那么如果使用ptr来指向一个结构
而结构内保存了fd以及处理这个fd所使用的函数指针
那当我们得到events数组内的事件时
就可以直接调用ptr指向的函数指针了.
这就类似Nginx中的hook函数.
在Nginx中几乎任何一种事件都会绑定其处理函数
而由模块实现距离的函数,然后在hook上去.

那么下面的代码我们就模拟这个方法:
我们建立一个数据结构来保存每个fd以及对应的处理函数

struct event_handle{
int fd;
int (* handle)(int fd);
};
handle_hook是我们为每个fd注册的处理函数
当accept获得新的accept_fd之后
我们使用

ev_handles[accept_handles].handle = handle_hook
来将对应的函数注册到对应的events内
在fd得到通知的时候
使用

(*current_handle)(current_fd)
来进行处理
*/


#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h>
#include <sys/epoll.h>

int create_listen_fd(int port){
	int listen_fd;
	struct sockaddr_in my_addr;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("create socket error");
		exit(1);
	}
	int flag;
	if (setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&flag,sizeof(flag)) == -1){
		perror("setsockopt error");
	}
	int flags = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, flags|O_NONBLOCK);
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(listen_fd, (struct sockaddr *)&my_addr,sizeof(struct sockaddr_in)) == -1) {
		perror("bind error");
		exit(1);
	}
	if (listen(listen_fd,1) == -1){
		perror("listen error");
		exit(1);
	}
	return listen_fd;
}

int create_accept_fd(int listen_fd){
	int addr_len = sizeof( struct sockaddr_in );
	struct sockaddr_in remote_addr;
	int accept_fd = accept( listen_fd,(struct sockaddr *)&remote_addr, &addr_len );
	int flags = fcntl(accept_fd, F_GETFL, 0);
	fcntl(accept_fd, F_SETFL, flags|O_NONBLOCK);
	return accept_fd;
}

int fork_process(int process_num){
	int i;
	int pid=-1;
	for(i = 0; i < process_num; i++){
		if(pid != 0){
			pid = fork();
		}
	}
	return pid;
}

int handle_normal(int socket_fd){
	char in_buf[1024];
	memset(in_buf, 0, 1024);
	int recv_num = recv( socket_fd, &in_buf, 1024, 0 );
	if( recv_num ==0 ){
		close(socket_fd);
		printf("ProcessID:%d,EPOLLIN,fd:%d,closed\n", getpid(), socket_fd);
	}
	else{
		printf("ProcessID:%d,EPOLLIN,fd:%d,recv:%s\n", getpid(), socket_fd, in_buf);
	}
	return recv_num;
}

int handle_hook(int socket_fd){
	int i=0;
	char in_buf[1024];
	memset(in_buf, 0, 1024);
	int recv_num = recv( socket_fd, &in_buf, 1024, 0 );
	if( recv_num ==0 ){
		close(socket_fd);
		printf("ProcessID:%d,EPOLLIN,fd:%d,closed\n", getpid(), socket_fd);
	}
	else{
		printf("ProcessID:%d,EPOLLIN,fd:%d,recv_num:%d;recv:", getpid(), socket_fd, recv_num);
		for (i = 0; i<recv_num;i++){
			printf("%02x ",in_buf[i]);
		}
		printf("\n");
	}
	return recv_num;
}

struct event_handle{
	int fd;
	int (* handle)(int fd);
};
typedef int (* EVENT_HANDLE)(int);
typedef struct event_handle * EH;

int main(){
	int listen_fd = create_listen_fd(3389);
	int pid = fork_process(3);
	if(pid == 0){
		int accept_handles = 0;
		struct epoll_event ev,events[20];
		int epfd = epoll_create(256);
		int ev_s = 0;

		ev.data.fd = listen_fd;
		ev.events = EPOLLIN|EPOLLET;
		epoll_ctl(epfd,EPOLL_CTL_ADD,listen_fd,&ev);
		struct event_handle ev_handles[256];
		for(;;){
			ev_s = epoll_wait( epfd,events, 20, 500 );
			int i = 0;
			for(i = 0; i<ev_s;i++){
				if(events[i].data.fd == listen_fd){
					int max_process_accept = 3;
					if(accept_handles < max_process_accept){
						accept_handles++;
						int accept_fd = create_accept_fd(listen_fd);
						ev_handles[accept_handles].fd = accept_fd;
						ev_handles[accept_handles].handle = handle_hook;
						ev.data.ptr = &ev_handles[accept_handles];
						ev.events = EPOLLIN|EPOLLET;
						epoll_ctl(epfd,EPOLL_CTL_ADD,accept_fd,&ev);
						printf("ProcessID:%d,EPOLLIN,fd:%d,accept:%d\n", getpid(), listen_fd, accept_fd);
					}
				}
				else if(events[i].events&EPOLLIN){
					EVENT_HANDLE current_handle = ((EH)(events[i].data.ptr))->handle;
					int current_fd = ((EH)(events[i].data.ptr))->fd;
					if( (*current_handle)(current_fd) == 0){
						accept_handles--;
					}
				}
				else if(events[i].events&EPOLLOUT){
					//need add write event process
				}
			}
		}
	}
	else{
		//manager the process
		int child_process_status;
		wait(&child_process_status);
	}

	return 0;
}