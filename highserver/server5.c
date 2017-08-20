/*
一、分类依据。服务器的网络模型分类主要依据以下几点
（1）是否阻塞方式处理请求，是否多路复用，使用哪种多路复用函数
（2）是否多线程，多线程间如何组织
（3）是否多进程，多进程的切入点一般都是accept函数前
二、分类。首先根据是否多路复用分为三大类：
（1）阻塞式模型
（2）多路复用模型
（3）实时信号模型
三、详细分类。
常见线程处理模型：
1、阻塞式模型根据是否多线程分四类：
（1）单线程处理
（2）一个请求一个线程。
（3）预派生一定数量线程，并且所有线程阻塞在accept处。
（4）预派生一定数量线程，并且所有线程阻塞在accept前的线程锁处。
（5）主线程处理accept，预派生多个线程（线程池）处理连接。
（6）预派生多线程阻塞在accept处，每个线程又有预派生线程专门处理连接。

2、多路复用模型根据多路复用点、是否多线程分类：
（1）accept函数在多路复用函数之前，主线程在accept处阻塞，多个从线程在多路复用函数处阻塞
对应从线程管道，从线程把管道的读端pipefd作为fd_set的第一个描述符，如pipefd可读，则读数据，
根据预定义格式分解出clientfd放入fd_set，如果clientfd可读，则read之后处理业务。

此方法可以避免select的fd_set上限限制，具体机器上select可以支持多少个描述符，
可以通过打印sizeof(fd_set)查看，我机器上是512字节，则支持512×8＝4096个。
为了支持多余4096的连接数，此模型下就可以创建多个从线程分别多路复用，
主线程accept后平均放入（顺序循环）各个线程的管道中。创建5个从线程以其对应管道，
就可以支持2w的连接，足够了。另一方面相对与单线程的select，单一连接可读的时候，
还可以减少循环扫描fd_set的次数。单线程下要扫描所有fd_set（如果再最后），
该模型下，只需要扫描所在线程的fd_set就可。

（2）accept函数在多路复用函数之前，与（1）的差别在于，主线程不直接与从线程通过管道通讯，而是将获取的fd放入另一缓存线程的线程消息队列，缓存线程读消息队列，然后通过管道与从线程通讯。
目的在主线程中减少系统调用，加快accept的处理，避免连接爆发情况下的拒绝连接。
（3）多路复用函数在accept之前。多路复用函数返回，如果可读的是serverfd，则accept，其它则read，后处理业务，这是多路复用通用的模型，也是经典的reactor模型。
（4）连接在单独线程中处理。
以上（1）（2）（3）都可以在检测到cliendfd可读的时候，把描述符写入另一线程（也可以是线程池）的线程消息队列，另一线程（或线程池）负责read，后处理业务。
（5）业务线程独立，下面的网络层读取结束后通知业务线程。
以上（1）（2）（3）（4）中都可以将业务线程（可以是线程池）独立，事先告之（1）、（2）、（3）、（4）中read所在线程（上面1、2、4都可以是线程池），
需要读取的字符串结束标志或者需要读取的字符串个数，读取结束，则将clientfd/buffer指针放入业务线程的线程消息队列，业务线程读取消息队列处理业务。
这也就是经典的proactor模拟。
总结：模型（1）是拓展select处理能力不错选择；模型（2）是模型（1）在爆发连接下的调整版本；模型（3）是经典的reactor，epoll在该模型下性能就已经很好，
而select/poll仍然存在爆发连接的拒绝连接情况；模型（4）（5）则是方便业务处理，对模型（3）进行多线程调整的版本。带有复杂业务处理的情况下推荐模型（5）。
根据测试显示，使用epoll的时候，模型（1）（2）相对（3）没有明显的性能优势，（1）由于主线程两次的系统调用，反而性能下降。

3、实时信号模型：
使用fcntl的F_SETSIG操作，把描述符可读的信号由不可靠的SIGIO(SYSTEM V)或者SIGPOLL(BSD)换成可靠信号。即可成为替代多路复用的方式。优于select/poll，特别是在大量死连接存在的情况下，但不及epoll。

4、多进程的参与的方式
（1）fork模型。fork后所有进程直接在accept阻塞。以上主线程在accept阻塞的都可以在accept前fork为多进程。同样面临惊群问题。
（2）fork模型。fork后所有进程阻塞在accept前的线程锁处。同线程中一样避免不支持所有进程直接阻塞在accept或者惊群问题，所有进程阻塞在共享内存上实现的线程互斥锁。
（3）业务和网络层分离为不同进程模型。这个模型可能是受unix简单哲学的影响，一个进程完成一件事情，复杂的事情通过多个进程结合管道完成。我见过进程方式的商业协议栈实现。自己暂时还没有写该模型的示例程序测试对比性能。
（4）均衡负载模型。起多个进程绑定到不同的服务端口，前端部署lvs等均衡负载系统，暴露一个网络地址，后端映射到不同的进程，实现可扩展的多进程方案。
总结：个人认为（1）（2）没什么意义。（3）暂不评价。（4）则是均衡负载方案，和以上所有方案不冲突。
以上模型的代码示例以及性能对比后面给出。

select server:
----------------------------------------------------------------
array[slect_len];
nSock=0;
array[nSock++]=listen_fd;(之前listen port已绑定并listen)
maxfd=listen_fd;
while{
   FD_ZERO(&set);
   foreach (fd in array) 
   {
       fd大于maxfd，则maxfd=fd
       FD_SET(fd,&set)
   }
   res=select(maxfd+1,&set,0,0,0)；
   if(FD_ISSET(listen_fd,&set))
   {
       newfd=accept(listen_fd);
       array[nsock++]=newfd;
            if(--res<=0) continue
   }
   foreach 下标1开始 (fd in array) 
   {
       if(FD_ISSET(fd,&set))
          执行读等相关操作
          如果错误或者关闭，则要删除该fd，将array中相应位置和最后一个元素互换就好，nsock减一
             if(--res<=0) continue

   }
}
------------------------------------------------------------------------

poll server:

----------------------------------------------------------------------
struct pollfd fds[POLL_LEN];
unsigned int nfds=0;
fds[0].fd=server_sockfd;
fds[0].events=POLLIN|POLLPRI;
nfds++;
while{
  res=poll(fds,nfds,-1);
  if(fds[0].revents&(POLLIN|POLLPRI)){
  执行accept并加入fds中,if(--res<=0)continue
  }
  循环之后的fds，
  if(fds[i].revents&(POLLIN|POLLERR )){
  操作略if(--res<=0)continue
  }
}
----------------------------------------------------------------------

epoll server
------------------------------------------------------------------------
epollfd=epoll_create(EPOLL_LEN);
epoll_ctl(epollfd,EPOLL_CTL_ADD,server_sockfd,&ev)
struct epoll_event events[EPOLL_MAX_EVENT];
while
{
nfds=epoll_wait(epollfd,events,EPOLL_MAX_EVENT,-1);
循环nfds，是server_sockfd则accept，否则执行响应操作
}
------------------------------------------------------------------------
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_PROCESSES = 3
int main(){
    int listen_fd,accept_fd,flag;
    struct sockaddr_in my_addr,remote_addr;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("create socket error");
        exit(1);
    }
    if (setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR
                    ,(char *)&flag,sizeof(flag)) == -1){
        perror("setsockopt error");
    }
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags|O_NONBLOCK);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3389);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);
    if (bind(listen_fd, (struct sockaddr *)&my_addr,
                sizeof(struct sockaddr_in)) == -1) {
        perror("bind error");
        exit(1);
    }
    if (listen(listen_fd,1) == -1){
        perror("listen error");
        exit(1);
    }
    int pid=-1;
    int addr_len = sizeof(struct sockaddr_in);
    int max_process_num = MAX_PROCESSES;
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
    if(pid == 0){//子进程处理业务
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
            for(i = 0; i<ev_s; i++){
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
    else{//父进程管理工作
        //manager the process
        wait(&child_process_status);
    }
   
    return 0;
}
