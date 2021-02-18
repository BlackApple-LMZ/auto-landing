//
// Created by limz on 2021/2/05.
//

#include "line_detect.h"
#include "tic_toc.h"

#include <cmath>
#include <opencv2/imgproc/imgproc.hpp>


namespace autolanding_lmz {

//注意这个地方是cv::String !!!!
const cv::String window_capture_name = "Video Capture";
const cv::String window_detection_name = "Object Detection";

lineDetect::lineDetect(std::string file_name): file_name_(file_name)
{
	//raw_image_ = cv::imread(file_name_ + std::to_string(curr_index_) + ".png");
	//image_road_result_ = cv::Mat(raw_image_.size(), CV_8UC1, cv::Scalar(0)); //最终的交集mask

	curr_index_ = start_index_;

}
lineDetect::~lineDetect()
{

}
void lineDetect::requestStart(const cv::Mat& image)
{
	raw_image_ = image;
	std::unique_lock<std::mutex> lock(mutexRequestStart_);
	startRequested_ = true;
	std::unique_lock<std::mutex> lock2(mutexStop_);
	stopped_ = false;
}
bool lineDetect::isStopped()
{
	std::unique_lock<std::mutex> lock(mutexStop_);
	return stopped_;
}
void lineDetect::setFinish()
{
	std::unique_lock<std::mutex> lock(mutexRequestStart_);
	startRequested_ = false;
	std::unique_lock<std::mutex> lock2(mutexStop_);
	stopped_ = true;
}

void lineDetect::run() {

	startRequested_ = false;
	int index{ 0 };
	while (1) {
		//wait until request
		while (!startRequested_) {
			;
		}

		detectLine();

		setFinish();
	}
}


cv::Mat lineDetect::getDetectLine()
{
	return lineDetect_image_;
}
bool lineDetect::detectLine1() {

	cv::Mat image_gauss;
	cv::GaussianBlur(raw_image_, image_gauss, cv::Size(3, 3), 0, 0);

	cv::Mat image_hsv;
	cvtColor(image_gauss, image_hsv, cv::COLOR_BGR2HSV);

	//195是直方图均衡化前的参数 也许均衡化后这个数值可以固定
	inRange(image_hsv, cv::Scalar(0, 0, 0), cv::Scalar(180, 255, 195), lineDetect_image_);
	cv::bitwise_not(lineDetect_image_, lineDetect_image_);

	//cv::imshow("腐蚀效果图", image_dilate);

	//cv::imshow("showaaa", lineDetect_image_);
	cv::waitKey(1);
	return true;
}
bool lineDetect::detectLine() {

	cv::Mat image_gauss;
	cv::GaussianBlur(raw_image_, image_gauss, cv::Size(3, 3), 0, 0);

	cv::Mat image_hsv;
	cvtColor(image_gauss, image_hsv, cv::COLOR_BGR2HSV);

	cv::Mat image_inRange;
	inRange(image_hsv, cv::Scalar(23, 0, 0), cv::Scalar(180, 255, 255), image_inRange);
	cv::bitwise_not(image_inRange, image_inRange);

	cv::Mat image_dilate;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	morphologyEx(image_inRange, image_dilate, cv::MORPH_CLOSE, element);

	std::vector<std::vector<cv::Point>> contours;
	findContours(image_dilate, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	int imax = 0; //find contour with max length  
	float imaxLength = 0.0;
	for (int i = 0; i < contours.size(); i++)
	{
		float length = arcLength(contours[i], false);
		if (length > imaxLength)
		{
			imax = i;
			imaxLength = length;
		}
	}

	if (imaxLength < 100) {
		curr_index_++;
		return false;
	}

	drawContours(raw_image_, contours, imax, cv::Scalar(0, 0, 255), 3);
	cv::imshow("aaaaaaaaaa", raw_image_);
	cv::waitKey(1);
	return true;
}

}