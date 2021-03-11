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
	//���������ʽ���� �ǲ��������ڲξ���ĺ���
	OpenGL_projectionMatrix_ << 0.001563, 0.0, 0.0, -1.0,
		0.0, 0.002778, 0.0, -1.0,
		0.0, 0.0, -1.0, 0.0,
		0.0, 0.0, 0.0, 1.0;
	 
	//world matrix��ʵʱ�仯�� ��ǰ�ӽ������opengl������̬//
	OpenGL_worldMatrix_ << 1.000, 0.0, 0.0, 0.0,
		0.0, 0.002778, 0.0, 0.0,
		0.0, 0.0, -1.0, 0.0,
		-1.0, -1.0, 0.0, 1.0;
		
	OpenGL_modelviewMatrix_ << 1.000, 0.0, 0.0, 0.0,
		0.0, 1.000, 0.0, 0.0,
		0.0, 0.0, 1.000, 0.0,
		0.0, 0.0, 0.0, 1.000;

	Toc_ << 1.000, 0.0, 0.0, 0.0,
		0.0, 1.000, 0.0, 0.0,
		0.0, 0.0, 1.000, 0.0,
		0.0, 0.0, 0.0, 1.000;

	M_per_ << -0.163069544364509, -1.660353349973082, 735.3956834532377, 0.0,
	-5.456968210637569e-16, -1.761855821465277, 690.647482014389, 0.0,
	-8.818629946085279e-19, -0.00262810160035237, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0;

	M_ = toCvMat(M_per_);

	airplane_image_ = cv::imread("E:\\project\\auto landing\\auto landing\\auto landing\\image\\airplane.jpg");
//	M_.at<float>(0, 0) = ;
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
void birdView::test()
{
	cv::Point2f src_points[] = {
		cv::Point2f(50, 490),
		cv::Point2f(1250, 490),
		cv::Point2f(574, 392),
		cv::Point2f(700, 392) };

	cv::Point2f dst_points[] = {
		cv::Point2f(300, 600),
		cv::Point2f(980, 600),
		cv::Point2f(300, 0),
		cv::Point2f(980, 0) };

	cv::Mat M = cv::getPerspectiveTransform(src_points, dst_points);
	//std::cout << M << std::endl;

	/*Eigen::Matrix4d T_pos;
	T_pos << 0.9078, 0.0042, 0.4194, 198.3021,
		-0.0020, 1.0000, -0.0058, -3.9361,
		-0.4195, 0.0044, 0.9078, -5.1069,
		0.0, 0.0, 0.0, 1.0000; 

	Eigen::Matrix4d T_tem = M_per_ * T_pos;
	cv::Mat M = toCvMat(T_pos); */

	cv::warpPerspective(raw_image_, perspective_image_, M, cv::Size(raw_image_.cols, raw_image_.rows), cv::INTER_LINEAR);

	cv::imshow("eeeeeeeeeeeeeeeeeeeee", raw_image_);
	cv::imshow("ssssssssssssssssss", perspective_image_);
	//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\point.png", raw_image_);
	//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\perspective.png", perspective);
	cv::waitKey(0);

	return ;
}

