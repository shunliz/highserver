/*前一个版本很重要的BUG
就是sendfile在发送大文件的时候会发送不完整
这个bug指出了另一需求
就是需要用到EPOLLOUT事件

前面版本我们事件处理都是在EPOLLIN中进行
当有accept_fd数据进来之后
我们判断指令的内容
再直接进行数据的处理

我们更换一种方式
当accept_fd获取到数据之后
解析数据.
当需要输出的时候,将accept_fd的事件变为EPOLLOUT

这样一种fd会有两种状态
接受指令状态以及输出数据状态
慢慢的.我们会发现这个程序越来越像Nginx或者Lighttpd了
因为一个连接会有不同的状态
事件+状态机就是Nginx以及Lighttpd高效的原因
将一个链接分成不同的生命周期然后处理

经过进化
我们的event_handle结构拥有了读写两种hook钩子

typedef struct event_handle{
    ...
    int ( * read_handle  )( struct event_handle * ev );
    int ( * write_handle )( struct event_handle * ev );
    ...
}
而我们在针对的处理过程中
初始化时会针对不同的事件挂上不同的钩子函数

int init_evhandle(...){
    ...
    ev->read_handle = r_handle;
    ev->write_handle = w_handle;
    ...
}
接着在事件发生时调用不同的钩子函数

else if( events[i].events&EPOLLIN ){
    EVENT_HANDLE current_handle = ( ( EH )( events[i].data.ptr ) )->read_handle;
    ...
}
else if( events[i].events&EPOLLOUT ){
    EVENT_HANDLE current_handle = ( ( EH )( events[i].data.ptr ) )->write_handle;
    ...
}
当分析完指令的时候
将fd变为EPOLLOUT

int parse_request(
    ...
    ev_temp.data.ptr = ev;
    ev_temp.events = EPOLLOUT|EPOLLET;
    epoll_ctl( ev->epoll_fd, EPOLL_CTL_MOD, ev->socket_fd, &ev_temp );
    ...
}
加入这些处理之后
代码越来越长了
我们还需要加很多东西
比如进程管理,等等
这之后会发现一个mini的nginx骨架或者lighttpd骨架出现了

*/

#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include "log.h"
#include "Locker.h"
#include "locker_pthread.h"

#define HANDLE_INFO   1
#define HANDLE_SEND   2
#define HANDLE_DEL    3
#define HANDLE_CLOSE  4

#define MAX_REQLEN          1024
#define MAX_PROCESS_CONN    3
#define FIN_CHAR            0x00
#define SUCCESS  0
#define ERROR   -1

//whole system log
Logger* log = NULL;

typedef struct event_handle{
    int socket_fd;
    int file_fd;
    int file_pos;
    int epoll_fd;
    char request[MAX_REQLEN];
    int request_len;
    int ( * read_handle )( struct event_handle * ev );
    int ( * write_handle )( struct event_handle * ev );
    int handle_method;
} EV,* EH;
typedef int ( * EVENT_HANDLE )( struct event_handle * ev );

int create_listen_fd( int port ){
    int listen_fd;
    struct sockaddr_in my_addr;
    if( ( listen_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ){
        perror( "create socket error" );
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"create socket error");
        exit( 1 );
    }
    int flag;
    int olen = sizeof(int);
    if( setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR , (const void *)&flag, olen ) == -1 ){
        perror( "setsockopt error" );
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"setsockopt SO_REUSEADDR error");
    }
    flag = 5;
    if( setsockopt( listen_fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &flag, olen ) == -1 ){
        perror( "setsockopt error" );
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"setsockopt TCP_DEFER_ACCEPT error");
    }
    flag = 1;
    if( setsockopt( listen_fd, IPPROTO_TCP, TCP_CORK, &flag, olen ) == -1 ){
        perror( "setsockopt error" );
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"setsockopt TCP_CORK error");
    }
    int flags = fcntl( listen_fd, F_GETFL, 0 );
    fcntl( listen_fd, F_SETFL, flags|O_NONBLOCK );
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons( port );
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero( &( my_addr.sin_zero ), 8 );
    if( bind( listen_fd, ( struct sockaddr * )&my_addr,sizeof( struct sockaddr_in ) ) == -1 ) {
        perror( "bind error" );
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"bind error");
        exit( 1 );
    }
    if( listen( listen_fd, 1 ) == -1 ){
        perror( "listen error" );
		log->error(log,__FILE__ ,__LINE__,__FUNCTION__,"listen error");
        exit( 1 );
    }
    return listen_fd;
}

int create_accept_fd( int listen_fd ){
    int addr_len = sizeof( struct sockaddr_in );
    struct sockaddr_in remote_addr;
    int accept_fd = accept( listen_fd,
        ( struct sockaddr * )&remote_addr, &addr_len );
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"accept a connection %d",accept_fd);
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

int init_evhandle(EH ev,int socket_fd,int epoll_fd,EVENT_HANDLE r_handle,EVENT_HANDLE w_handle){
    ev->epoll_fd = epoll_fd;
    ev->socket_fd = socket_fd;
    ev->read_handle = r_handle;
    ev->write_handle = w_handle;
    ev->file_pos = 0;
    ev->request_len = 0;
    ev->handle_method = 0;
    memset( ev->request, 0, 1024 );
}
//accept->accept_queue->request->request_queue->output->output_queue
//multi process sendfile
int parse_request(EH ev){
    ev->request_len--;
    *( ev->request + ev->request_len - 1 ) = 0x00;
    int i;
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"before split the ev->request is:%s",ev->request);
    for( i=0; i<ev->request_len; i++ ){
        if( ev->request[i] == ':' ){
            ev->request_len = ev->request_len-i-1;
            char temp[MAX_REQLEN];
            memcpy( temp, ev->request, i );
            ev->handle_method = atoi( temp );
			log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"ev->handle_method is:%d",ev->handle_method);
            memcpy( temp, ev->request+i+1, ev->request_len );
            memcpy( ev->request, temp, ev->request_len );
			log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"ev->request is:%s",ev->request);
            break;
        }
    }
    //handle_request( ev );
    //register to epoll EPOLLOUT

    struct epoll_event ev_temp;
    ev_temp.data.ptr = ev;
    ev_temp.events = EPOLLOUT|EPOLLET;
    epoll_ctl( ev->epoll_fd, EPOLL_CTL_MOD, ev->socket_fd, &ev_temp );
    return SUCCESS;
}

