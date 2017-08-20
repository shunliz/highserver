/*
1、当有一个有相同本地地址和端口的socket1处于TIME_WAIT状态时，而你启
动的程序的socket2要占用该地址和端口，你的程序就要用到该选项。 
    2、SO_REUSEADDR允许同一port上启动同一服务器的多个实例(多个进程)。但
每个实例绑定的IP地址是不能相同的。在有多块网卡或用IP Alias技术的机器可
以测试这种情况。 
    3、SO_REUSEADDR允许单个进程绑定相同的端口到多个socket上，但每个soc
ket绑定的ip地址不同。这和2很相似，区别请看UNPv1。 
    4、SO_REUSEADDR允许完全相同的地址和端口的重复绑定。但这只用于UDP的
多播，不用于TCP。
*/

#include <netinet/in.h> 
#include <sys/socket.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 

#define MAXLINE 100 

int main(int argc, char** argv) 
{ 
   int listenfd,connfd; 
   struct sockaddr_in servaddr; 
   char buff[MAXLINE+1]; 
   time_t ticks; 
   unsigned short port; 
   int flag=1,len=sizeof(int); 

   port=10013; 
   if( (listenfd=socket(AF_INET,SOCK_STREAM,0)) == -1) 
   { 
     perror("socket"); 
     exit(1); 
   } 
   bzero(&servaddr,sizeof(servaddr)); 
   servaddr.sin_family=AF_INET; 
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY); 
   servaddr.sin_port=htons(port); 
   if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) == -
1) 
   { 
      perror("setsockopt"); 
      exit(1); 
   } 
   if( bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == 
-1) 
   { 
      perror("bind"); 
      exit(1); 
   } 
   else 
      printf("bind call OK!\n"); 
   if( listen(listenfd,5) == -1) 
   { 
      perror("listen"); 
      exit(1); 
   } 
   for(;;) 
   { 
      if( (connfd=accept(listenfd,(struct sockaddr*)NULL,NULL)) == -1)

      { 
          perror("accept"); 
          exit(1); 
      } 
      if( fork() == 0)/*child process*/ 
      { 
        close(listenfd);/*这句不能少，原因请大家想想就知道了。*/ 
        ticks=time(NULL); 
        snprintf(buff,100,"%.24s\r\n",ctime(&ticks)); 
        write(connfd,buff,strlen(buff)); 
        close(connfd); 
        sleep(1); 
        execlp("server2",NULL); 
        perror("execlp"); 
        exit(1); 
     } 
     close(connfd); 
     exit(0);/* end parent*/ 
  } 
} 
