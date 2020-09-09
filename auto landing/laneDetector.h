#ifndef _LANEDETECTOR_H_
#define _LANEDETECTOR_H_

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace mavsim
{
class LaneDetector 
{
public:
	//find left and right lines in area
    bool detectRunway(bool restart, std::vector<cv::Point> vecPts, cv::Vec4i &left_line, cv::Vec4i &right_line, cv::Mat &frame);
    LaneDetector();
    ~LaneDetector();
    
private:
    float GetCross(cv::Point& p1, cv::Point& p2, cv::Point& p);
    //check the point whether in the area
    bool IsPointInMatrix(cv::Point p, const std::vector<cv::Point> &vecPts);

    //get the skeleton of the image
    cv::Mat thinImage(const cv::Mat &src, const int maxIterations = -1);
    
    //compute slope of all lines
    void computeSlope(std::vector<cv::Vec4i> lines, std::vector<double> &slopes);
    
    //separate all lines to left and right lines
    void lineSeparation(std::vector<cv::Vec4i> lines, std::vector<cv::Point> vecPoints, std::vector<double> slopes, std::vector<cv::Vec4i> &left_lines, std::vector<cv::Vec4i> &right_lines, double slope_thresh = 0.5);
    //separate all lines to left and right lines without area detection
    void lineSeparation(std::vector<cv::Vec4i> lines, std::vector<double> slopes, std::vector<cv::Vec4i> &left_lines, std::vector<cv::Vec4i> &right_lines, cv::Vec4i left_line, cv::Vec4i right_line, double slope_thresh = 0.5);
    
    //regress the line and Get only one line for each side of the lane
    void regression(std::vector<cv::Vec4i> lines, const cv::Mat &inputImage, cv::Vec4i &fit_line);    
    
    //generate the slide window for line detection
    void generateMask(const cv::Mat &src, const std::vector<cv::Point> &vecPts, cv::Mat &mask);
    
    //fill vecPts with last detected line
    void fillVecPts(cv::Vec4i line, std::vector<cv::Point> &vecPts, const cv::Mat &inputImage);
};

}

#endif
