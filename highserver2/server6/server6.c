/*
上一个版本在accept的位置仍旧会被阻塞
这样当一个链接进来的时候就会产生一个新的进程
进程的不断开启和释放会造成很大的性能影响
而一般Apache和Nginx的做法就是先产生N个进程用以备用
当有实际链接的时候,就由这些进程获取并进行处理
(注:Nginx的线程模式只在Windows下有效,在Linux下是使用进程模型的)
这样我们就有两个地方需要改造

第一个是将listen端口也变为非阻塞
这样当有新链接进来的时候我们得到通知才去处理
这样accept调用就不会被阻塞在进程导致进程无法进行后续的工作

第二是进程一启动之后就fork N个进程
这些进程启动之后就自行获取各自的accept
然后自行处理各自获取的链接并管理其生命周期内的所有内容

将listen也放置到epoll中
就需要在每次获得epoll events的时候判断下
这个events是前面放进去的listen,如果是listen就要accept
如果是accept的就进行数据传输处理
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
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("create socket error");
		exit(1);
	}
	if (setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&flag,sizeof(flag)) == -1){
		perror("setsockopt error");
	}
	int flags = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, flags|O_NONBLOCK);
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(3389);
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
	int pid=-1;
	int addr_len = sizeof(struct sockaddr_in);
	int max_process_num = 3;
	int child_id;
	int i;
	int child_process_status;
	for(i = 0; i < max_process_num; i++){
		if(pid != 0){
			pid = fork();
		}
		else{
			child_id = i;
		}
	}
	if(pid == 0){
		int accept_handles = 0;
		struct epoll_event ev,events[20];
		int epfd = epoll_create(256);
		int ev_s = 0;
		ev.data.fd = listen_fd;
		ev.events = EPOLLIN|EPOLLET;
		epoll_ctl(epfd,EPOLL_CTL_ADD,listen_fd,&ev);
		for(;;){
			ev_s = epoll_wait( epfd,events, 20, 500 );
			int i = 0;
			for(i = 0; i<ev_s;i++){
				if(events[i].data.fd == listen_fd){
					int max_process_accept = 3;
					if(accept_handles < max_process_accept){
						accept_handles++;
						int addr_len = sizeof( struct sockaddr_in );
						accept_fd = accept( listen_fd,
							(struct sockaddr *)&remote_addr, &addr_len );
						int flags = fcntl(accept_fd, F_GETFL, 0);
						fcntl(accept_fd, F_SETFL, flags|O_NONBLOCK);
						ev.data.fd = accept_fd;
						ev.events = EPOLLIN|EPOLLET;
						epoll_ctl(epfd,EPOLL_CTL_ADD,accept_fd,&ev);
						printf("Child:%d,ProcessID:%d,EPOLLIN,fd:%d,accept:%d\n", child_id, getpid(), listen_fd, accept_fd);
					}
				}
				else if(events[i].events&EPOLLIN){
					char in_buf[1024];
					memset(in_buf, 0, 1024);
					int recv_num = recv( events[i].data.fd, &in_buf, 1024, 0 );
					if( recv_num ==0 ){
						close(events[i].data.fd);
						accept_handles--;
						printf("Child:%d,ProcessID:%d,EPOLLIN,fd:%d,closed\n", child_id, getpid(), events[i].data.fd);
					}
					else{
						printf("Child:%d,ProcessID:%d,EPOLLIN,fd:%d,recv:%s\n", child_id, getpid(), events[i].data.fd, in_buf);
					}
				}
			}
		}
	}
	else{
		//manager the process
		wait(&child_process_status);
	}

	return 0;
}