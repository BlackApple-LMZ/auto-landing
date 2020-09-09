#include "laneDetector.h"
#include <iostream>
#include <map>
#include <set>

namespace mavsim
{
#define WINDOW_SIZE 10

LaneDetector::LaneDetector(){
    
}

LaneDetector::~LaneDetector(){
    
}
float LaneDetector::GetCross(cv::Point& p1, cv::Point& p2, cv::Point& p)
{
	return (p2.x - p1.x) * (p.y - p1.y) -(p.x - p1.x) * (p2.y - p1.y);
}
bool LaneDetector::IsPointInMatrix(cv::Point p, const std::vector<cv::Point> &vecPts)
{
	cv::Point p1 = vecPts[0];
	cv::Point p2 = vecPts[1];
	cv::Point p3 = vecPts[2];
	cv::Point p4 = vecPts[3];
 
	return GetCross(p1,p2,p) * GetCross(p3,p4,p) >= 0 && GetCross(p2,p3,p) * GetCross(p4,p1,p) >= 0;
}

void LaneDetector::generateMask(const cv::Mat &src, const std::vector<cv::Point> &vecPts, cv::Mat &mask)
{
	int minX = std::min(std::min(std::min(vecPts[0].x, vecPts[1].x), vecPts[2].x), vecPts[3].x);
	int	maxX = std::max(std::max(std::max(vecPts[0].x, vecPts[1].x), vecPts[2].x), vecPts[3].x);
	int	minY = std::min(std::min(std::min(vecPts[0].y, vecPts[1].y), vecPts[2].y), vecPts[3].y);
	int	maxY = std::max(std::max(std::max(vecPts[0].y, vecPts[1].y), vecPts[2].y), vecPts[3].y);
 
	for (size_t j= minY; j<maxY; ++j)
	{
		const uchar* pS = src.ptr<uchar>(j);
		uchar* pM = mask.ptr<uchar>(j);
 
		for (size_t i = minX; i < maxX; ++i)
		{
			// in the region.
			if (IsPointInMatrix(cv::Point(i, j), vecPts))
			{
				pM[i] = pS[i];
			}
		}
	}
 
	return ;
}

void LaneDetector::regression(std::vector<cv::Vec4i> lines, const cv::Mat &inputImage, cv::Vec4i &fit_line){
	cv::Point ini, fini;
	cv::Vec4d line;
	std::vector<cv::Point> line_pts;
	
    cv::Point b;
    double m;
    
	// If lines are being detected, fit a line using all the init and final points of the lines
	for(auto i : lines){
		ini = cv::Point(i[0], i[1]);
		fini = cv::Point(i[2], i[3]);

		line_pts.push_back(ini);
		line_pts.push_back(fini);
	}

	if(line_pts.size() > 0){
		// The line is formed here
		cv::fitLine(line_pts, line, CV_DIST_L2, 0, 0.01, 0.01);
		m = line[1] / line[0];
		b = cv::Point(line[2], line[3]);
	}

	// One the slope and offset points have been obtained, apply the line equation to obtain the line points
	int ini_y = inputImage.rows*0.48;
	int fin_y = inputImage.rows*0.2;

    //check if the start point or the end point is beyond the edge
	double ini_x = ((ini_y - b.y) / m) + b.x;
	if(ini_x < 0 || ini_x >= inputImage.cols){
	    ini_x = 0;
	    ini_y = ((ini_x - b.x) * m) + b.y;
	}
	else if(ini_x >= inputImage.cols){
	    ini_x = inputImage.cols;
	    ini_y = ((ini_x - b.x) * m) + b.y;
	}
	
	double fin_x = ((fin_y - b.y) / m) + b.x;
    if(fin_x < 0 || fin_x >= inputImage.cols){
	    fin_x = 0;
	    fin_y = ((fin_x - b.x) * m) + b.y;
	}
	else if(fin_x >= inputImage.cols){
	    fin_x = inputImage.cols;
	    fin_y = ((fin_x - b.x) * m) + b.y;
	}
	
    fit_line[0] = ini_x; 
    fit_line[1] = ini_y; 
    fit_line[2] = fin_x;
    fit_line[3] = fin_y;
    
	return ;
}

void LaneDetector::computeSlope(std::vector<cv::Vec4i> lines, std::vector<double> &slopes) {
	size_t j = 0;
	cv::Point ini;
	cv::Point fini;

	// Calculate the slope of all the detected lines
	for (auto i : lines) {
		ini = cv::Point(i[0], i[1]);
		fini = cv::Point(i[2], i[3]);

		// Basic algebra: slope = (y1 - y0)/(x1 - x0)
		double slope = (static_cast<double>(fini.y) - static_cast<double>(ini.y)) / (static_cast<double>(fini.x) - static_cast<double>(ini.x) + 0.00001);

		slopes.push_back(slope);
	}

	return ;
}

void LaneDetector::lineSeparation(std::vector<cv::Vec4i> lines, std::vector<double> slopes, std::vector<cv::Vec4i> &left_lines, std::vector<cv::Vec4i> &right_lines, cv::Vec4i left_line, cv::Vec4i right_line, double slope_thresh) {

    cv::Point ini = cv::Point(left_line[0], left_line[1]);
	cv::Point fini = cv::Point(left_line[2], left_line[3]);
    double left_slope = (static_cast<double>(fini.y) - static_cast<double>(ini.y)) / (static_cast<double>(fini.x) - static_cast<double>(ini.x) + 0.00001);
    
    ini = cv::Point(right_line[0], right_line[1]);
	fini = cv::Point(right_line[2], right_line[3]);
    double right_slope = (static_cast<double>(fini.y) - static_cast<double>(ini.y)) / (static_cast<double>(fini.x) - static_cast<double>(ini.x) + 0.00001);
    
	// Split the lines into right and left lines
	for(int i=0; i<lines.size(); i++){
	    
		// If the slope is too horizontal, discard the line
		if(std::abs(slopes[i]) < slope_thresh){
			continue;
		}
	
		// Condition to classify line as left side or right side
		//slope>0, besides the start point and end point are both in right area 
		if(slopes[i] > 0 && std::abs(slopes[i]-right_slope) < 0.1){
		    std::cout<<slopes[i]<<std::endl;
			right_lines.push_back(lines[i]);
		}
		else if(slopes[i] < 0 && std::abs(slopes[i]-left_slope) < 0.1) {
		    std::cout<<slopes[i]<<std::endl;
			left_lines.push_back(lines[i]);
		}
	}
	return ;
}

void LaneDetector::lineSeparation(std::vector<cv::Vec4i> lines, std::vector<cv::Point> vecPoints, std::vector<double> slopes, std::vector<cv::Vec4i> &left_lines, std::vector<cv::Vec4i> &right_lines, double slope_thresh) {
	cv::Point ini;
	cv::Point fini;

    std::vector<cv::Point> vecPointsLeft, vecPointsRight;
    vecPointsLeft.resize(4);
    vecPointsRight.resize(4);
    vecPointsLeft[0] = vecPoints[0];
    vecPointsLeft[1] = vecPoints[1];
    vecPointsLeft[2] = (vecPoints[1]+vecPoints[2])/2;
    vecPointsLeft[3] = (vecPoints[0]+vecPoints[3])/2;
    
    vecPointsRight[0] = (vecPoints[0]+vecPoints[3])/2;
    vecPointsRight[1] = (vecPoints[1]+vecPoints[2])/2;
    vecPointsRight[2] = vecPoints[2];
    vecPointsRight[3] = vecPoints[3];
    
	// Split the lines into right and left lines
	for(int i=0; i<lines.size(); i++){
	    std::cout<<slopes[i]<<std::endl;
		// If the slope is too horizontal, discard the line
		if(std::abs(slopes[i]) < slope_thresh){
			continue;
		}
	
		ini = cv::Point(lines[i][0], lines[i][1]);
		fini = cv::Point(lines[i][2], lines[i][3]);

		// Condition to classify line as left side or right side
		//slope>0, besides the start point and end point are both in right area 
		if(slopes[i] > 0 && IsPointInMatrix(ini, vecPointsRight) && IsPointInMatrix(fini, vecPointsRight)){
			right_lines.push_back(lines[i]);
		}
		else if(slopes[i] < 0 && IsPointInMatrix(ini, vecPointsLeft) && IsPointInMatrix(fini, vecPointsLeft)) {
			left_lines.push_back(lines[i]);
		}
	}
	return ;
}

cv::Mat LaneDetector::thinImage(const cv::Mat &src, const int maxIterations)
{
    assert(src.type() == CV_8UC1);
    cv::Mat dst;
    int width = src.cols;
    int height = src.rows;
    src.copyTo(dst);
    int count = 0;  //记录迭代次数
    while (true)
    {
        count++;
        if (maxIterations != -1 && count > maxIterations) //限制次数并且迭代次数到达
            break;
        std::vector<uchar *> mFlag; //用于标记需要删除的点

        //对点标记
        for (int i = 0; i < height; ++i)
        {
            uchar * p = dst.ptr<uchar>(i);
            for (int j = 0; j < width; ++j)
            {
                //如果满足四个条件，进行标记
                //  p9 p2 p3
                //  p8 p1 p4
                //  p7 p6 p5
                uchar p1 = p[j];
                //cout << (int)p1 << endl;
                if (p1 != 1) continue;
                uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
                uchar p8 = (j == 0) ? 0 : *(p + j - 1);
                uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
                uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
                uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
                uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
                uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
                uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
                if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
                {
                    int ap = 0;
                    if (p2 == 0 && p3 == 1) ++ap;
                    if (p3 == 0 && p4 == 1) ++ap;
                    if (p4 == 0 && p5 == 1) ++ap;
                    if (p5 == 0 && p6 == 1) ++ap;
                    if (p6 == 0 && p7 == 1) ++ap;
                    if (p7 == 0 && p8 == 1) ++ap;
                    if (p8 == 0 && p9 == 1) ++ap;
                    if (p9 == 0 && p2 == 1) ++ap;

                    if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
                    {
                        //标记
                        mFlag.push_back(p + j);
                    }
                }
            }
        }

        //将标记的点删除
        for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
        {
            **i = 0;
        }

        //直到没有点满足，算法结束
        if (mFlag.empty())
        {
            break;
        }
        else
        {
            mFlag.clear();//将mFlag清空
        }

        //对点标记
        for (int i = 0; i < height; ++i)
        {
            uchar * p = dst.ptr<uchar>(i);
            for (int j = 0; j < width; ++j)
            {
                //如果满足四个条件，进行标记
                //  p9 p2 p3
                //  p8 p1 p4
                //  p7 p6 p5
                uchar p1 = p[j];
                if (p1 != 1) continue;
                uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
                uchar p8 = (j == 0) ? 0 : *(p + j - 1);
                uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
                uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
                uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
                uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
                uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
                uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);

                if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
                {
                    int ap = 0;
                    if (p2 == 0 && p3 == 1) ++ap;
                    if (p3 == 0 && p4 == 1) ++ap;
                    if (p4 == 0 && p5 == 1) ++ap;
                    if (p5 == 0 && p6 == 1) ++ap;
                    if (p6 == 0 && p7 == 1) ++ap;
                    if (p7 == 0 && p8 == 1) ++ap;
                    if (p8 == 0 && p9 == 1) ++ap;
                    if (p9 == 0 && p2 == 1) ++ap;

                    if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
                    {
                        //标记
                        mFlag.push_back(p + j);
                    }
                }
            }
        }

        //将标记的点删除
        for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
        {
            **i = 0;
        }

        //直到没有点满足，算法结束
        if (mFlag.empty())
        {
            break;
        }
        else
        {
            mFlag.clear();//将mFlag清空
        }
    }
    return dst;
}
void LaneDetector::fillVecPts(cv::Vec4i line, std::vector<cv::Point> &vecPts, const cv::Mat &inputImage){

    if(line[0] < WINDOW_SIZE){
        vecPts.push_back(cv::Point(line[0], line[1]-WINDOW_SIZE));
        vecPts.push_back(cv::Point(line[0]+WINDOW_SIZE, line[1]));
    }
    else if(line[0] > inputImage.cols-WINDOW_SIZE){
        vecPts.push_back(cv::Point(line[0]-WINDOW_SIZE, line[1]));
        vecPts.push_back(cv::Point(line[0], line[1]-WINDOW_SIZE));
    }
    else{
        vecPts.push_back(cv::Point(line[0]-WINDOW_SIZE, line[1]));
        vecPts.push_back(cv::Point(line[0]+WINDOW_SIZE, line[1]));
    }
    
    if(line[2] < WINDOW_SIZE){
        vecPts.push_back(cv::Point(line[2]+WINDOW_SIZE, line[3]));
        vecPts.push_back(cv::Point(line[2], line[3]+WINDOW_SIZE));
    }
    else if(line[2] > inputImage.cols-WINDOW_SIZE){
        vecPts.push_back(cv::Point(line[2], line[3]+WINDOW_SIZE));
        vecPts.push_back(cv::Point(line[2]-WINDOW_SIZE, line[3]));
    }
    else{
        vecPts.push_back(cv::Point(line[2]+WINDOW_SIZE, line[3]));
        vecPts.push_back(cv::Point(line[2]-WINDOW_SIZE, line[3]));
    }
//    std::cout<<vecPts[0].x<<" "<<vecPts[0].y<<" "<<vecPts[1].x<<" "<<vecPts[1].y<<" "
//             <<vecPts[2].x<<" "<<vecPts[2].y<<" "<<vecPts[3].x<<" "<<vecPts[3].y<<" "<<std::endl;
}

