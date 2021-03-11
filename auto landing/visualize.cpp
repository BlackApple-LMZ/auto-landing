//
// Created by limz on 2021/01/25.
//

#include "visualize.h"

namespace autolanding_lmz {
	const int imageWidth = 1280 + 600;
	const int imageHeight = 720;

	visualize::visualize()
	{

		bnext_ = false;
		bicp_ = false;
		breturn_ = false;

		badd_ = false;

		init();
	}
	visualize::~visualize() {

	}
	void visualize::init() {
		//设置窗体的样子啥
		//cv::Mat raw_image = cv::imread(pre_name_ + std::to_string(0) + ".png");
		//cv::imshow("raw image", raw_image);
		heading_image_ = cv::Mat::zeros(360, 600, CV_8UC3);

		
	}
	void visualize::setBirdImage(const cv::Mat& image) {
		bird_image_ = image;
	}
	void visualize::setRawImage(const cv::Mat& image) {
		raw_image_ = image.clone();
	}
	void visualize::setLineImage(const cv::Mat& image) {
		line_image_ = image;
	}
	void visualize::setHeading(double predictHeading, double realHeading, int index) {
		predictHeading_ = predictHeading;
		realHeading_ = realHeading;
		index_ = index;
		std::cout << realHeading_ << std::endl;
	}
	bool visualize::show(int index){
		cv::Mat raw_image = cv::imread(pre_name_ + std::to_string(index) + ".png");

		if (raw_image.empty()) {
			std::cout << "Can not read this image !" << index << std::endl;
			return false;
		}

		cv::imshow("raw image", raw_image);
		cv::waitKey(1);

		return true;
	}
	void visualize::gray2bgr(cv::Mat gray, cv::Mat& bgr) {
		int channels = bgr.channels();
		for (int i = 0; i < gray.rows; i++) {
			uchar *pSrcData = gray.ptr<uchar>(i);
			uchar *pResuiltData = bgr.ptr<uchar>(i);
			for (int j = 0, index = 2; j < gray.cols; j++, index += channels) {
				pResuiltData[index] = pSrcData[j];
			}
		}
	}
	void visualize::cutGray(cv::Mat& gray, int th) {
		for (int i = 0; i < th; i++) {
			uchar *pSrcData = gray.ptr<uchar>(i);
			for (int j = 0; j < gray.cols; j++) {
				pSrcData[j] = 0;
			}
		}
	}
	void visualize::drawLineImage(cv::Mat& gray, int th) {
		int channels = raw_image_.channels();
		for (int i = th; i < raw_image_.rows; i++) {
			uchar *pSrcData = gray.ptr<uchar>(i);
			uchar *pResuiltData = raw_image_.ptr<uchar>(i);
			for (int j = 0, index = 1; j < gray.cols; j++, index += channels) {
				if (pSrcData[j] == 255) {
					pResuiltData[index-1] = 0;
					pResuiltData[index] = 0;
					pResuiltData[index+1] = 255;
				}

			}
		}
	}
	void visualize::drawHeadingImage() {
		heading_image_ = cv::Mat::zeros(360, 600, CV_8UC3);

		if (traj_real_.size() > 400) {
			traj_real_.pop_front();
			traj_predict_.pop_front();
		}
		traj_real_.push_back(int((75 - realHeading_) * 360 / 40));
		traj_predict_.push_back(int((75 - predictHeading_) * 360 / 40));

		// plot the information
		std::string text11 = "Red color: real heading";
		std::string text22 = "Green color: predict heading";

		putText(heading_image_, text11, cv::Point2f(10, 50), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1, 5);
		putText(heading_image_, text22, cv::Point2f(10, 35), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0), 1, 5);

		// plot the information
		std::string text1 = "real_heading: " + std::to_string(realHeading_);
		std::string text2 = "predict heading: " + std::to_string(predictHeading_);

		double error = abs(realHeading_ - predictHeading_);
		std::string text3 = "error: " + std::to_string(error);

		if (error < 10) {
			sum_error_ += error;
			++count_;
		}
		double mean_error = count_ > 0 ? sum_error_ / count_ : 0.0;
		std::string text4 = "mean error: " + std::to_string(mean_error);

		if (error < 10) {
			sum_error_square_ += (error - mean_error) * (error - mean_error);
		}
		double std_dev = count_ > 0 ? sqrt(sum_error_square_ / count_) : 0.0;
		std::string text5 = "std_dev: " + std::to_string(std_dev);

		putText(heading_image_, text1, cv::Point2f(330, 35), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1, 5);
		putText(heading_image_, text2, cv::Point2f(330, 50), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1, 5);
		putText(heading_image_, text3, cv::Point2f(330, 65), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1, 5);
		putText(heading_image_, text4, cv::Point2f(330, 80), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1, 5);
		putText(heading_image_, text5, cv::Point2f(330, 95), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1, 5);

		for (int i = 0; i < traj_real_.size(); i++) {
			cv::Point2f point_real = cv::Point2f(i, traj_real_[i]);
			cv::Point2f point_predict = cv::Point2f(i, traj_predict_[i]);

			cv::circle(heading_image_, point_real, 1, cv::Scalar(0, 0, 255), 2);
			cv::circle(heading_image_, point_predict, 1, cv::Scalar(0, 255, 0), 2);
		}	
	}
	bool visualize::show() {

		cv::Mat showWindowImages(imageHeight, imageWidth, CV_8UC3, cv::Scalar::all(0));

		//show line image
		drawLineImage(line_image_, 390);

		//show raw image
		cv::Mat tempRawImage = showWindowImages(cv::Rect(0, 0, 1280, 720));
		resize(raw_image_, tempRawImage, cv::Size(1280, 720));

		//show birdview image
		cv::Mat tempBirdImage = showWindowImages(cv::Rect(1280, 0, 600, 360));
		resize(bird_image_, tempBirdImage, cv::Size(600, 360));

		//show heading image
		drawHeadingImage();
		cv::Mat tempHeadingImage = showWindowImages(cv::Rect(1280, 360, 600, 360));
		resize(heading_image_, tempHeadingImage, cv::Size(600, 360));

		cv::imshow("show", showWindowImages);
		//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\show\\image" + std::to_string(index_) + ".png", showWindowImages);
		cv::waitKey(1);

		return true;
	}
}