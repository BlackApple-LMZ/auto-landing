#include <stdio.h>  
#include <winsock2.h>  
#include <iostream>
#include "cp_socket.h"
#include <process.h>  

#pragma comment(lib,"ws2_32.lib")

namespace autolanding_lmz {


	CPSocket::CPSocket()
	{

	}
	CPSocket::~CPSocket()
	{

	}
	void CPSocket::requestStart()
	{
		std::unique_lock<std::mutex> lock(mutexRequestStart_);
		//发送数据  
		const char * sendData = "startProcess\n";
		send(sClient, sendData, strlen(sendData), 0);
		std::cout << "startProcess" << std::endl;

		std::unique_lock<std::mutex> lock2(mutexStop_);
		stopped_ = false;
	}
	bool CPSocket::isStopped()
	{
		std::unique_lock<std::mutex> lock(mutexStop_);
		return stopped_;
	}
	bool CPSocket::isConnected()
	{
		return connected_;
	}

	//用线程处理一个客户端的收发信息
	DWORD CPSocket::ThreadFunc() {
		sockaddr_in addr = { 0 };
		int addrLen = sizeof(addr);
		getpeername(sClient, (sockaddr *)&addr, &addrLen);//获取套接字的信息

		char buf[1024];
		int ret;

		connected_ = true;
		while (true) {

			//接收数据  
			int numbytes = recv(sClient, buf, sizeof(buf), 0);
			if (numbytes == 0 || (numbytes < 0 && errno != EAGAIN))
			{
				std::cout << "Receive Error!" << errno << std::endl;
				break;
			}
			else {
				if (buf[numbytes - 1] == '\n')
				{
					buf[numbytes - 1] = '\0';
					numbytes--;
				}

				if (strncmp(buf, "iFinish", 7) == 0)
				{
					std::unique_lock<std::mutex> lock(mutexStop_);
					stopped_ = true;
				}
				else {
					std::cout << buf << std::endl;
				}

			}
		}
		connected_ = false;

		return 0;
	}
	void CPSocket::run() {

		//初始化WSA  
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(sockVersion, &wsaData) != 0)
		{
			return ;
		}

		//创建套接字  
		SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (slisten == INVALID_SOCKET)
		{
			printf("socket error !");
			return ;
		}

		//绑定IP和端口  
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(5024);
		sin.sin_addr.S_un.S_addr = INADDR_ANY;
		if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		{
			printf("bind error !");
		}

		//开始监听  
		if (listen(slisten, 5) == SOCKET_ERROR)
		{
			printf("listen error !");
			return ;
		}

		//循环接收数据  
		DWORD threadId;
		//SOCKET sClient;
		sockaddr_in remoteAddr;
		int nAddrlen = sizeof(remoteAddr);
		char revData[255];
		while (true)
		{
			printf("wait for connect...\n");
			sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
			if (sClient == INVALID_SOCKET)
			{
				//printf("accept error !");
				continue;
			}

			printf("receive a new connecter: %s \r\n", inet_ntoa(remoteAddr.sin_addr));

			CreateThread(NULL, 0, StaticThreadFunc, (void*)this, 0, &threadId);

			//unsigned long ul = 1;
			//int ret = ioctlsocket(slisten, FIONBIO, (unsigned long *)&ul);    //设置成非阻塞模式

		}

		closesocket(slisten);
		WSACleanup();
		return ;
	}

}



