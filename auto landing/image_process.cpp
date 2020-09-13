//
// Created by limz on 2020/8/27.
//

#include "image_process.h"
#include "tic_toc.h"

#include <cmath>
#include <opencv2/imgproc/imgproc.hpp>


namespace autolanding_lmz {

const cv::String window_capture_name = "Video Capture";
const cv::String window_detection_name = "Object Detection";
const int max_value_H = 360 / 2; //因为进行8位存储，所以除以2
const int max_value = 255;
const int TH_angle = 4;

int low_H = 0, low_S = 0, low_V = 0;
int high_H = max_value_H, high_S = max_value, high_V = max_value;

static void on_low_H_thresh_trackbar(int, void*)
{
	low_H = std::min(high_H - 1, low_H);//防止最大值小于最小值
	setTrackbarPos("Low H", window_detection_name, low_H);
}

static void on_high_H_thresh_trackbar(int, void*)
{
	high_H = std::max(high_H, low_H + 1);
	setTrackbarPos("High H", window_detection_name, high_H);
}

static void on_low_S_thresh_trackbar(int, void*)
{
	low_S = std::min(high_S - 1, low_S);
	setTrackbarPos("Low S", window_detection_name, low_S);
}

static void on_high_S_thresh_trackbar(int, void*)
{
	high_H = std::max(high_S, low_S + 1);
	setTrackbarPos("High S", window_detection_name, high_S);
}

static void on_low_V_thresh_trackbar(int, void*)
{
	low_V = std::min(high_V - 1, low_V);
	setTrackbarPos("Low V", window_detection_name, low_V);
}

static void on_high_V_thresh_trackvar(int, void*)
{
	high_V = std::max(high_V, low_V + 1);
	setTrackbarPos("High V", window_detection_name, high_V);
}

void setTrackBar()
{
	namedWindow(window_detection_name);

	createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
	createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);

	createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
	createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);

	createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
	createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackvar);
}

imageProcess::imageProcess(std::string file_name): file_name_(file_name)
{
	;
}
imageProcess::~imageProcess()
{

}

