class TCPClient
{
public:
    TCPClient()
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
        mServerAddr.sin_addr.s_addr    = inet_addr(SERVER_IP);
        mServerAddr.sin_port        = htons((u_short)SERVER_PORT);

        // 连接到服务器
        if ( ::connect(mServerSocket, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr)))
        {
            ::closesocket(mServerSocket);
            std::cout << "连接服务器失败!" << std::endl;
            return;    
        }
    }

    ~TCPClient()
    {
        ::closesocket(mServerSocket);
    }

    void run()
    {
        int nRecvSize = 0;
        char buff[BUFFER_SIZE];
        memset(buff, 0, sizeof(buff) );
        while (nRecvSize = ::recv(mServerSocket, buff, BUFFER_SIZE, 0) )
        {
            if (mServerSocket == INVALID_SOCKET)
            {                
                break;
            }

            std::cout << buff << std::endl;
        }

        std::cout << "已经和服务器断开连接!" << std::endl;
    }

private:
    SOCKET        mServerSocket;    ///< 服务器套接字句柄
    sockaddr_in    mServerAddr;    ///< 服务器地址
};


int _tmain(int argc, _TCHAR* argv[])
{
    TCPClient client;
    client.run();

    system("pause");
    return 0;
}