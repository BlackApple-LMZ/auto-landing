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
		double getPitch();
		double getRoll();
		double getX();
		double getY();
		double getZ();

		//main function
		void run();
	private:
		//�������غ�
		int get_pxsum(const cv::Mat& image);

		//ͨ��ģ��ƥ���ҵ����Ƶ��ַ�
		//image ����Ĵ������ַ� 
		//flag ѡ��ģ��0����ɫ��1����ɫ����һ��
		//index ��ǰ���������ǵڼ�λ
 		int getSubstract(const cv::Mat& image, int flag, int index);

		//ץȡ����
		//data_num ��ָץȡ��λ�� xplane��ʾ����6λ ���dataref��8λ
		//flag ��ָѡȡ��ͬ��template 0��ʾxplane��ʾ�� 1��ʾ���dataref
		double recognition(int start_left, int start_top, int data_num, int flag);

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat raw_image_;

		//ץȡ��������
		double heading_, pitch_, roll_, local_x_, local_y_, local_z_;
		int index = 0;
	};
}



#endif //DIGITAL_RECG_H

