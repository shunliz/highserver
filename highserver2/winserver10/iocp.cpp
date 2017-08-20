/*
当然TCP方式的模型还有事件选择模型。
就是把所有的网络事件和我们的一个程序里定义的事件梆定。
这个有它的好处，可能可以让我们更好的写一个线程来管理
接收与发送。
现在来讲一下一个完成端口模型。

  完成端口
  
 一个完成端口其实就是一个通知队列，由操作系统把已经完成的重叠I/O请求的通知
 放入其中。当某项I/O操作一旦完成，某个可以对该操作结果进行处理的工作者线程
 就会收到一则通知。而套接字在被创建后，可以在任何时候与某个完成端口进行关
 联。
 
 步骤:
 1、创建一个空的完成端口；
 2、得到本地机器的CPU个数；
 3、开启CPU*2个工作线程(又名线程池),全部都在等待完成端口的完成包;
 4、创建TCP的监听socket，使用事件邦定，创建监听线程;
 5、当有人连接进入的时候，将Client socket保存到一个我们自己定义的关键键，
    并把它与我们创建的完成端口关联；
 6、使用WSARecv和WSASend函数投递一些请求，这是使用重叠I/O的方式;
 7、重复5~6;

 注：1、重叠I/O的方式中，接收与发送数据包的时候，一定要进行投递请求这是
   它们这个体系结构的特点
   当然，在完成端口方式中，不是直接使用的WSARecv和WSASend函数进行请求
   的投递的。而是使用的ReadFile,Write的方式
  2、完成端口使用了系统内部的一些模型，所以我们只要按照一定的顺序调用就
   可以完成了。
  3、完成端口是使用在这样的情况下，有成千上万的用户连接的时候，它能够
   保证性能不会降低。

*/
 

 #include  < winsock2.h > 
#include  < windows.h > 
#include  < stdio.h > 
 
 #define  PORT 5150 
 #define  DATA_BUFSIZE 8192 
 
 // 关键项 
 typedef  struct 
 {
   OVERLAPPED Overlapped;
   WSABUF DataBuf;
   CHAR Buffer[DATA_BUFSIZE];
   DWORD BytesSEND;
   DWORD BytesRECV;
}  PER_IO_OPERATION_DATA,  *  LPPER_IO_OPERATION_DATA;


typedef  struct  
 {
   SOCKET Socket;
}  PER_HANDLE_DATA,  *  LPPER_HANDLE_DATA;

DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID);

 void  main( void )
 {
   SOCKADDR_IN InternetAddr;
   SOCKET Listen;
   SOCKET Accept;
   HANDLE CompletionPort;
   SYSTEM_INFO SystemInfo;
   LPPER_HANDLE_DATA PerHandleData;
   LPPER_IO_OPERATION_DATA PerIoData;
    int  i;
   DWORD RecvBytes;
   DWORD Flags;
   DWORD ThreadID;
   WSADATA wsaData;
   DWORD Ret;

    if  ((Ret  =  WSAStartup( 0x0202 ,  & wsaData))  !=   0 )
    {
      printf( " WSAStartup failed with error %d\n " , Ret);
       return ;
   } 
 
    // 打开一个空的完成端口 
 
    if  ((CompletionPort  =  CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL,  0 ,  0 ))  ==  NULL)
    {
      printf(  " CreateIoCompletionPort failed with error: %d\n " , GetLastError());
       return ;
   } 
 
    //  Determine how many processors are on the system. 
 
   GetSystemInfo( & SystemInfo);

    //  开启cpu个数的2倍个的线程 
 
    for (i  =   0 ; i  <  SystemInfo.dwNumberOfProcessors  *   2 ; i ++ )
    {
      HANDLE ThreadHandle;

       //  Create a server worker thread and pass the completion port to the thread. 
 
       if  ((ThreadHandle  =  CreateThread(NULL,  0 , ServerWorkerThread, CompletionPort,
          0 ,  & ThreadID))  ==  NULL)
       {
         printf( " CreateThread() failed with error %d\n " , GetLastError());
          return ;
      } 
 
       //  Close the thread handle 
       CloseHandle(ThreadHandle);
   } 
 
    // 打开一个服务器socket 
 
    if  ((Listen  =  WSASocket(AF_INET, SOCK_STREAM,  0 , NULL,  0 ,
      WSA_FLAG_OVERLAPPED))  ==  INVALID_SOCKET)
    {
      printf( " WSASocket() failed with error %d\n " , WSAGetLastError());
       return ;
   }  

   InternetAddr.sin_family  =  AF_INET;
   InternetAddr.sin_addr.s_addr  =  htonl(INADDR_ANY);
   InternetAddr.sin_port  =  htons(PORT);

    if  (bind(Listen, (PSOCKADDR)  & InternetAddr,  sizeof (InternetAddr))  ==  SOCKET_ERROR)
    {
      printf( " bind() failed with error %d\n " , WSAGetLastError());
       return ;
   } 
 
 
    if  (listen(Listen,  5 )  ==  SOCKET_ERROR)
    {
      printf( " listen() failed with error %d\n " , WSAGetLastError());
       return ;
   } 
 
    // 开始接收从客户端来的连接 
 
    while (TRUE)
    {
       if  ((Accept  =  WSAAccept(Listen, NULL, NULL, NULL,  0 ))  ==  SOCKET_ERROR)
       {
         printf( " WSAAccept() failed with error %d\n " , WSAGetLastError());
          return ;
      } 
 
       //  创建一个关键项用于保存这个客户端的信息，用户接收发送的重叠结构，
       //  还有使用到的缓冲区 
        if  ((PerHandleData  =  (LPPER_HANDLE_DATA) GlobalAlloc(GPTR, 
          sizeof (PER_HANDLE_DATA)))  ==  NULL)
       {
         printf( " GlobalAlloc() failed with error %d\n " , GetLastError());
          return ;
      } 
 
       //  Associate the accepted socket with the original completion port. 
 
      printf( " Socket number %d connected\n " , Accept);
      PerHandleData -> Socket  =  Accept;

       // 与我们的创建的那个完成端口关联起来,将关键项也与指定的一个完成端口关联 
        if  (CreateIoCompletionPort((HANDLE) Accept, CompletionPort, (DWORD) PerHandleData,
          0 )  ==  NULL)
       {
         printf( " CreateIoCompletionPort failed with error %d\n " , GetLastError());
          return ;
      } 
 
       //  投递一次接收，由于接收都需要使用这个函数来投递一个接收的准备 
 
       if  ((PerIoData  =  (LPPER_IO_OPERATION_DATA) GlobalAlloc(GPTR,           sizeof (PER_IO_OPERATION_DATA)))  ==  NULL)
       {
         printf( " GlobalAlloc() failed with error %d\n " , GetLastError());
          return ;
      } 
 
      ZeroMemory( & (PerIoData -> Overlapped),  sizeof (OVERLAPPED));
      PerIoData -> BytesSEND  =   0 ;
      PerIoData -> BytesRECV  =   0 ;
      PerIoData -> DataBuf.len  =  DATA_BUFSIZE;
      PerIoData -> DataBuf.buf  =  PerIoData -> Buffer;

      Flags  =   0 ;
       if  (WSARecv(Accept,  & (PerIoData -> DataBuf),  1 ,  & RecvBytes,  & Flags,
          & (PerIoData -> Overlapped), NULL)  ==  SOCKET_ERROR)
       {
          if  (WSAGetLastError()  !=  ERROR_IO_PENDING)
          {
            printf( " WSARecv() failed with error %d\n " , WSAGetLastError());
             return ;
         } 
      } 
   } 
} 
 // 工作线程 
 DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
 {
   HANDLE CompletionPort  =  (HANDLE) CompletionPortID;
   DWORD BytesTransferred;
   LPOVERLAPPED Overlapped;
   LPPER_HANDLE_DATA PerHandleData;
   LPPER_IO_OPERATION_DATA PerIoData;
   DWORD SendBytes, RecvBytes;
   DWORD Flags;
   
    while (TRUE)
    {
       // 完成端口有消息来了 
        if  (GetQueuedCompletionStatus(CompletionPort,  & BytesTransferred,
         (LPDWORD) & PerHandleData, (LPOVERLAPPED  * )  & PerIoData, INFINITE)  ==   0 )
       {
         printf( " GetQueuedCompletionStatus failed with error %d\n " , GetLastError());
          return   0 ;
      } 
 
 
       // 是不是有人退出了 
 
       if  (BytesTransferred  ==   0 )
       {
         printf( " Closing socket %d\n " , PerHandleData -> Socket);

          if  (closesocket(PerHandleData -> Socket)  ==  SOCKET_ERROR)
          {
            printf( " closesocket() failed with error %d\n " , WSAGetLastError());
             return   0 ;
         } 
 
         GlobalFree(PerHandleData);
         GlobalFree(PerIoData);
          continue ;
      } 
 
       //
 
       if  (PerIoData -> BytesRECV  ==   0 )
       {
         PerIoData -> BytesRECV  =  BytesTransferred;
         PerIoData -> BytesSEND  =   0 ;
      } 
       else 
        {
         PerIoData -> BytesSEND  +=  BytesTransferred;
      } 
 
       if  (PerIoData -> BytesRECV  >  PerIoData -> BytesSEND)
       {

          //  Post another WSASend() request.
          //  Since WSASend() is not gauranteed to send all of the bytes requested,
          //  continue posting WSASend() calls until all received bytes are sent. 
 
         ZeroMemory( & (PerIoData -> Overlapped),  sizeof (OVERLAPPED));

         PerIoData -> DataBuf.buf  =  PerIoData -> Buffer  +  PerIoData -> BytesSEND;
         PerIoData -> DataBuf.len  =  PerIoData -> BytesRECV  -  PerIoData -> BytesSEND;

          if  (WSASend(PerHandleData -> Socket,  & (PerIoData -> DataBuf),  1 ,  & SendBytes,  0 ,
             & (PerIoData -> Overlapped), NULL)  ==  SOCKET_ERROR)
          {
             if  (WSAGetLastError()  !=  ERROR_IO_PENDING)
             {
               printf( " WSASend() failed with error %d\n " , WSAGetLastError());
                return   0 ;
            } 
         } 
      } 
       else 
        {
         PerIoData -> BytesRECV  =   0 ;

          //  Now that there are no more bytes to send post another WSARecv() request. 
 
         Flags  =   0 ;
         ZeroMemory( & (PerIoData -> Overlapped),  sizeof (OVERLAPPED));

         PerIoData -> DataBuf.len  =  DATA_BUFSIZE;
         PerIoData -> DataBuf.buf  =  PerIoData -> Buffer;

          if  (WSARecv(PerHandleData -> Socket,  & (PerIoData -> DataBuf),  1 ,  & RecvBytes,  & Flags,
             & (PerIoData -> Overlapped), NULL)  ==  SOCKET_ERROR)
          {
             if  (WSAGetLastError()  !=  ERROR_IO_PENDING)
             {
               printf( " WSARecv() failed with error %d\n " , WSAGetLastError());
                return   0 ;
            } 
         } 
      } 
   } 
} 
