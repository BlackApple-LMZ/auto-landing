//
// Created by limz on 2021/01/27.
//

#include "bird_view.h"
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


birdView::birdView()
{

}
birdView::~birdView()
{

}
void birdView::requestStart(const cv::Mat& image)
{
	raw_image_ = image;
	std::unique_lock<std::mutex> lock(mutexRequestStart_);
	startRequested_ = true;
	std::unique_lock<std::mutex> lock2(mutexStop_);
	stopped_ = false;
}
bool birdView::isStopped()
{
	std::unique_lock<std::mutex> lock(mutexStop_);
	return stopped_;
}
void birdView::setFinish()
{
	std::unique_lock<std::mutex> lock(mutexRequestStart_);
	startRequested_ = false;
	std::unique_lock<std::mutex> lock2(mutexStop_);
	stopped_ = true;
}
void birdView::test(const cv::Mat& image, cv::Mat& perspective)
{
	cv::Point2f src_points[] = {
		cv::Point2f(0, 572),
		cv::Point2f(1280, 557),
		cv::Point2f(572, 465),
		cv::Point2f(748, 465) };

	cv::Point2f dst_points[] = {
		cv::Point2f(300, 600),
		cv::Point2f(980, 600),
		cv::Point2f(300, 0),
		cv::Point2f(980, 0) };

	cv::Mat M = cv::getPerspectiveTransform(src_points, dst_points);

	cv::warpPerspective(image, perspective, M, cv::Size(image.cols, image.rows), cv::INTER_LINEAR);

	return ;
}
cv::Mat birdView::getBirdView()
{
	return birdView_image_;
}
void birdView::run(){

	startRequested_ = false;
	int index{ 0 };
	while (1) {
		//wait until request
		while (!startRequested_) {
			;
		}

		//todo 用透视变换得到鸟瞰图 目前先随便加载一张图
		//test(raw_image_, birdView_image_);
		birdView_image_ = cv::imread("E:\\Games\\X-Plane 11 Global Scenery\\Output\\" + std::to_string(index++) + ".jpg");
		if (index > 6)
			index = 0;
		setFinish();

	}
}






}