#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

int main(){


	int pid = fork();
	
	if(pid == 0){
		                ///root/code/net/tcpip/server/highserver2/server9/1.txt
		int fd = open("/root/code/net/tcpip/server/highserver2/server9/1.txt",O_RDONLY);

		if(fd == -1){
			perror("open fail");
		}else{
			printf("open ok");
		}
	
	}
	
	

	return 0;
}