int handle_request(EH ev){
    struct stat file_info;
    switch( ev->handle_method ){
        case HANDLE_INFO:
            ev->file_fd = open( ev->request, O_RDONLY );
            if( ev->file_fd == -1 ){
                send( ev->socket_fd, "open file failed\n", strlen("open file failed\n"), 0 );
                return -1;
            }
            fstat(ev->file_fd, &file_info);
            char info[MAX_REQLEN];
            sprintf(info,"file len:%d\n",file_info.st_size);
            send( ev->socket_fd, info, strlen( info ), 0 );
            break;
        case HANDLE_SEND:
            ev->file_fd = open( ev->request, O_RDONLY );
            if( ev->file_fd == -1 ){
                send( ev->socket_fd, "open file failed\n", strlen("open file failed\n"), 0 );
                return -1;
            }
            fstat(ev->file_fd, &file_info);
            sendfile( ev->socket_fd, ev->file_fd, 0, file_info.st_size );
            break;
        case HANDLE_DEL:
            break;
        case HANDLE_CLOSE:
            break;
    }
    finish_request( ev );
    return SUCCESS;
}

int finish_request(EH ev){
    close(ev->socket_fd);
    close(ev->file_fd);
    ev->handle_method = -1;
    clean_request( ev );
    return SUCCESS;
}

int clean_request(EH ev){
    memset( ev->request, 0, MAX_REQLEN );
    ev->request_len = 0;
}

int read_hook_v2( EH ev ){
    char in_buf[MAX_REQLEN];
    memset( in_buf, 0, MAX_REQLEN );
    int recv_num = recv( ev->socket_fd, &in_buf, MAX_REQLEN, 0 );
    if( recv_num ==0 ){
        close( ev->socket_fd );
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"read from socket %d data error",ev->socket_fd);
        return ERROR;
    }
    else{
        //check ifoverflow
        if( ev->request_len > MAX_REQLEN-recv_num ){
            close( ev->socket_fd );
            clean_request( ev );
        }
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"read from socket %d data:%s",ev->socket_fd,in_buf);
        memcpy( ev->request + ev->request_len, in_buf, recv_num );
        ev->request_len += recv_num;
		
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"recv_num=%d,&in_buf[recv_num-2]=%s",recv_num,&in_buf[recv_num-2]);
		
        if( recv_num == 2 && ( !memcmp( &in_buf[recv_num-2], "\r\n", 2 ) ) ){
			log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"if anyone here????");
            parse_request(ev);
        }
    }
    return recv_num;
}

int write_hook_v1( EH ev ){
    struct stat file_info;
    ev->file_fd = open( ev->request, O_RDONLY );
	perror("opern file error");
	log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"try to open file:%s",ev->request);
    if( ev->file_fd == ERROR ){
        send( ev->socket_fd, "open file failed\n", strlen("open file failed\n"), 0 );
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"open file failed");
        return ERROR;
    }
    fstat(ev->file_fd, &file_info);
    int write_num;
    while(1){
		log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"begin to sendfile block");
        write_num = sendfile( ev->socket_fd, ev->file_fd, (off_t *)&ev->file_pos, 10240 );
        ev->file_pos += write_num;
        if( write_num == ERROR ){
            if( errno == EAGAIN ){
                break;
            }
        }
        else if( write_num == 0 ){
            printf( "writed:%d\n", ev->file_pos );
            //finish_request( ev );
			log->debug(log,__FILE__ ,__LINE__,__FUNCTION__,"sendfile error at pos:%d",ev->file_pos);
            break;
        }
    }
    return SUCCESS;
}

int main(){

	Locker* locker = locker_pthread_create();
	log = create_log("server9.log", locker, 6);
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
            for( i = 0; i<ev_s; i++ ){
                if( events[i].data.fd == listen_fd ){
                    if( accept_handles < MAX_PROCESS_CONN ){
                        accept_handles++;
                        int accept_fd = create_accept_fd( listen_fd );
                        init_evhandle(&ev_handles[accept_handles],accept_fd,epfd,read_hook_v2,write_hook_v1);
                        ev.data.ptr = &ev_handles[accept_handles];
                        ev.events = EPOLLIN|EPOLLET;
                        epoll_ctl( epfd, EPOLL_CTL_ADD, accept_fd, &ev );
                    }
                }
                else if( events[i].events&EPOLLIN ){
                    EVENT_HANDLE current_handle = ( ( EH )( events[i].data.ptr ) )->read_handle;
                    EH current_event = ( EH )( events[i].data.ptr );
                    ( *current_handle )( current_event );
                }
                else if( events[i].events&EPOLLOUT ){
                    EVENT_HANDLE current_handle = ( ( EH )( events[i].data.ptr ) )->write_handle;
                    EH current_event = ( EH )( events[i].data.ptr );
                    if( ( *current_handle )( current_event )  == 0 ){
                        accept_handles--;
                    }
                }
            }
        }
    }
    else{
        //manager the process
        int child_process_status;
        wait( &child_process_status );
    }

    return SUCCESS;
}