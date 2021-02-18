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

		//���غ����
		double getHeading();

		//main function
		void run();
	private:
		//�������غ�
		int get_pxsum(const cv::Mat& image);

		//ͨ��ģ��ƥ���ҵ����Ƶ��ַ�
		int getSubstract(const cv::Mat& image);

		//ץȡ����
		double recognition(int start_left, int start_top);

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat raw_image_;

		//ץȡ��������
		double heading_;
	};
}



#endif //DIGITAL_RECG_H

