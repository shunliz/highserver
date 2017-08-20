#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 

int main(){
	int listen_fd,accept_fd,flag,cnt;
	struct sockaddr_in my_addr,remote_addr;
	if ( (listen_fd = socket( AF_INET,SOCK_STREAM,0 )) == -1 ){
		perror("create socket error!");
		exit(1);
	}
	if ( setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&flag,sizeof(flag)) == -1 ){
		perror("setsockopt error");
	}
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
	int pid;
	int addr_len = sizeof( struct sockaddr_in );
	for(;;){
		accept_fd = accept( listen_fd,
			(struct sockaddr *)&remote_addr,&addr_len );
		
		if(accept_fd == -1)
			break; //���յ����Ӳ���ȷ�����½��ա�
		pid = fork();
		if(pid == 0){//�ӽ��̴������������
			for(;;){
				//���ڲ��ö�����̴������ѭ�����ӽ����д������������ӽ�����
				//�������½��̴�������ͬʱ�������ͻ������ӡ�
				char in_buf[1024];
				memset(in_buf, 0, 1024);
				int ret ;
				ret = recv( accept_fd ,&in_buf ,1024 ,0 );//�ͻ��˲���������ҲҪ�����ȴ�
				if(ret <0){//�������ݳ���
					break;
				}else if(ret == 0){//�ͻ����Ѿ��Ͽ����ӡ������������ж�,�ٴ�recv������û�����壬���� �������ݡ�
					close(accept_fd);
					break;
				}else{
					printf("accept:%s\n", in_buf );
					if(strcmp(in_buf,"bye") == 0){
						close(accept_fd);
						break;
					}
				}
			}
		}else if(pid == -1){//fork ����
			perror("fork new process error!");
			break;
		}else{
			//�����̿�����һЩ����Ĳ�������������Ǹ��ӽ��̸ɻ��������kill����
		}
		
	}
	return 0;
}
