//
// Created by limz on 2021/01/29.
// c++��pyͨ�ŵĳ���
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

		//auto_landing ����ʼpy����ͼ��
		void requestStart();
		bool isStopped();
		bool isConnected();

		//main function
		void run();
		
	private:
		SOCKET sClient;

		//socket �ͻ����߳�
		static DWORD WINAPI StaticThreadFunc(void* Param) {
			CPSocket* This = (CPSocket*)Param;
			return This->ThreadFunc();
		}
		DWORD WINAPI ThreadFunc();

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		//py��c++ͨѶ���ӵı�־λ true��ʾ������ false��ʾ���ӶϿ�
		bool connected_{ false };
	};
}



#endif //CP_SOCKET_H