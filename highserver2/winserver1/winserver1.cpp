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
        // 创建套接字
        mServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (mServerSocket == INVALID_SOCKET)
        {
            std::cout << "创建套接字失败!" << std::endl;
            return;
        }

        // 填充服务器的IP和端口号
        mServerAddr.sin_family        = AF_INET;
        mServerAddr.sin_addr.s_addr    = INADDR_ANY;
        mServerAddr.sin_port        = htons((u_short)SERVER_PORT);

        // 绑定IP和端口
        if ( ::bind(mServerSocket, (sockaddr*)&mServerAddr, sizeof(mServerAddr)) == SOCKET_ERROR)
        {
            std::cout << "绑定IP和端口失败!" << std::endl;
            return;
        }

        // 监听客户端请求,最大同时连接数设置为10.
        if ( ::listen(mServerSocket, SOMAXCONN) == SOCKET_ERROR)
        {
            std::cout << "监听端口失败!" << std::endl;
            return;
        }

        std::cout << "启动TCP服务器成功!" << std::endl;
    }

    ~TCPServer()
    {
        ::closesocket(mServerSocket);
        std::cout << "关闭TCP服务器成功!" << std::endl;
    }

    void run()
    {
        int nAcceptAddrLen = sizeof(mAcceptAddr);
        for (;;)
        {
            // 以阻塞方式,等待接收客户端连接
            mAcceptSocket = ::accept(mServerSocket, (struct sockaddr*)&mAcceptAddr, &nAcceptAddrLen);
            std::cout << "接受客户端IP:" << inet_ntoa(mAcceptAddr.sin_addr) << std::endl;

            // 发送消息
            int ret = ::send(mAcceptSocket, SEND_STRING, (int)strlen(SEND_STRING), 0);
            if (ret != 0)
            {
                std::cout << "发送消息成功!" << std::endl;
            }

            // 关闭客户端套接字
            ::closesocket(mAcceptSocket);
            std::cout << "断开客户端端成功!" << std::endl;
        }
    }

private:
    SOCKET        mServerSocket;    ///< 服务器套接字句柄
    sockaddr_in    mServerAddr;    ///< 服务器地址

    SOCKET        mAcceptSocket;    ///< 接受的客户端套接字句柄
    sockaddr_in    mAcceptAddr;    ///< 接收的客户端地址
};

int _tmain(int argc, _TCHAR* argv[])
{
    TCPServer server;
    server.run();

    return 0;
}
