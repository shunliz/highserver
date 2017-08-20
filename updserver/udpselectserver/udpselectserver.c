/************SERVER**************/  
#include <winsock2.h>  
#include <stdio.h>  
#define PORT_A  11111  
#define PORT_B  22222  
void main(int argc, char **argv)  
{  
    WSADATA wsaData; // 套接口信息数据  
    SOCKET socka;   // 套接口a  
    SOCKET sockb;   // 套接口b  
      
    int nPortA = PORT_A;  
    int nPortB = PORT_B;  
    fd_set rfd;     // 读描述符集  
    timeval timeout;    // 定时变量  
    sockaddr_in addr; // 告诉sock 应该在什么地方licence  
    char recv_buf[1024];    // 接收缓冲区  
      
    int nRecLen; // 客户端地址长度!!!!!!  
      
    sockaddr_in cli;    // 客户端地址  
    int nRet; // select返回值  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  
    {  
        printf("failed to laod winsock!/n");  
        return;  
    }  
    socka = socket(AF_INET, SOCK_DGRAM, 0); // 创建数据报socka  
    if (socka == INVALID_SOCKET)  
    {  
        printf("socket()/n");  
        return;  
    }  
    sockb = socket(AF_INET, SOCK_DGRAM, 0); // 创建数据报sockb  
    if (sockb == INVALID_SOCKET)  
    {  
        printf("socket()/n");  
        return;  
    }  
    memset(&addr, 0, sizeof(addr));  
      
    addr.sin_family = AF_INET;   // IP协议  
    addr.sin_port = htons(nPortA); // 端口  
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 在本机的所有ip上开始监听  
    if (bind(socka, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)// bind socka  
    {  
        printf("bind()/n");  
        return;  
    }  
      
    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;   // IP协议  
    addr.sin_port = htons(nPortB); // 端口  
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 在本机的所有ip上开始监听  
      
    if (bind(sockb, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) // bind sockb  
    {  
        printf("bind()/n");  
        return;  
    }  
    // 设置超时时间为6s  
    timeout.tv_sec = 6;   
    timeout.tv_usec = 0;  
      
    memset(recv_buf, 0, sizeof(recv_buf)); // 清空接收缓冲区  
    while (true)  
    {  
        FD_ZERO(&rfd); // 在使用之前总是要清空  
          
        // 开始使用select  
        FD_SET(socka, &rfd); // 把socka放入要测试的描述符集中  
        FD_SET(sockb, &rfd); // 把sockb放入要测试的描述符集中  
          
        nRet = select(0, &rfd, NULL, NULL, &timeout);// 检测是否有套接口是否可读  
        if (nRet == SOCKET_ERROR)     
        {  
            printf("select()/n");  
            return;  
        }  
        else if (nRet == 0) // 超时  
        {  
            printf("timeout/n");  
            closesocket(socka);  
            closesocket(sockb);  
            break;  
        }  
        else    // 检测到有套接口可读  
        {  
            if (FD_ISSET(socka, &rfd))  // socka可读  
            {  
                nRecLen = sizeof(cli);  
                int nRecEcho = recvfrom(socka, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&cli, &nRecLen);  
                if (nRecEcho == INVALID_SOCKET)  
                {  
                    printf("recvfrom()/n");  
                    break;  
                }  
                printf("data to port 11111: %s/n", recv_buf);  
            }  
            if (FD_ISSET(sockb, &rfd)) // sockb 可读  
            {  
                nRecLen = sizeof(cli);  
                int nRecEcho = recvfrom(sockb, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&cli, &nRecLen);  
                if (nRecEcho == INVALID_SOCKET)  
                {  
                    printf("recvfrom()/n");  
                    break;  
                }  
                printf("data to port 22222: %s/n", recv_buf);  
            }  
        }  
    }  
    WSACleanup();  
}  