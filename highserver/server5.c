/*
һ���������ݡ�������������ģ�ͷ�����Ҫ�������¼���
��1���Ƿ�������ʽ���������Ƿ��·���ã�ʹ�����ֶ�·���ú���
��2���Ƿ���̣߳����̼߳������֯
��3���Ƿ����̣�����̵������һ�㶼��accept����ǰ
�������ࡣ���ȸ����Ƿ��·���÷�Ϊ�����ࣺ
��1������ʽģ��
��2����·����ģ��
��3��ʵʱ�ź�ģ��
������ϸ���ࡣ
�����̴߳���ģ�ͣ�
1������ʽģ�͸����Ƿ���̷߳����ࣺ
��1�����̴߳���
��2��һ������һ���̡߳�
��3��Ԥ����һ�������̣߳����������߳�������accept����
��4��Ԥ����һ�������̣߳����������߳�������acceptǰ���߳�������
��5�����̴߳���accept��Ԥ��������̣߳��̳߳أ��������ӡ�
��6��Ԥ�������߳�������accept����ÿ���߳�����Ԥ�����߳�ר�Ŵ������ӡ�

2����·����ģ�͸��ݶ�·���õ㡢�Ƿ���̷߳��ࣺ
��1��accept�����ڶ�·���ú���֮ǰ�����߳���accept��������������߳��ڶ�·���ú���������
��Ӧ���̹߳ܵ������̰߳ѹܵ��Ķ���pipefd��Ϊfd_set�ĵ�һ������������pipefd�ɶ���������ݣ�
����Ԥ�����ʽ�ֽ��clientfd����fd_set�����clientfd�ɶ�����read֮����ҵ��

�˷������Ա���select��fd_set�������ƣ����������select����֧�ֶ��ٸ���������
����ͨ����ӡsizeof(fd_set)�鿴���һ�������512�ֽڣ���֧��512��8��4096����
Ϊ��֧�ֶ���4096������������ģ���¾Ϳ��Դ���������̷ֱ߳��·���ã�
���߳�accept��ƽ�����루˳��ѭ���������̵߳Ĺܵ��С�����5�����߳������Ӧ�ܵ���
�Ϳ���֧��2w�����ӣ��㹻�ˡ���һ��������뵥�̵߳�select����һ���ӿɶ���ʱ��
�����Լ���ѭ��ɨ��fd_set�Ĵ��������߳���Ҫɨ������fd_set���������󣩣�
��ģ���£�ֻ��Ҫɨ�������̵߳�fd_set�Ϳɡ�

��2��accept�����ڶ�·���ú���֮ǰ���루1���Ĳ�����ڣ����̲߳�ֱ������߳�ͨ���ܵ�ͨѶ�����ǽ���ȡ��fd������һ�����̵߳��߳���Ϣ���У������̶߳���Ϣ���У�Ȼ��ͨ���ܵ�����߳�ͨѶ��
Ŀ�������߳��м���ϵͳ���ã��ӿ�accept�Ĵ����������ӱ�������µľܾ����ӡ�
��3����·���ú�����accept֮ǰ����·���ú������أ�����ɶ�����serverfd����accept��������read������ҵ�����Ƕ�·����ͨ�õ�ģ�ͣ�Ҳ�Ǿ����reactorģ�͡�
��4�������ڵ����߳��д���
���ϣ�1����2����3���������ڼ�⵽cliendfd�ɶ���ʱ�򣬰�������д����һ�̣߳�Ҳ�������̳߳أ����߳���Ϣ���У���һ�̣߳����̳߳أ�����read������ҵ��
��5��ҵ���̶߳����������������ȡ������֪ͨҵ���̡߳�
���ϣ�1����2����3����4���ж����Խ�ҵ���̣߳��������̳߳أ����������ȸ�֮��1������2������3������4����read�����̣߳�����1��2��4���������̳߳أ���
��Ҫ��ȡ���ַ���������־������Ҫ��ȡ���ַ�����������ȡ��������clientfd/bufferָ�����ҵ���̵߳��߳���Ϣ���У�ҵ���̶߳�ȡ��Ϣ���д���ҵ��
��Ҳ���Ǿ����proactorģ�⡣
�ܽ᣺ģ�ͣ�1������չselect������������ѡ��ģ�ͣ�2����ģ�ͣ�1���ڱ��������µĵ����汾��ģ�ͣ�3���Ǿ����reactor��epoll�ڸ�ģ�������ܾ��Ѿ��ܺã�
��select/poll��Ȼ���ڱ������ӵľܾ����������ģ�ͣ�4����5�����Ƿ���ҵ������ģ�ͣ�3�����ж��̵߳����İ汾�����и���ҵ�����������Ƽ�ģ�ͣ�5����
���ݲ�����ʾ��ʹ��epoll��ʱ��ģ�ͣ�1����2����ԣ�3��û�����Ե��������ƣ���1���������߳����ε�ϵͳ���ã����������½���

3��ʵʱ�ź�ģ�ͣ�
ʹ��fcntl��F_SETSIG���������������ɶ����ź��ɲ��ɿ���SIGIO(SYSTEM V)����SIGPOLL(BSD)���ɿɿ��źš����ɳ�Ϊ�����·���õķ�ʽ������select/poll���ر����ڴ��������Ӵ��ڵ�����£�������epoll��

4������̵Ĳ���ķ�ʽ
��1��forkģ�͡�fork�����н���ֱ����accept�������������߳���accept�����Ķ�������acceptǰforkΪ����̡�ͬ�����پ�Ⱥ���⡣
��2��forkģ�͡�fork�����н���������acceptǰ���߳�������ͬ�߳���һ�����ⲻ֧�����н���ֱ��������accept���߾�Ⱥ���⣬���н��������ڹ����ڴ���ʵ�ֵ��̻߳�������
��3��ҵ�����������Ϊ��ͬ����ģ�͡����ģ�Ϳ�������unix����ѧ��Ӱ�죬һ���������һ�����飬���ӵ�����ͨ��������̽�Ϲܵ���ɡ��Ҽ������̷�ʽ����ҵЭ��ջʵ�֡��Լ���ʱ��û��д��ģ�͵�ʾ��������ԶԱ����ܡ�
��4�����⸺��ģ�͡��������̰󶨵���ͬ�ķ���˿ڣ�ǰ�˲���lvs�Ⱦ��⸺��ϵͳ����¶һ�������ַ�����ӳ�䵽��ͬ�Ľ��̣�ʵ�ֿ���չ�Ķ���̷�����
�ܽ᣺������Ϊ��1����2��ûʲô���塣��3���ݲ����ۡ���4�����Ǿ��⸺�ط��������������з�������ͻ��
����ģ�͵Ĵ���ʾ���Լ����ܶԱȺ��������

select server:
----------------------------------------------------------------
array[slect_len];
nSock=0;
array[nSock++]=listen_fd;(֮ǰlisten port�Ѱ󶨲�listen)
maxfd=listen_fd;
while{
   FD_ZERO(&set);
   foreach (fd in array) 
   {
       fd����maxfd����maxfd=fd
       FD_SET(fd,&set)
   }
   res=select(maxfd+1,&set,0,0,0)��
   if(FD_ISSET(listen_fd,&set))
   {
       newfd=accept(listen_fd);
       array[nsock++]=newfd;
            if(--res<=0) continue
   }
   foreach �±�1��ʼ (fd in array) 
   {
       if(FD_ISSET(fd,&set))
          ִ�ж�����ز���
          ���������߹رգ���Ҫɾ����fd����array����Ӧλ�ú����һ��Ԫ�ػ����ͺã�nsock��һ
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
  ִ��accept������fds��,if(--res<=0)continue
  }
  ѭ��֮���fds��
  if(fds[i].revents&(POLLIN|POLLERR )){
  ������if(--res<=0)continue
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
ѭ��nfds����server_sockfd��accept������ִ����Ӧ����
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
    if(pid == 0){//�ӽ��̴���ҵ��
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
    else{//�����̹�����
        //manager the process
        wait(&child_process_status);
    }
   
    return 0;
}
