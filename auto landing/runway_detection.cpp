#include <iostream>
#include "laneDetector.h"

using namespace std;
std::vector<cv::Point> vecPts;
void on_mouse(int EVENT, int x, int y, int flags, void* userdata)
{
    cv::Mat hh;
    hh = *(cv::Mat*)userdata;
    cv::Point p(x, y);
    switch (EVENT)
    {
        case cv::EVENT_LBUTTONDOWN:
        {
            cv::circle(hh, p, 2, cv::Scalar(255),3);
            cv::imshow("raw image", hh);
            vecPts.push_back(p);
        }
        break;
    }
}
	

int main()
{
    cv::VideoCapture capture("/home/chenhua/dataset/landing/video/Cessna_172SP_line_good2.mp4");
    if(!capture.isOpened())
        cout << "fail toopen!" << endl;

    //获取整个帧数
    long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
    cout << "整个视频共" << totalFrameNumber << "帧" << endl;

    //设置开始帧()
    long frameToStart = 0;
    capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
    cout << "从第" << frameToStart << "帧开始读" << endl;

    //设置结束帧
    int frameToStop = totalFrameNumber;

    if(frameToStop < frameToStart){
        cout << "结束帧小vecPts于开始帧，程序错误，即将退出！" << endl;
        return -1;
    }
    else{
        cout << "结束帧为：第" << frameToStop << "帧" << endl;
    }

    //获取帧率
    double rate = capture.get(CV_CAP_PROP_FPS);
    cout << "帧率为:" << rate << endl;

    bool stop = false, restart = true;
    cv::Mat frame;
    
    double delay = 1000 / rate;
    long currentFrame = frameToStart;
    
    cv::Vec4i left_line, right_line;
    mavsim::LaneDetector lane_detector;
    
    //save video
    int fps = rate/2;
	cv::Size size = cv::Size(1920, 1080);//size一定要和frame尺寸匹配
	cv::VideoWriter Writer("/home/chenhua/dataset/landing/video/Cessna_172SP.mp4", CV_FOURCC('D', 'I', 'V', 'X'), fps, size, true);
	
    while(currentFrame<218){
        if(!capture.read(frame)){
            cout << "读取视频失败" << endl;
            break;
        }
        
        if(currentFrame == frameToStart || restart){
            cv::imshow("raw image", frame);
            setMouseCallback("raw image", on_mouse, &frame);
//            cv::waitKey(0);
        }
        
//        restart = !lane_detector.detectRunway(restart, vecPts, left_line, right_line, frame);
        
        if(currentFrame != frameToStart)
            Writer.write(frame);
        
        int c = cv::waitKey(1);
        if((char)c == 27 || currentFrame > frameToStop){
            stop = true;
        }
        cout<<currentFrame<<endl;
        currentFrame++;
    }
    
    Writer.release();
    capture.release();
    return 0;
}
