//
// Created by limz on 2021/01/27.
//

#include "alerting.h"
#include "tic_toc.h"

#include <cmath>
#include <opencv2/imgproc/imgproc.hpp>

#define WIN_32

#ifdef WIN_32
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN_32

namespace autolanding_lmz {


	Alerting::Alerting(int th_heading) : th_heading_(th_heading)
	{

	}
	Alerting::~Alerting()
	{

	}
	void Alerting::requestStart(const cv::Mat& image, double heading)
	{
		raw_image_ = image;
		std::unique_lock<std::mutex> lock(mutexRequestStart_);
		startRequested_ = true;
		std::unique_lock<std::mutex> lock2(mutexStop_);
		stopped_ = false;
	}
	bool Alerting::isStopped()
	{
		std::unique_lock<std::mutex> lock(mutexStop_);
		return stopped_;
	}
	void Alerting::setFinish()
	{
		std::unique_lock<std::mutex> lock(mutexRequestStart_);
		startRequested_ = false;
		std::unique_lock<std::mutex> lock2(mutexStop_);
		stopped_ = true;
	}
	int Alerting::getAlert()
	{
		return area_;
	}
	void Alerting::run() {

		startRequested_ = false;
		int index{ 0 };
		while (1) {
			//wait until request
			while (!startRequested_) {
				;
			}

			//todo 根据输入的航向角和中线 判断是否告警

			++index;
			setFinish();

		}
	}

}