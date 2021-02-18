//
// Created by limz on 2021/01/29.
// c++与py通信的程序
//

#ifndef CP_SOCKET_H
#define CP_SOCKET_H

#include <iostream>
#include <vector>
#include <string>

#include <mutex>

namespace autolanding_lmz {

	class CPSocket {
	public:
		CPSocket();
		~CPSocket();

		//auto_landing 请求开始py处理图像
		void requestStart();
		bool isStopped();
		bool isConnected();

		//main function
		void run();
		
	private:
		SOCKET sClient;

		//socket 客户端线程
		static DWORD WINAPI StaticThreadFunc(void* Param) {
			CPSocket* This = (CPSocket*)Param;
			return This->ThreadFunc();
		}
		DWORD WINAPI ThreadFunc();

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		//py与c++通讯连接的标志位 true表示连接上 false表示连接断开
		bool connected_{ false };
	};
}



#endif //CP_SOCKET_H