/***************************************************************************
               file: udp_epoll_c.cpp
              -------------------
    begin : 2006/01/17
    copyright : (C) 2005 by ’≈ºˆ¡÷
    email : zhangjianlin_8 at 126.com
***************************************************************************/

/***************************************************************************
* *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or *
* (at your option) any later version. *
* *
***************************************************************************/


#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <pthread.h>
#include <vector>
using namespace std;
int Read(int fd,void *buffer,unsigned int length) 
{ 
unsigned int nleft;
int nread;
char *ptr;
ptr = (char *)buffer;
nleft = length;
while(nleft > 0)
{
  if((nread = read(fd, ptr, nleft))< 0)
  {
   if(errno == EINTR)
    nread = 0;
   else
    return -1;
  }
  else if(nread == 0)
  {
   break;
  }
  nleft -= nread;
  ptr += nread;
}
return length - nleft;
} 

int Write(int fd,const void *buffer,unsigned int length) 
{ 
unsigned int nleft;
int nwritten;
const char *ptr;
ptr = (const char *)buffer;
nleft = length;
while(nleft > 0)
{
  if((nwritten = write(fd, ptr, nleft))<=0)
  {
   if(errno == EINTR)
    nwritten=0;
   else
    return -1;
  }
  nleft -= nwritten;
  ptr += nwritten;
}
return length;
} 
int CreateThread(void *(*start_routine)(void *), void *arg = NULL, pthread_t *thread = NULL, pthread_attr_t *pAttr = NULL)
{
pthread_attr_t thr_attr;
if(pAttr == NULL)
{
  pAttr = &thr_attr;
  pthread_attr_init(pAttr);
  pthread_attr_setstacksize(pAttr, 1024 * 1024); // 1 Mµƒ∂—’ª

   pthread_attr_setdetachstate(pAttr, PTHREAD_CREATE_DETACHED);
}
pthread_t tid;
if(thread == NULL)
{
  thread = &tid;
}
int r = pthread_create(thread, pAttr, start_routine, arg);
pthread_attr_destroy(pAttr);
return r;
}

static int SetRLimit()
{
struct rlimit rlim;
rlim.rlim_cur = 20480;
rlim.rlim_max = 20480;
if (setrlimit(RLIMIT_NOFILE, &rlim) != 0)
{
  perror("setrlimit");
}
else
{
  printf("setrlimit ok\n");
}
return 0;
}

int setnonblocking(int sock)
{
int opts;
opts=fcntl(sock,F_GETFL);
if(opts<0)
{
  return -1;
}
opts = opts|O_NONBLOCK;
if(fcntl(sock,F_SETFL,opts)<0)
{
  return -1;
} 
return 0;
}

int ConnectToUdperver(const char *host, unsigned short port)
{
int sock = socket(AF_INET, SOCK_DGRAM, 0);
if(sock < 0)
{
  perror("socket");
        return -1;
}
struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(port);
addr.sin_addr.s_addr = inet_addr(host);

if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
{
  perror("bind");
  close(sock);
     return -1;
}
return sock;
}

void *SendThread(void *arg)
{
vector<int> sockets;
sockets = *((vector<int> *)arg);

int n = 0;
char data[1024];
int i = 0;
while(1)
{
  for(vector<int>::iterator itr = sockets.begin(), last = sockets.end(); itr != last; ++itr)
  {
   sprintf(data, "test data %d\n", i++);
   n = Write(*itr, "hello", 5);
   printf("socket %d write to server[ret = %d]:%s",*itr, n, data); 
  }
  sleep(1);
}
}

int main(int argc, char **argv)
{
SetRLimit();
printf("FD_SETSIZE= %d\n", FD_SETSIZE);
if (argc != 3)
{
  printf("usage: %s <IPaddress> <PORT>\n", argv[0]);
  return 1;
}

int epfd = epoll_create(20480);
if(epfd < 0)
{
  perror("epoll_create");
  return 1;
}
struct epoll_event event;
struct epoll_event ev[20480];
vector<int> sockets;
for(int i = 0; i < 3000; i++)
{
  int sockfd = ConnectToUdperver(argv[1], (unsigned short)(atoi(argv[2])));
  if(sockfd < 0)
  {
   printf("Cannot connect udp server %s %s\n", argv[1], argv[2]);
   return 1;
  }
  
  sockets.push_back(sockfd);
  setnonblocking(sockfd);
  event.data.fd = sockfd;
     event.events = EPOLLIN|EPOLLET;
     if(0 != epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event))
  {
   perror("epoll_ctl");
  }
}
if(0 != CreateThread(SendThread, (void *)&sockets))
{
  perror("CreateThread");
  return 2;
}
int nfds = 0; 
while(1)
{
        nfds=epoll_wait(epfd,ev,20480,500);
  if(nfds < 0)
  {
   perror("epoll_wait");
   break;
  }
  else if(nfds == 0)
  {
   printf("epoll_wait timeout!\n");
   continue;
  }
  for(int i = 0; i < nfds; i++)
  {
   if(ev[i].events & EPOLLIN)
   {
    printf("can read for %d now\n", ev[i].data.fd);
    char data[1024] = {0};
    int n = read(ev[i].data.fd, data, sizeof(data));
    printf("Received %d bytes from server!\n", n);
   }
  }
}
for(vector<int>::iterator itr = sockets.begin(), last = sockets.end(); itr != last; itr++)
{
  close(*itr);
}
close(epfd);
return 0;
}