cv::Mat birdView::getBirdView()
{
	return birdView_image_;
}
cv::Mat birdView::getPerspective()
{
	return perspective_image_;
}
void birdView::setContour(const std::vector<cv::Point>& contour) {
	contour_ = contour;
}
cv::Mat birdView::toCvMat(const Eigen::Matrix4d &m)
{
	cv::Mat cvMat(3, 3, CV_32F);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			cvMat.at<float>(i, j) = m(i, j);
	return cvMat.clone();
}
void birdView::setPosition(double heading, double pitch, double roll, double x, double y, double z) {

	gamma_ = (heading - 11.22) * DEG2RAD;
	theta_ = pitch * DEG2RAD;

	//��ʼ��ŷ����(Z-Y-X) 11.22���ֶ��ӵ�����
	Eigen::Vector3d ea(pitch*DEG2RAD, (heading-11.22)*DEG2RAD, roll*DEG2RAD);

	Eigen::Matrix3d rotation_matrix;
	rotation_matrix = Eigen::AngleAxisd(ea[0], Eigen::Vector3d::UnitZ()) *
		Eigen::AngleAxisd(ea[1], Eigen::Vector3d::UnitY()) *
		Eigen::AngleAxisd(ea[2], Eigen::Vector3d::UnitX());

	Eigen::Vector3d tr;
	tr << x, y, z;

	T_cur_.setIdentity();
	T_cur_.block<3, 3>(0, 0) = rotation_matrix;
	T_cur_.topRightCorner<3, 1>() = tr;

	if (++index_ == 2) {
		//������֡��λ������Ϊ��ʼλ�� ¼��Ƶ��ʱ��ǰ��֡��������״̬//
		T_origin_.setIdentity();
		T_origin_.block<3, 3>(0, 0) = rotation_matrix;
		T_origin_.topRightCorner<3, 1>() = tr;

		T_origin_inv_ = T_origin_.inverse();
		gamma0_ = gamma_;
	}
	else if (index_ > 2) {
		Toc_ = T_origin_inv_ * T_cur_;

		//cv::Mat image = cv::imread("E:\\Games\\X-Plane 11 Global Scenery\\Output\\111\\Cessna_172SP_40.png");
		//cv::Mat perspective, M = toCvMat(Toc_);
		std::cout << "relative position in setPosition: "<<Toc_(0, 3)<<" "<< Toc_(1, 3) << " " << Toc_(2, 3) << std::endl;
		//std::cout << M << std::endl;

		//cv::warpPerspective(image, perspective, M, cv::Size(image.cols, image.rows), cv::INTER_LINEAR);
		//cv::imshow("perspective", perspective);
	}

	//std::cout << T_cur_ << std::endl;
}

