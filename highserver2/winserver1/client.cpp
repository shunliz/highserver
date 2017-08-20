class TCPClient
{
public:
    TCPClient()
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
        mServerAddr.sin_addr.s_addr    = inet_addr(SERVER_IP);
        mServerAddr.sin_port        = htons((u_short)SERVER_PORT);

        // ���ӵ�������
        if ( ::connect(mServerSocket, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr)))
        {
            ::closesocket(mServerSocket);
            std::cout << "���ӷ�����ʧ��!" << std::endl;
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

        std::cout << "�Ѿ��ͷ������Ͽ�����!" << std::endl;
    }

private:
    SOCKET        mServerSocket;    ///< �������׽��־��
    sockaddr_in    mServerAddr;    ///< ��������ַ
};


int _tmain(int argc, _TCHAR* argv[])
{
    TCPClient client;
    client.run();

    system("pause");
    return 0;
}