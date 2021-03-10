#pragma once

#ifndef DIGITAL_RECG_H
#define DIGITAL_RECG_H

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include <mutex>

namespace autolanding_lmz {

	class digitalRecg {
	public:
		digitalRecg();
		~digitalRecg();

		void requestStart(const cv::Mat& image);
		bool isStopped();
		void setFinish();

		//返回航向角
		double getHeading();
		double getPitch();
		double getRoll();
		double getX();
		double getY();
		double getZ();

		//main function
		void run();
	private:
		//计算像素和
		int get_pxsum(const cv::Mat& image);

		//通过模板匹配找到相似的字符
		//image 输入的待检测的字符 
		//flag 选择模板0是蓝色的1是绿色的那一套
		//index 当前检测的数字是第几位
 		int getSubstract(const cv::Mat& image, int flag, int index);

		//抓取数据
		//data_num 是指抓取的位数 xplane显示器是6位 插件dataref是8位
		//flag 是指选取不同的template 0表示xplane显示器 1表示插件dataref
		double recognition(int start_left, int start_top, int data_num, int flag);

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat raw_image_;

		//抓取到的数据
		double heading_, pitch_, roll_, local_x_, local_y_, local_z_;
		int index = 0;
	};
}



#endif //DIGITAL_RECG_H

