/************SERVER**************/  
#include <winsock2.h>  
#include <stdio.h>  
#define PORT_A  11111  
#define PORT_B  22222  
void main(int argc, char **argv)  
{  
    WSADATA wsaData; // �׽ӿ���Ϣ����  
    SOCKET socka;   // �׽ӿ�a  
    SOCKET sockb;   // �׽ӿ�b  
      
    int nPortA = PORT_A;  
    int nPortB = PORT_B;  
    fd_set rfd;     // ����������  
    timeval timeout;    // ��ʱ����  
    sockaddr_in addr; // ����sock Ӧ����ʲô�ط�licence  
    char recv_buf[1024];    // ���ջ�����  
      
    int nRecLen; // �ͻ��˵�ַ����!!!!!!  
      
    sockaddr_in cli;    // �ͻ��˵�ַ  
    int nRet; // select����ֵ  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  
    {  
        printf("failed to laod winsock!/n");  
        return;  
    }  
    socka = socket(AF_INET, SOCK_DGRAM, 0); // �������ݱ�socka  
    if (socka == INVALID_SOCKET)  
    {  
        printf("socket()/n");  
        return;  
    }  
    sockb = socket(AF_INET, SOCK_DGRAM, 0); // �������ݱ�sockb  
    if (sockb == INVALID_SOCKET)  
    {  
        printf("socket()/n");  
        return;  
    }  
    memset(&addr, 0, sizeof(addr));  
      
    addr.sin_family = AF_INET;   // IPЭ��  
    addr.sin_port = htons(nPortA); // �˿�  
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // �ڱ���������ip�Ͽ�ʼ����  
    if (bind(socka, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)// bind socka  
    {  
        printf("bind()/n");  
        return;  
    }  
      
    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;   // IPЭ��  
    addr.sin_port = htons(nPortB); // �˿�  
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // �ڱ���������ip�Ͽ�ʼ����  
      
    if (bind(sockb, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) // bind sockb  
    {  
        printf("bind()/n");  
        return;  
    }  
    // ���ó�ʱʱ��Ϊ6s  
    timeout.tv_sec = 6;   
    timeout.tv_usec = 0;  
      
    memset(recv_buf, 0, sizeof(recv_buf)); // ��ս��ջ�����  
    while (true)  
    {  
        FD_ZERO(&rfd); // ��ʹ��֮ǰ����Ҫ���  
          
        // ��ʼʹ��select  
        FD_SET(socka, &rfd); // ��socka����Ҫ���Ե�����������  
        FD_SET(sockb, &rfd); // ��sockb����Ҫ���Ե�����������  
          
        nRet = select(0, &rfd, NULL, NULL, &timeout);// ����Ƿ����׽ӿ��Ƿ�ɶ�  
        if (nRet == SOCKET_ERROR)     
        {  
            printf("select()/n");  
            return;  
        }  
        else if (nRet == 0) // ��ʱ  
        {  
            printf("timeout/n");  
            closesocket(socka);  
            closesocket(sockb);  
            break;  
        }  
        else    // ��⵽���׽ӿڿɶ�  
        {  
            if (FD_ISSET(socka, &rfd))  // socka�ɶ�  
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
            if (FD_ISSET(sockb, &rfd)) // sockb �ɶ�  
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