bool LaneDetector::detectRunway(bool restart, std::vector<cv::Point> vecPts, cv::Vec4i &left_line, cv::Vec4i &right_line, cv::Mat &frame){

    cv::Mat img_gray;
    cv::cvtColor(frame, img_gray, cv::COLOR_BGR2GRAY);
    cv::Mat img_mask = cv::Mat::zeros(img_gray.size(), CV_8UC1);
    if(restart){
        generateMask(img_gray, vecPts, img_mask);
    }
    else{
        std::vector<cv::Point> vecPtsLeft, vecPtsRight;
        fillVecPts(left_line, vecPtsLeft, frame);
        fillVecPts(right_line, vecPtsRight, frame);
        
        generateMask(img_gray, vecPtsLeft, img_mask);
        generateMask(img_gray, vecPtsRight, img_mask);
    }
    cv::imshow("mask image", img_mask);
//    cv::waitKey(0);
    
    cv::Mat img_canny;
    // apply different param
    if(restart){
        cv::Canny(img_gray, img_canny, 60, 80, 3);
    }
    else{
        cv::Canny(img_gray, img_canny, 20, 40, 3);
    }
    
    cv::bitwise_and(img_canny, img_mask, img_canny);
    imshow("canny", img_canny);

    cv::Mat img_se;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::dilate(img_canny, img_se, element);

    cv::Mat img_bin;
    cv::threshold(img_se, img_bin, 127, 1, cv::THRESH_BINARY);

    cv::Mat img_skel = thinImage(img_bin);
    img_skel *= 255;
    imshow("thin image", img_skel);

    std::vector<cv::Vec4i> lines, left_lines, right_lines;
    cv::HoughLinesP(img_skel, lines, 1, CV_PI / 180, 20, 23, 10); // 20 23 8 //右边的维持了很久
    
    std::vector<double> slopes;
    computeSlope(lines, slopes);
    
    if(restart)
        lineSeparation(lines, vecPts, slopes, left_lines, right_lines);
    else{
        lineSeparation(lines, slopes, left_lines, right_lines, left_line, right_line);
    }
        
    regression(left_lines, frame, left_line);
    regression(right_lines, frame, right_line);
    
    cv::Mat temp = frame.clone();
    
    cv::Vec4i l = left_line;
//    cv::line(frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
//    cout<<l[0]<<" "<<l[1]<<" "<<l[2]<<" "<<l[3]<<endl;
    
    l = right_line;
//    cv::line(frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
//    cout<<l[0]<<" "<<l[1]<<" "<<l[2]<<" "<<l[3]<<endl;

    imshow("raw image", frame);
    
    for(int i=0; i<lines.size(); ++i){
        cv::Vec4i l = lines[i];
        cv::line(frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }
	imshow("raw lines", frame);

    return true;
}

}