void birdView::run(){

	startRequested_ = false;
	int index{ 0 };
	while (1) {
		//wait until request
		while (!startRequested_) {
			;
		}
		computeDisplay();

		setFinish();

	}
}
const int TH_HEIGHT = 390;
const int TH_WIDTH = 20;
void birdView::computeBirdview() {

	//�ȸ���contourѡ��4������:��������ֵ��ѡȡ//
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

	//Ȼ��ѡ��ƥ��ĵ�Թ������ͼ//
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
	//std::cout << M << std::endl;
	
	cv::warpPerspective(raw_image_, perspective_image_, M, cv::Size(raw_image_.cols, raw_image_.rows), cv::INTER_LINEAR);

	for (int i = 0; i < 4; i++) {
		cv::circle(raw_image_, src_points[i], 2, cv::Scalar(0, 255, 255), 2, 8);
	}
	//cv::imshow("eeeeeeeeeeeeeeeeeeeee", raw_image_);
	//cv::imshow("ssssssssssssssssss", perspective_image_);
	//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\point.png", raw_image_);
	//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\perspective.png", perspective);
	//cv::waitKey(0);
}
void birdView::computeDisplay() {
	birdView_image_ = cv::Mat(360, 600, CV_8UC3, cv::Scalar(128, 128, 128));

	//�ܵ��߽���//
	cv::line(birdView_image_, cv::Point(120, 0), cv::Point(120, 360), cv::Scalar(255, 255, 255), 5, cv::LINE_AA);
	cv::line(birdView_image_, cv::Point(480, 0), cv::Point(480, 360), cv::Scalar(255, 255, 255), 5, cv::LINE_AA);

	//�ܵ�����//
	cv::line(birdView_image_, cv::Point(300, 0), cv::Point(300, 80), cv::Scalar(255, 255, 255), 15, cv::LINE_AA);
	cv::line(birdView_image_, cv::Point(300, 140), cv::Point(300, 220), cv::Scalar(255, 255, 255), 15, cv::LINE_AA);
	cv::line(birdView_image_, cv::Point(300, 280), cv::Point(300, 360), cv::Scalar(255, 255, 255), 15, cv::LINE_AA);

	int x = 300 + 20 * Toc_(2, 3);
	if (x > 600) {
		std::cout << Toc_(2, 3) << " " << x << std::endl;
	}
	cv::circle(birdView_image_, cv::Point(x, 180), 2, cv::Scalar(0, 0, 255), 2, 8);
	cv::line(birdView_image_, cv::Point(x, 180), cv::Point(300, 180), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);

	std::string text1 = "offset: " + std::to_string(Toc_(2, 3)) + "m";
	putText(birdView_image_, text1, cv::Point(x, 200), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1, 5);

	//����ͷ//
	int end_x = 300 + 90 * sin(gamma_ - gamma0_);
	int end_y = 180 - 90 * cos(gamma_ - gamma0_);
	arrowedLine(birdView_image_, cv::Point(300, 180), cv::Point(end_x, end_y), cv::Scalar(0, 0, 255), 3, 8, 0, 0.1);

	//���طɻ�ͼƬ//
	//cv::Mat imageROI = birdView_image_(cv::Rect(x-20, 160, 40, 40));
	//resize(airplane_image_, imageROI, cv::Size(40, 40));
}
void birdView::fieldView() {
	//cv::Mat gray_image, perspective_image(gray_image.rows, gray_image.cols, CV_8UC1, 0);
	
	const int SRC_RESIZED_WIDTH = 720;
	const int SRC_RESIZED_HEIGHT = 360;
	const int DST_REMAPPED_WIDTH = 400;
	const int DST_REMAPPED_HEIGHT = 400;

	const float alpha_h = 0.5f * FOV_H_ * DEG2RAD;
	const float alpha_v = 0.5f * FOV_V_ * DEG2RAD;

	const float front_map_scale_factor = 0.1; //scale�������Ƴ߶� ������ͶӰ������ݸ��õķŵ�map����
	const float side_map_scale_factor = 0.2;

	cv::Mat imresize;
	cv::Mat imremapped = cv::Mat(DST_REMAPPED_HEIGHT, DST_REMAPPED_WIDTH, CV_8UC1, cv::Scalar(0));
	const int side_map_mid_position = DST_REMAPPED_WIDTH >> 1;
	resize(raw_image_, imresize, cv::Size(SRC_RESIZED_WIDTH, SRC_RESIZED_HEIGHT));

	dy_ = 1.8;
	for (int i = 0; i < imremapped.rows; i++) {
		uchar *pResuiltData = imremapped.ptr<uchar>(i);
		for (int j = 0; j < imremapped.cols; j++) {
			int deltax = front_map_scale_factor * (imremapped.rows - i - dx_);
			int deltay = side_map_scale_factor * (j - side_map_mid_position - dz_);

			if (deltay == 0)
				continue;
			else
			{
				//int u = (int)((atan(dy_ * sin(atan((float)deltay / deltax)) / deltay) - (theta_ - alpha_v)) / (2 * alpha_v / SRC_RESIZED_HEIGHT));
				//int v = (int)((atan((float)deltay / deltax) - (gamma_ - alpha_h)) / (2 * alpha_h / SRC_RESIZED_WIDTH));

				int u = (int)((atan(dy_ * sin(atan((float)deltay / deltax)) / deltay) - (theta_ - alpha_v)) / (2 * alpha_v / SRC_RESIZED_HEIGHT));
				int v = (int)((atan((float)deltay / deltax) - (gamma_ - alpha_h)) / (2 * alpha_h / SRC_RESIZED_WIDTH));
				//std::cout << (2 * alpha_v / SRC_RESIZED_HEIGHT) << " " << v << std::endl;
				if (u >= 0 && u < SRC_RESIZED_HEIGHT && v >= 0 && v < SRC_RESIZED_WIDTH){
					pResuiltData[j] = imresize.at<uchar>(u, v);
					
				}
			}

			//double X = dy_ / tan(theta_ - vertical_field_of_view_deg_ + i * (2 * vertical_field_of_view_deg_ / (m_ - 1))) *
				//cos(lambda_ - vertical_field_of_view_deg_ + j * (2 * vertical_field_of_view_deg_ / (n_ - 1))) + dx_;
			//double Z = dy_ / tan(theta_ - field_of_view_deg_ + i * (2 * field_of_view_deg_ / (m_ - 1))) *
				//sin(lambda_ - field_of_view_deg_ + j * (2 * field_of_view_deg_ / (n_ - 1))) + dz_;
			
		}
	}

	cv::imshow("resize", imresize);
	cv::imshow("remap", imremapped);
	//cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\point.png", imresize);
	//6cv::imwrite("E:\\Games\\X-Plane 11 Global Scenery\\Output\\perspective.png", imremapped);
	cv::waitKey(0);

}


}