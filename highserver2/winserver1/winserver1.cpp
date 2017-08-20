#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib" )


class WinSocketSystem
{
public:
    WinSocketSystem()
    {
        int iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
        if (iResult != NO_ERROR)
        {
            exit(-1);
        }
    }

    ~WinSocketSystem()
    {
        WSACleanup();
    }

protected:
    WSADATA wsaData;
};

static WinSocketSystem g_winsocketsystem;

class TCPServer
{
public:
    TCPServer()
        : mServerSocket(INVALID_SOCKET)
    {
        // �����׽���
        mServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (mServerSocket == INVALID_SOCKET)
        {
            std::cout << "�����׽���ʧ��!" << std::endl;
            return;
        }

        // ����������IP�Ͷ˿ں�
        mServerAddr.sin_family        = AF_INET;
        mServerAddr.sin_addr.s_addr    = INADDR_ANY;
        mServerAddr.sin_port        = htons((u_short)SERVER_PORT);

        // ��IP�Ͷ˿�
        if ( ::bind(mServerSocket, (sockaddr*)&mServerAddr, sizeof(mServerAddr)) == SOCKET_ERROR)
        {
            std::cout << "��IP�Ͷ˿�ʧ��!" << std::endl;
            return;
        }

        // �����ͻ�������,���ͬʱ����������Ϊ10.
        if ( ::listen(mServerSocket, SOMAXCONN) == SOCKET_ERROR)
        {
            std::cout << "�����˿�ʧ��!" << std::endl;
            return;
        }

        std::cout << "����TCP�������ɹ�!" << std::endl;
    }

    ~TCPServer()
    {
        ::closesocket(mServerSocket);
        std::cout << "�ر�TCP�������ɹ�!" << std::endl;
    }

    void run()
    {
        int nAcceptAddrLen = sizeof(mAcceptAddr);
        for (;;)
        {
            // ��������ʽ,�ȴ����տͻ�������
            mAcceptSocket = ::accept(mServerSocket, (struct sockaddr*)&mAcceptAddr, &nAcceptAddrLen);
            std::cout << "���ܿͻ���IP:" << inet_ntoa(mAcceptAddr.sin_addr) << std::endl;

            // ������Ϣ
            int ret = ::send(mAcceptSocket, SEND_STRING, (int)strlen(SEND_STRING), 0);
            if (ret != 0)
            {
                std::cout << "������Ϣ�ɹ�!" << std::endl;
            }

            // �رտͻ����׽���
            ::closesocket(mAcceptSocket);
            std::cout << "�Ͽ��ͻ��˶˳ɹ�!" << std::endl;
        }
    }

private:
    SOCKET        mServerSocket;    ///< �������׽��־��
    sockaddr_in    mServerAddr;    ///< ��������ַ

    SOCKET        mAcceptSocket;    ///< ���ܵĿͻ����׽��־��
    sockaddr_in    mAcceptAddr;    ///< ���յĿͻ��˵�ַ
};

int _tmain(int argc, _TCHAR* argv[])
{
    TCPServer server;
    server.run();

    return 0;
}
