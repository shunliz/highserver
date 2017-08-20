/*
��ȻTCP��ʽ��ģ�ͻ����¼�ѡ��ģ�͡�
���ǰ����е������¼������ǵ�һ�������ﶨ����¼��𶨡�
��������ĺô������ܿ��������Ǹ��õ�дһ���߳�������
�����뷢�͡�
��������һ��һ����ɶ˿�ģ�͡�

  ��ɶ˿�
  
 һ����ɶ˿���ʵ����һ��֪ͨ���У��ɲ���ϵͳ���Ѿ���ɵ��ص�I/O�����֪ͨ
 �������С���ĳ��I/O����һ����ɣ�ĳ�����ԶԸò���������д���Ĺ������߳�
 �ͻ��յ�һ��֪ͨ�����׽����ڱ������󣬿������κ�ʱ����ĳ����ɶ˿ڽ��й�
 ����
 
 ����:
 1������һ���յ���ɶ˿ڣ�
 2���õ����ػ�����CPU������
 3������CPU*2�������߳�(�����̳߳�),ȫ�����ڵȴ���ɶ˿ڵ���ɰ�;
 4������TCP�ļ���socket��ʹ���¼�������������߳�;
 5�����������ӽ����ʱ�򣬽�Client socket���浽һ�������Լ�����Ĺؼ�����
    �����������Ǵ�������ɶ˿ڹ�����
 6��ʹ��WSARecv��WSASend����Ͷ��һЩ��������ʹ���ص�I/O�ķ�ʽ;
 7���ظ�5~6;

 ע��1���ص�I/O�ķ�ʽ�У������뷢�����ݰ���ʱ��һ��Ҫ����Ͷ����������
   ���������ϵ�ṹ���ص�
   ��Ȼ������ɶ˿ڷ�ʽ�У�����ֱ��ʹ�õ�WSARecv��WSASend������������
   ��Ͷ�ݵġ�����ʹ�õ�ReadFile,Write�ķ�ʽ
  2����ɶ˿�ʹ����ϵͳ�ڲ���һЩģ�ͣ���������ֻҪ����һ����˳����þ�
   ��������ˡ�
  3����ɶ˿���ʹ��������������£��г�ǧ������û����ӵ�ʱ�����ܹ�
   ��֤���ܲ��ή�͡�

*/
 

 #include  < winsock2.h > 
#include  < windows.h > 
#include  < stdio.h > 
 
 #define  PORT 5150 
 #define  DATA_BUFSIZE 8192 
 
 // �ؼ��� 
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
 
    // ��һ���յ���ɶ˿� 
 
    if  ((CompletionPort  =  CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL,  0 ,  0 ))  ==  NULL)
    {
      printf(  " CreateIoCompletionPort failed with error: %d\n " , GetLastError());
       return ;
   } 
 
    //  Determine how many processors are on the system. 
 
   GetSystemInfo( & SystemInfo);

    //  ����cpu������2�������߳� 
 
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
 
    // ��һ��������socket 
 
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
 
    // ��ʼ���մӿͻ����������� 
 
    while (TRUE)
    {
       if  ((Accept  =  WSAAccept(Listen, NULL, NULL, NULL,  0 ))  ==  SOCKET_ERROR)
       {
         printf( " WSAAccept() failed with error %d\n " , WSAGetLastError());
          return ;
      } 
 
       //  ����һ���ؼ������ڱ�������ͻ��˵���Ϣ���û����շ��͵��ص��ṹ��
       //  ����ʹ�õ��Ļ����� 
        if  ((PerHandleData  =  (LPPER_HANDLE_DATA) GlobalAlloc(GPTR, 
          sizeof (PER_HANDLE_DATA)))  ==  NULL)
       {
         printf( " GlobalAlloc() failed with error %d\n " , GetLastError());
          return ;
      } 
 
       //  Associate the accepted socket with the original completion port. 
 
      printf( " Socket number %d connected\n " , Accept);
      PerHandleData -> Socket  =  Accept;

       // �����ǵĴ������Ǹ���ɶ˿ڹ�������,���ؼ���Ҳ��ָ����һ����ɶ˿ڹ��� 
        if  (CreateIoCompletionPort((HANDLE) Accept, CompletionPort, (DWORD) PerHandleData,
          0 )  ==  NULL)
       {
         printf( " CreateIoCompletionPort failed with error %d\n " , GetLastError());
          return ;
      } 
 
       //  Ͷ��һ�ν��գ����ڽ��ն���Ҫʹ�����������Ͷ��һ�����յ�׼�� 
 
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
 // �����߳� 
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
       // ��ɶ˿�����Ϣ���� 
        if  (GetQueuedCompletionStatus(CompletionPort,  & BytesTransferred,
         (LPDWORD) & PerHandleData, (LPOVERLAPPED  * )  & PerIoData, INFINITE)  ==   0 )
       {
         printf( " GetQueuedCompletionStatus failed with error %d\n " , GetLastError());
          return   0 ;
      } 
 
 
       // �ǲ��������˳��� 
 
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