void imageProcess::lineSeparation(std::vector<cv::Vec4i> lines, std::vector<cv::Vec4i> &left_lines, std::vector<cv::Vec4i> &right_lines, const cv::Mat& mask, double slope_thresh) {

	std::vector<double> slopes;
	// Calculate the slope of all the detected lines
	for (int i = 0; i<lines.size(); i++) {
		cv::Point ini = cv::Point(lines[i][0], lines[i][1]);
		cv::Point fini = cv::Point(lines[i][2], lines[i][3]);

		//check the line whether in the mask(the last road area) mask is in bgr with 0 255 0 for true
		std::cout << lines[i][0] << " " << lines[i][1] << " " << lines[i][2] << " " << lines[i][3] << std::endl;
		//if (mask.at<uchar>(lines[i][0], lines[i][1]) == 0 || mask.at<uchar>(lines[i][2], lines[i][3]) == 0) {
			//continue;
		//}

		// Basic algebra: slope = (y1 - y0)/(x1 - x0)
		// 这里顺时针为正，与习惯相反，因为图像y轴是向下的 (-180， 180]
		double slope = atan2(fini.y - ini.y, fini.x - ini.x);
		double angle = slope * 180 / CV_PI;
		if (angle < -(180-TH_angle) || abs(angle) < TH_angle || angle > 180-TH_angle 
			|| abs(angle-90) < TH_angle || abs(angle+90) < TH_angle) {
			continue;
		}
		else if (angle > 0) {
			right_lines.push_back(lines[i]);
		}
		else
			left_lines.push_back(lines[i]);

		//std::cout << angle << " ";
	}
	//std::cout << std::endl;
	
	return;
}
void imageProcess::regression(std::vector<cv::Vec4i> lines, int rows, int cols, cv::Vec4i &fit_line) {
	cv::Point ini, fini;
	cv::Vec4d line;
	std::vector<cv::Point> line_pts;

	cv::Point b;
	double m;

	// If lines are being detected, fit a line using all the init and final points of the lines
	for (auto i : lines) {
		ini = cv::Point(i[0], i[1]);
		fini = cv::Point(i[2], i[3]);

		line_pts.push_back(ini);
		line_pts.push_back(fini);
	}

	if (line_pts.size() > 0) {
		// The line is formed here
		cv::fitLine(line_pts, line, CV_DIST_L2, 0, 0.01, 0.01);
		m = line[1] / line[0];
		b = cv::Point(line[2], line[3]);
		//std::cout << m << std::endl;
	
		//std::cout << line_pts.size() << std::endl;
		// One the slope and offset points have been obtained, apply the line equation to obtain the line points
		int ini_y = rows;
		int fin_y = 0;

		//check if the start point or the end point is beyond the edge
		double ini_x = ((ini_y - b.y) / m) + b.x;
		if (ini_x < 0) {
			ini_x = 0;
			ini_y = ((ini_x - b.x) * m) + b.y;
		}
		else if (ini_x >= cols) {
			ini_x = cols;
			ini_y = ((ini_x - b.x) * m) + b.y;
		}

		double fin_x = ((fin_y - b.y) / m) + b.x;
		if (fin_x < 0) {
			fin_x = 0;
			fin_y = ((fin_x - b.x) * m) + b.y;
		}
		else if (fin_x >= cols) {
			fin_x = cols;
			fin_y = ((fin_x - b.x) * m) + b.y;
		}

		fit_line[0] = ini_x;
		fit_line[1] = ini_y;
		fit_line[2] = fin_x;
		fit_line[3] = fin_y;
	}
	return;
}
bool imageProcess::findCrosspoint(cv::Vec4i left_line, cv::Vec4i right_line, int rows, int cols, cv::Point &crosspoint) {
	// computer the crosspoint by two equations of the two lines
	int a1 = left_line[1] - left_line[3];
	int b1 = left_line[2] - left_line[0];
	int c1 = left_line[0] * left_line[3] - left_line[2] * left_line[1];

	int a2 = right_line[1] - right_line[3];
	int b2 = right_line[2] - right_line[0];
	int c2 = right_line[0] * right_line[3] - right_line[2] * right_line[1];

	int d = a1 * b2 - a2 * b1;

	if (!d)
		return false;

	int cross_x = (b1*c2 - b2 * c1) / d;
	int cross_y = (a2*c1 - a1 * c2) / d;

	if (cross_x<0 || cross_x>cols || cross_y<0 || cross_y>rows)
	{
		return false;
	}
	crosspoint.x = cross_x;
	crosspoint.y = cross_y;

	return true;
}
bool imageProcess::checkLine(cv::Vec4i line) {
	// line detect fail if all the point is zero
	return (line[0] || line[1]) || (line[2] || line[3]);
}
void imageProcess::maskFusion(cv::Vec4i left_line, cv::Vec4i right_line, const cv::Mat &image_mask_contour, cv::Mat &image_mask_result) {
	cv::Mat image_poly(image_mask_contour.size(), CV_8UC1, cv::Scalar(0));
	
	//if detect the left and the right line
	if (checkLine(left_line) && checkLine(right_line)) {
		//if there is no crosspoint between left line and right line
		int npt[1] = { 4 };
		cv::Point root_points[1][4];
		root_points[0][0] = cv::Point(left_line[0], left_line[1]);
		root_points[0][1] = cv::Point(left_line[2], left_line[3]);
		root_points[0][2] = cv::Point(right_line[2], right_line[3]);
		root_points[0][3] = cv::Point(right_line[0], right_line[1]);
		const cv::Point* ppt[1] = { root_points[0] };
		fillPoly(image_poly, ppt, npt, 1, cv::Scalar(255));

		cv::bitwise_and(image_mask_contour, image_poly, image_mask_result);

		cv::Mat image_poly2(image_mask_contour.size(), CV_8UC1, cv::Scalar(0));
		//check the cross of the line and the bound  Fill in the bottom
		if (left_line[0] <= 0 || right_line[0] >= image_mask_contour.cols - 1) {
			int npt[1] = { 4 };
			cv::Point root_points[1][4];
			root_points[0][0] = cv::Point(left_line[0], left_line[1]);
			root_points[0][1] = cv::Point(0, image_mask_contour.rows);
			root_points[0][2] = cv::Point(image_mask_contour.cols, image_mask_contour.rows);
			root_points[0][3] = cv::Point(right_line[0], right_line[1]);
			const cv::Point* ppt[1] = { root_points[0] };
			fillPoly(image_poly2, ppt, npt, 1, cv::Scalar(255));
		}
		cv::bitwise_or(image_mask_result, image_poly2, image_mask_result);
	}
	else
		image_mask_result = image_mask_contour.clone();

	return;
}
void imageProcess::gray2bgr(cv::Mat gray, cv::Mat& bgr) {
	int channels = bgr.channels();
	for (int i = 0; i < gray.rows; i++) {
		uchar *pSrcData = gray.ptr<uchar>(i);
		uchar *pResuiltData = bgr.ptr<uchar>(i);
		for (int j = 0, index = 1; j < gray.cols; j++, index += channels) {
			pResuiltData[index] = pSrcData[j];
		}
	}
}
void imageProcess::loadImage() {
	
	TicToc t1;
	raw_image_ = cv::imread(file_name_ + std::to_string(curr_index_) + ".png");
	image_road_result_ = cv::Mat(raw_image_.size(), CV_8UC1, cv::Scalar(0)); //最终的交集mask

//	std::cout << t1.toc() << std::endl;

	TicToc t11;
	cv::Mat image_gauss;
	cv::GaussianBlur(raw_image_, image_gauss, cv::Size(3, 3), 0, 0);
//	std::cout << t11.toc() << std::endl;

	TicToc t111;
	cv::Mat image_hsv;
	cvtColor(image_gauss, image_hsv, cv::COLOR_BGR2HSV); 
//	std::cout << t111.toc() << std::endl;
//	cv::imshow("gauss image", image_gauss);
//	cv::imshow("hsv image", image_hsv);
//	std::cout << t1.toc() << std::endl;

	TicToc t2;
	cv::Mat image_inRange;
	inRange(image_hsv, cv::Scalar(23, 0, 0), cv::Scalar(180, 255, 255), image_inRange);
	cv::bitwise_not(image_inRange, image_inRange);
	cv::imshow("in range image", image_inRange);
//	std::cout << t2.toc() << std::endl;
	
	cv::Mat image_dilate;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	morphologyEx(image_inRange, image_dilate, cv::MORPH_CLOSE, element);
	//erode(srcImage, outImage, image);   //腐蚀：减少高亮部分
	//imshow("腐蚀效果图", outImage);
	
	std::vector<std::vector<cv::Point>> contours;     
	findContours(image_dilate, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	int imax = 0; //find contour with max length
	float imaxLength = 0.0;
	for (int i = 0; i < contours.size(); i++)
	{
		float length = arcLength(contours[i], false);
		if (length > imaxLength)
		{
			imax = i;
			imaxLength = length;
			//std::cout << "length: " << length << std::endl;
		}
	}

	cv::Mat image_mask_contour(raw_image_.size(), CV_8UC1, cv::Scalar(0)), image_mask_line(raw_image_.size(), CV_8UC1, cv::Scalar(0));
	drawContours(image_mask_contour, contours, imax, cv::Scalar(255), CV_FILLED); //for fusion todo 这个是不是可以用inrange那个图来代替
	drawContours(image_mask_line, contours, imax, cv::Scalar(255), 1);
//	drawContours(raw_image_, contours, imax, cv::Scalar(0, 0, 255), 3);
	cv::imshow("line image", image_mask_line);

	std::vector<cv::Vec4i> lines, left_lines, right_lines;
	cv::HoughLinesP(image_mask_line, lines, 1, CV_PI / 180, 80, 100, 10);

	lineSeparation(lines, left_lines, right_lines, image_road_result_, 0.1);

	//from bottom to top
	cv::Vec4i left_line, right_line;
	regression(left_lines, raw_image_.rows, raw_image_.cols, left_line);
	regression(right_lines, raw_image_.rows, raw_image_.cols, right_line);

	//cv::line(raw_image_, cv::Point(left_line[0], left_line[1]), cv::Point(left_line[2], left_line[3]), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
	//cv::line(raw_image_, cv::Point(right_line[0], right_line[1]), cv::Point(right_line[2], right_line[3]), cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
	
	maskFusion(left_line, right_line, image_inRange, image_road_result_);

	cv::Mat image_show(raw_image_.size(), CV_8UC3, cv::Scalar(0, 0, 0));
	gray2bgr(image_road_result_, image_show);

	addWeighted(raw_image_, 0.75, image_show, 0.25, 0.0, image_show);
	cv::imshow("final image", image_show);
	cv::imwrite(file_name_ + "record/" + std::to_string(curr_index_++) + ".png", image_show);

//	std::vector<cv::Point> approx;
//	cv::approxPolyDP(cv::Mat(contours[imax]), approx, 10, true);      //多边形拟合
	
	for (int i = 0; i < lines.size(); ++i) {
		cv::Vec4i l = lines[i];
		cv::line(raw_image_, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
	}

//	const cv::Point* p = &approx[0];
//	int m = (int)approx.size();
//	std::cout << "length: " << m << std::endl;
//	polylines(raw_image_, &p, &m, 1, true, cv::Scalar(0, 0, 255), 3);

	cv::imshow("raw image", raw_image_);
//	cv::imshow("mask image", image_mask);
	
	std::cout << t1.toc() << std::endl;
	cv::waitKey(0);
}
void imageProcess::selectHSVParam() {
	cv::Mat image = cv::imread(file_name_ + "200.png");

	cv::Mat image_gauss;
	cv::GaussianBlur(image, image_gauss, cv::Size(3, 3), 0, 0);

	cv::Mat image_hsv, image_result; 
	cvtColor(image_gauss, image_hsv, cv::COLOR_BGR2HSV);

	std::vector<cv::Mat> hsvSplit;
	split(image_hsv, hsvSplit);     
	equalizeHist(hsvSplit[2], hsvSplit[2]);//直方图均衡化，对比均衡化
	merge(hsvSplit, image_hsv);//合并三通道

	cv::imshow("gauss image", image_gauss);
	cv::imshow("hsv image", image_hsv);

	setTrackBar();

	inRange(image_hsv, cv::Scalar(low_H, low_S, low_V), cv::Scalar(high_H, high_S, high_V), image_result);

	cv::Rect rect(0, image_hsv.rows / 3, image_hsv.cols, image_hsv.rows * 2 / 3);
	cv::Mat image_roi = image_result(rect);

	cv::imshow(window_detection_name, image_roi);
	cv::waitKey(1);
}
void imageProcess::detect() {
	while(1)
		loadImage();

}









}