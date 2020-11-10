//
// Created by limz on 2020/8/27.
// 图像的相关操作
//

#ifndef IMAGE_PROCESS_H
#define IMAGE_PROCESS_H

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include "KMeans.h"

namespace autolanding_lmz {
	//todo 为什么1会出错，但是2就不会出错了？？？？？
	//1 const char *window_capture_name = "Video Capture";
	//2 const char window_capture_name[] = "Video Capture";

    class imageProcess {
    public:
		imageProcess(std::string file_name);
        ~imageProcess();

		void detect();
		bool adjustDirection(double &angle_left, double &angle_right);
		void collectData();
    private:
		bool dataCompute(std::string name, double &angle_left, double &angle_right);

		bool loadImage();
		/*
		 * 通过track bar调整参数来实现选取ROI
		 */
		void selectHSVParam();
		
		void gray2bgr(cv::Mat gray, cv::Mat& bgr);
		bool checkLine(cv::Vec4i line);
		void maskFusion(cv::Vec4i left_line, cv::Vec4i right_line, const cv::Mat &image_mask_contour, cv::Mat &image_mask_result);
		bool findCrosspoint(cv::Vec4i left_line, cv::Vec4i right_line, int rows, int cols, cv::Point &crosspoint);
		void regression(std::vector<cv::Vec4i> lines, int rows, int cols, cv::Vec4i &fit_line);
		void lineSeparation(std::vector<cv::Vec4i> lines, std::vector<cv::Vec4i> &left_lines, std::vector<cv::Vec4i> &right_lines, const cv::Mat& image, double slope_thresh, bool bfirst);
		std::string file_name_;
		int curr_index_;
		int start_index_ = 520;

		cv::Mat raw_image_;
		cv::Mat image_road_result_; //final road image

		std::shared_ptr<KMeans> pKMeans_;
    };
}



#endif //IMAGE_PROCESS_H
