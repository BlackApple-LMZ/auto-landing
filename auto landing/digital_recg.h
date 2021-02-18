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

		//main function
		void run();
	private:
		//计算像素和
		int get_pxsum(const cv::Mat& image);

		//通过模板匹配找到相似的字符
		int getSubstract(const cv::Mat& image);

		//抓取数据
		double recognition(int start_left, int start_top);

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat raw_image_;

		//抓取到的数据
		double heading_;
	};
}



#endif //DIGITAL_RECG_H

