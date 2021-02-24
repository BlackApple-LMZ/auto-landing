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
void birdView::setContour(const std::vector<cv::Point>& contour) {
	contour_ = contour;
}
void birdView::setPosition(double heading, double pitch, double roll, double x, double y, double z) {
	double scale = 3.1415926 / 180;
	//初始化欧拉角(Z-Y-X)
	Eigen::Vector3d ea(pitch*scale, heading*scale, roll*scale);

	Eigen::Matrix3d rotation_matrix;
	rotation_matrix = Eigen::AngleAxisd(ea[0], Eigen::Vector3d::UnitZ()) *
		Eigen::AngleAxisd(ea[1], Eigen::Vector3d::UnitY()) *
		Eigen::AngleAxisd(ea[2], Eigen::Vector3d::UnitX());

	Eigen::Vector3d tr;
	tr << x, y, z;

	T_cur_.setIdentity();
	T_cur_.block<3, 3>(0, 0) = rotation_matrix;
	T_cur_.topRightCorner<3, 1>() = tr;

	if (++index_ == 3) {
		//将第三帧的位姿设置为初始位姿 录视频的时候前几帧会是其他状态//
		T_origin_.setIdentity();
		T_origin_.block<3, 3>(0, 0) = rotation_matrix;
		T_origin_.topRightCorner<3, 1>() = tr;

		T_origin_inv_ = T_origin_.inverse();
	}
	else if (index_ > 3) {
		Toc_ = T_origin_inv_ * T_cur_;
	}

	std::cout << T_cur_ << std::endl;
}

void birdView::run(){

	startRequested_ = false;
	int index{ 0 };
	while (1) {
		//wait until request
		while (!startRequested_) {
			;
		}
		computeBirdview();
		//todo 用透视变换得到鸟瞰图 目前先随便加载一张图
		//test(raw_image_, birdView_image_);
		birdView_image_ = cv::imread("E:\\Games\\X-Plane 11 Global Scenery\\Output\\" + std::to_string(index++) + ".jpg");
		if (index > 6)
			index = 0;
		setFinish();

	}
}
const int TH_HEIGHT = 390;
const int TH_WIDTH = 20;
void birdView::computeBirdview() {

	//先根据contour选择4个顶点:根据坐标值来选取//
	int min_x = INT_MAX, max_x = INT_MIN;
	cv::Point2f left(1280, 720), right(1280, 720);


	for (int i = 0; i < contour_.size(); i++) {
		cv::Point pt = contour_[i];
		if (pt.y == TH_HEIGHT) {
			if(pt.x < min_x)
				min_x = pt.x;
			if (pt.x > max_x)
				max_x = pt.x;
		}
		if (pt.x < TH_WIDTH) {
			if (pt.y < left.y)
				left = pt;
		}
		else if (pt.x > 1280 - TH_WIDTH) {
			if (pt.y < right.y)
				right = pt;
		}
	}

	//然后选择匹配的点对构建鸟瞰图//
	cv::Point2f src_points[] = {
		left,
		right,
		cv::Point2f(min_x, TH_HEIGHT),
		cv::Point2f(max_x, TH_HEIGHT) };

	//std::cout << "in bird view: " << left.x << " " << left.y << " " << right.x << " " << right.y << " " << min_x << " " << max_x << std::endl;

	cv::Point2f dst_points[] = {
		cv::Point2f(300, 719),
		cv::Point2f(980, 719),
		cv::Point2f(300, 0),
		cv::Point2f(980, 0) };

	cv::Mat M = cv::getPerspectiveTransform(src_points, dst_points);

	cv::Mat perspective;
	cv::warpPerspective(raw_image_, perspective, M, cv::Size(raw_image_.cols, raw_image_.rows), cv::INTER_LINEAR);

	for (int i = 0; i < 4; i++) {
		cv::circle(raw_image_, src_points[i], 2, cv::Scalar(0, 255, 255), 2, 8);
	}
	cv::imshow("eeeeeeeeeeeeeeeeeeeee", raw_image_);
	cv::imshow("ssssssssssssssssss", perspective);
	//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\point.png", raw_image_);
	//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\perspective.png", perspective);
	cv::waitKey(1);
}




}