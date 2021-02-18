#pragma once
//
// Created by limz on 2021/01/27.
// 根据实际航向与跑道中线计算告警信息
//

#ifndef ALERTING_H
#define ALERTING_H

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include <mutex>

namespace autolanding_lmz {

	class Alerting {
	public:
		Alerting(int th_area);
		~Alerting();

		//auto_landing 请求开始计算告警信息
		/*
		image: 当前的frame
		center: python检测的中线 todo 表示形式还没有想好
		heading: 从飞机采集的航向角
		*/
		void requestStart(const cv::Mat& image, double heading);
		bool isStopped();
		void setFinish();

		//返回告警的级别 一个mask 
		int getAlert();

		//main function
		void run();
	private:
		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat raw_image_;

		//告警信息阈值 角度大于这个数值就告警
		int th_heading_;

		int area_;
	};
}



#endif //ALERTING_H