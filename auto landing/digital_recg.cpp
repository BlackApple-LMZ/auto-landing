#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include "digital_recg.h"


namespace autolanding_lmz {

	const int digital_height = 9;
	const int digital_width = 6;
	const int heading_start_left = 150;
	const int roll_start_left = 54;
	const int pitch_start_left = 6;
	const int heading_start_top = 79;

	const int local_start_left = 232;
	const int local_x_start_top = 160;
	const int local_y_start_top = 175;
	const int local_z_start_top = 190;

	digitalRecg::digitalRecg()
	{

	}
	digitalRecg::~digitalRecg()
	{

	}
	void digitalRecg::requestStart(const cv::Mat& image)
	{
		//转成灰度图
		raw_image_ = cv::imread("E:\\Games\\X-Plane 11 Global Scenery\\Output\\111\\Cessna_172SP_" + std::to_string(++index) + ".png", CV_LOAD_IMAGE_GRAYSCALE);
		//cv::cvtColor(raw_image_, raw_image_, CV_RGB2GRAY);
		//cv::cvtColor(image, raw_image_, CV_BGR2GRAY);
		std::unique_lock<std::mutex> lock(mutexRequestStart_);
		startRequested_ = true;
		std::unique_lock<std::mutex> lock2(mutexStop_);
		stopped_ = false;
	}
	bool digitalRecg::isStopped()
	{
		std::unique_lock<std::mutex> lock(mutexStop_);
		return stopped_;
	}
	void digitalRecg::setFinish()
	{
		std::unique_lock<std::mutex> lock(mutexRequestStart_);
		startRequested_ = false;
		std::unique_lock<std::mutex> lock2(mutexStop_);
		stopped_ = true;
	}
	double digitalRecg::getHeading()
	{
		return heading_;
	}
	double digitalRecg::getPitch()
	{
		return pitch_;
	}
	double digitalRecg::getRoll()
	{
		return roll_;
	}
	double digitalRecg::getX()
	{
		return local_x_;
	}
	double digitalRecg::getY()
	{
		return local_y_;
	}
	double digitalRecg::getZ()
	{
		return local_z_;
	}
	void digitalRecg::run() {

		startRequested_ = false;
		int index{ 0 };
		while (1) {
			//wait until request
			while (!startRequested_) {
				;
			}

			//抓取航向角
			heading_ = recognition(heading_start_left, heading_start_top, 6, 0);
			pitch_ = recognition(pitch_start_left, heading_start_top, 6, 0);
			roll_ = recognition(roll_start_left, heading_start_top, 6, 0); //
			local_x_ = recognition(local_start_left, local_x_start_top, 8, 1); //如果数据位数太多会导致溢出
			local_y_ = recognition(local_start_left, local_y_start_top, 8, 1);
			local_z_ = recognition(local_start_left, local_z_start_top, 8, 1);

			++index;
			setFinish();
		}
	}
	double digitalRecg::recognition(int start_left, int start_top, int data_num, int flag) {
		//记录小数点是在数据的第几位
		int index = -1;
		int res = 0;
		bool negative = false;
		for (int j = 0; j < data_num; j++) {
			cv::Rect rect(start_left + j * digital_width, start_top, digital_width, digital_height);
			cv::Mat test = raw_image_(rect).clone();
			int result = getSubstract(test, flag);
			if (result == 10) {
				std::cout << ".";
				index = j;
			}
			else if (result == 11) {
				std::cout << "-";
				negative = true;
			}
			else if (result == 12) {
				std::cout << " ";
				continue;
			}
			else {
				std::cout << result;
				res = 10 * res + result;
			}
		}
		std::cout << std::endl;
		std::cout << res << std::endl;
		double result = 0.0;
		if (index > 0) {
			result = res * 1.0 / (pow(10, (data_num - index - 1)));
		}
		if (negative) {
			result = -result;
		}
		return result;
	}
	int digitalRecg::get_pxsum(const cv::Mat& image) {
		int a = 0;
		for (int i = 0; i < image.rows; i++) {
			for (int j = 0; j < image.cols; j++) {
				a += image.at<uchar>(i, j);
			}
		}
		return a;
	}

	int digitalRecg::getSubstract(const cv::Mat& image, int flag) {
		cv::Mat result_img;
		int min = INT_MAX;
		int result_num = 0;
		for (int i = 0; i < 13; i++) {

			cv::Mat templ;
			if (flag == 0) {
				templ = cv::imread("E:\\project\\auto landing\\auto landing\\auto landing\\template\\" + std::to_string(i) + ".png", CV_LOAD_IMAGE_GRAYSCALE);
			}
			else if(flag == 1)
				templ = cv::imread("E:\\project\\auto landing\\auto landing\\auto landing\\template\\local\\" + std::to_string(i) + ".png", CV_LOAD_IMAGE_GRAYSCALE);
			if (templ.empty()) {
				//std::cout << "no image." << std::endl;
				continue;
			}
			absdiff(templ, image, result_img);

			if (get_pxsum(result_img) < min) {
				min = get_pxsum(result_img);
				result_num = i;
			}
		}
		return result_num;
	}
}