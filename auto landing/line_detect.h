//
// Created by limz on 2021/2/05.
// 检测跑道边界线
//

#ifndef LINE_DETECT_H
#define LINE_DETECT_H

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include <mutex>
#include "KMeans.h"

namespace autolanding_lmz {

    class lineDetect {
    public:
		lineDetect(std::string file_name);
        ~lineDetect();

		//auto_landing 请求开始检测直线//
		void requestStart(const cv::Mat& image);
		bool isStopped();
		void setFinish();
		cv::Mat getDetectLine();
		//获取检测的轮廓用来显示或者birdview用//
		std::vector<cv::Point> getContour();

		void run();
    private:
		//这个是用hsv提取出跑道区域轮廓，然后根据轮廓搞边界线//
		bool detectLine();
		//这个是用hsv单独提取跑道边界线的方法//
		bool detectLine1();

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat lineDetect_image_;
		cv::Mat raw_image_;		

		bool loadImage();
		
		void gray2bgr(cv::Mat gray, cv::Mat& bgr);
		bool checkLine(cv::Vec4i line);
		void maskFusion(cv::Vec4i left_line, cv::Vec4i right_line, const cv::Mat &image_mask_contour, cv::Mat &image_mask_result);
		bool findCrosspoint(cv::Vec4i left_line, cv::Vec4i right_line, int rows, int cols, cv::Point &crosspoint);
		void regression(std::vector<cv::Vec4i> lines, int rows, int cols, cv::Vec4i &fit_line);
		void lineSeparation(std::vector<cv::Vec4i> lines, std::vector<cv::Vec4i> &left_lines, std::vector<cv::Vec4i> &right_lines, double &left_angle, double &right_angle, const cv::Mat& image, double slope_thresh, bool bfirst);
		std::string file_name_;
		int curr_index_;
		int start_index_ = 520;

		std::vector<cv::Point> contour_;
		//cv::Mat raw_image_;
		//cv::Mat image_road_result_; //final road image

		//std::shared_ptr<KMeans> pKMeans_;
    };
}



#endif //LINE_DETECT_H
