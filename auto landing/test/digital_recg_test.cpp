//
// Created by limz on 2021/3/17.
//

#include "digital_recg.h"
#include <thread>
#include <iostream>

#include <sstream>
#include <string.h>
#include <functional>


#define WIN_32

#ifdef WIN_32
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN_32

int test_digital_recg() {
	cv::Mat frame = cv::imread("E:\\Games\\X-Plane 11 Global Scenery\\Output\\aaaaaaaa.png");
	//cv::Mat frame = cv::imread("E:\\Games\\X-Plane 11 Global Scenery\\Output\\aaaaaaaa.png", CV_LOAD_IMAGE_GRAYSCALE);
	//std::thread tbirdView(&birdView::run, pbirdView_);
	//tbirdView.detach();

	std::thread tdigitalRecg(&autolanding_lmz::digitalRecg::run, pdigitalRecg_);
	tdigitalRecg.detach();

	pdigitalRecg_->requestStart(frame);
	while (!pdigitalRecg_->isStopped()) {
		;
	}
	std::cout << pdigitalRecg_->getX() << " " << pdigitalRecg_->getY() << " " << pdigitalRecg_->getZ() << std::endl;
	return 0;
}








