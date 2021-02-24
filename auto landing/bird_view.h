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
#include <Eigen/Core>
#include <Eigen/Geometry>

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
		void setContour(const std::vector<cv::Point>& contour);
		//设置飞行状态参数，计算相对初始时刻的姿态变换矩阵//
		void setPosition(double heading, double pitch, double roll, double x, double y, double z);

		void test(const cv::Mat& image, cv::Mat& perspective);
		//main function
		void run();
    private:
		void computeBirdview();

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat birdView_image_;
		cv::Mat raw_image_;
		std::vector<cv::Point> contour_;

		Eigen::Matrix4d T_origin_, T_origin_inv_;
		Eigen::Matrix4d T_cur_, Toc_;

		int index_ = 0;
    };
}



#endif //BIRD_VIEW_H
