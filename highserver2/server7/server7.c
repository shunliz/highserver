/*
���������ṹ�Ѿ�����ȷ����
��������һЩϸ�ڹ���
���Ȱ�һЩ������ȡ����.
����prefork��������.socket->bind->listen��������

������������һ���µ�˼·
ԭ����ͳһ�ĺ�����epoll_wait֮���events�����fd���д���
����ÿ��fd������Ҫ����ķ�ʽ����ͬ.
��ô����Բ�ͬ��fd�������ض��ĺ�����?

������epoll_event�ṹ����data��Ա
��data�Ķ�������

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
�ɼ��ȿ�����events�����data.fd
Ҳ����ʹ��data.ptr��ָ��һ��ָ��
��fd����Ϣʱ�ں˽���Ӧ��ev��������events�����ʱ��
�������ֻ����fd��ָ��ע���,��ô��ȡ���ݵ�ʱ��ֻ�ܵõ���Ӧ��fd
����ʹ��ʲô�������������fd����Ҫ�����ж�

��ô���ʹ��ptr��ָ��һ���ṹ
���ṹ�ڱ�����fd�Լ��������fd��ʹ�õĺ���ָ��
�ǵ����ǵõ�events�����ڵ��¼�ʱ
�Ϳ���ֱ�ӵ���ptrָ��ĺ���ָ����.
�������Nginx�е�hook����.
��Nginx�м����κ�һ���¼�������䴦����
����ģ��ʵ�־���ĺ���,Ȼ����hook��ȥ.

��ô����Ĵ������Ǿ�ģ���������:
���ǽ���һ�����ݽṹ������ÿ��fd�Լ���Ӧ�Ĵ�����

struct event_handle{
int fd;
int (* handle)(int fd);
};
handle_hook������Ϊÿ��fdע��Ĵ�����
��accept����µ�accept_fd֮��
����ʹ��

ev_handles[accept_handles].handle = handle_hook
������Ӧ�ĺ���ע�ᵽ��Ӧ��events��
��fd�õ�֪ͨ��ʱ��
ʹ��

(*current_handle)(current_fd)
�����д���
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