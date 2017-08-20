/************CLIENT*************/  
#include <winsock2.h>  
#include <stdio.h>  
#include <stdlib.h>  
#define SERVER_PORT_A 11111     // �������˿�A  
#define SERVER_PORT_B 22222     // �������˿�B  
typedef struct tagSERVER    // ������  
{  
    char* ip;   // ip��ַ  
    int nPort;  // �˿ں�  
} SERVER, *PSERVER;   
int SendData(SOCKET s, char *ip, int nPort, char *pData); // �������ݵ�IP:nPort  
int main(int argc, char **argv)  
{  
    int i;  
    WSADATA wsaData;        // socket����  
    SOCKET sClient;         // �ͻ����׽ӿ�  
    char send_buf[] = "hello! I am LiangFei whoes SNO=06060734";    // ���͵���������  
    int nSend; // �������ݺ�ķ���ֵ  
    // ������  
    SERVER sers[] = {   {"127.0.0.1", SERVER_PORT_A},   
                        {"127.0.0.1", SERVER_PORT_B} };  
    int nSerCount = sizeof(sers) / sizeof(sers[0]); // ����������  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  // ����socket  
    {  
        printf("failed to start up socket!/n");  
        return 0;  
    }  
      
    // �����ͻ������ݰ��׽ӿ�  
    sClient = socket(AF_INET, SOCK_DGRAM, 0);  
    if (sClient == INVALID_SOCKET)  
    {  
        printf("socket()/n");  
        return 0;  
    }  
    for (i = 0; i < nSerCount; i++)  
    {  
        nSend = SendData(sClient, sers[i].ip, sers[i].nPort, send_buf); // ��������  
        if (nSend == 0) // ����ʧ��  
        {  
            return 0;  
        }  
        else if (nSend == SOCKET_ERROR) // �׽ӿڳ���  
        {  
            printf("sendto()/n");  
            return 0;  
        }  
    }  
    closesocket(sClient);   // �ر��׽ӿ�  
    WSACleanup(); // ж��winsock  
    return 0;  
}  
int SendData(SOCKET s, char *ip, int nPort, char *pData)  
{  
    sockaddr_in ser;    // �������˵�ַ     
    ser.sin_family = AF_INET;   // IPЭ��  
    ser.sin_port = htons(nPort);    // �˿ں�  
    ser.sin_addr.s_addr = inet_addr(ip);    // IP��ַ  
    int nLen = sizeof(ser); // ��������ַ����  
      
    return sendto(s, pData, strlen(pData) + 1, 0, (sockaddr*)&ser, nLen);   // ���������������   
}