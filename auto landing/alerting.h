#pragma once
//
// Created by limz on 2021/01/27.
// ����ʵ�ʺ������ܵ����߼���澯��Ϣ
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

		//auto_landing ����ʼ����澯��Ϣ
		/*
		image: ��ǰ��frame
		center: python�������� todo ��ʾ��ʽ��û�����
		heading: �ӷɻ��ɼ��ĺ����
		*/
		void requestStart(const cv::Mat& image, double heading);
		bool isStopped();
		void setFinish();

		//���ظ澯�ļ��� һ��mask 
		int getAlert();

		//main function
		void run();
	private:
		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat raw_image_;

		//�澯��Ϣ��ֵ �Ƕȴ��������ֵ�͸澯
		int th_heading_;

		int area_;
	};
}



#endif //ALERTING_H