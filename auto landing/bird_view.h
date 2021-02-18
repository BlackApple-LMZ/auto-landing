//
// Created by limz on 2021/01/27.
// 投影变换得到鸟瞰图
//

#ifndef BIRD_VIEW_H
#define BIRD_VIEW_H

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include <mutex>

namespace autolanding_lmz {

    class birdView {
    public:
		birdView();
        ~birdView();

		//auto_landing 请求开始计算鸟瞰图
		void requestStart(const cv::Mat& image);
		bool isStopped();
		void setFinish();
		cv::Mat getBirdView();

		void test(const cv::Mat& image, cv::Mat& perspective);
		//main function
		void run();
    private:
		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat birdView_image_;
		cv::Mat raw_image_;
    };
}



#endif //BIRD_VIEW_H
