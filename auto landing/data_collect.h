//
// Created by limz on 2020/11/2.
// 从xplane采集数据相关操作
//

#ifndef DATA_COLLECT_H
#define DATA_COLLECT_H

#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <conio.h>
#include<bitset>

namespace autolanding_lmz {

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) 

    class dataCollect {
    public:
		dataCollect() {};
		~dataCollect() {};

		void keyPress() {
			keybd_event(17, 0, 0, 0);
			keybd_event(72, 0, 0, 0);
			keybd_event(72, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(17, 0, KEYEVENTF_KEYUP, 0);
			//std::cout << "press ctrl+h" << std::endl;
		}

		char waitKey() {
			if (KEY_DOWN('S')) {//需要大写
				return 'S';
			}
			char ch = _getch();
			//std::cout << (int)ch << std::endl;
			return ch;
		}
    private:
		
    };
}


#endif //DATA_COLLECT_H
