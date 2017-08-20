/************CLIENT*************/  
#include <winsock2.h>  
#include <stdio.h>  
#include <stdlib.h>  
#define SERVER_PORT_A 11111     // 服务器端口A  
#define SERVER_PORT_B 22222     // 服务器端口B  
typedef struct tagSERVER    // 服务器  
{  
    char* ip;   // ip地址  
    int nPort;  // 端口号  
} SERVER, *PSERVER;   
int SendData(SOCKET s, char *ip, int nPort, char *pData); // 发送数据到IP:nPort  
int main(int argc, char **argv)  
{  
    int i;  
    WSADATA wsaData;        // socket数据  
    SOCKET sClient;         // 客户端套接口  
    char send_buf[] = "hello! I am LiangFei whoes SNO=06060734";    // 发送的数据内容  
    int nSend; // 发送数据后的返回值  
    // 服务器  
    SERVER sers[] = {   {"127.0.0.1", SERVER_PORT_A},   
                        {"127.0.0.1", SERVER_PORT_B} };  
    int nSerCount = sizeof(sers) / sizeof(sers[0]); // 服务器个数  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  // 启动socket  
    {  
        printf("failed to start up socket!/n");  
        return 0;  
    }  
      
    // 建立客户端数据包套接口  
    sClient = socket(AF_INET, SOCK_DGRAM, 0);  
    if (sClient == INVALID_SOCKET)  
    {  
        printf("socket()/n");  
        return 0;  
    }  
    for (i = 0; i < nSerCount; i++)  
    {  
        nSend = SendData(sClient, sers[i].ip, sers[i].nPort, send_buf); // 发送数据  
        if (nSend == 0) // 发送失败  
        {  
            return 0;  
        }  
        else if (nSend == SOCKET_ERROR) // 套接口出错  
        {  
            printf("sendto()/n");  
            return 0;  
        }  
    }  
    closesocket(sClient);   // 关闭套接口  
    WSACleanup(); // 卸载winsock  
    return 0;  
}  
int SendData(SOCKET s, char *ip, int nPort, char *pData)  
{  
    sockaddr_in ser;    // 服务器端地址     
    ser.sin_family = AF_INET;   // IP协议  
    ser.sin_port = htons(nPort);    // 端口号  
    ser.sin_addr.s_addr = inet_addr(ip);    // IP地址  
    int nLen = sizeof(ser); // 服务器地址长度  
      
    return sendto(s, pData, strlen(pData) + 1, 0, (sockaddr*)&ser, nLen);   // 向服务器发送数据   
}