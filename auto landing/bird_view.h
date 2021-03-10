//
// Created by limz on 2021/01/27.
// 投影变换得到鸟瞰图
//

#ifndef BIRD_VIEW_H
#define BIRD_VIEW_H

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include <mutex>

#define DEG2RAD 0.01745329252f
namespace autolanding_lmz {

    class birdView {
    public:
		birdView();
        ~birdView();

		//auto_landing 请求开始计算鸟瞰图
		void requestStart(const cv::Mat& image);
		bool isStopped();
		void setFinish();
		cv::Mat getBirdView();
		cv::Mat getPerspective();

		void setContour(const std::vector<cv::Point>& contour);
		//设置飞行状态参数，计算相对初始时刻的姿态变换矩阵//
		void setPosition(double heading, double pitch, double roll, double x, double y, double z);

		void test();
		//main function
		void run();
    private:
		//采用四组点对来计算H矩阵 然后做反投影//
		void computeBirdview();
		//采用补偿手动投影的方式进行反变换//
		void fieldView();
		//构建ipm表 后面计算就只需要查表就ok
		void build_ipm_table(const int srcw, const int srch, const int dstw, const int dsth, int* maptable);

		void inverse_perspective_mapping(const int dstw, const int dsth, const unsigned char* src, const int* maptable, unsigned char* dst);
		//eigen to cv mat
		cv::Mat toCvMat(const Eigen::Matrix4d &m);

		std::mutex mutexStop_, mutexRequestStart_;
		bool startRequested_{ false }, stopped_{ false };

		cv::Mat birdView_image_;
		cv::Mat raw_image_;
		std::vector<cv::Point> contour_;

		cv::Mat perspective_image_;

		Eigen::Matrix4d T_origin_, T_origin_inv_;
		Eigen::Matrix4d T_cur_, Toc_;

		Eigen::Matrix4d M_per_;

		Eigen::Matrix4d OpenGL_projectionMatrix_, OpenGL_worldMatrix_, OpenGL_modelviewMatrix_;
		int index_ = 0;

		//for IPM
		double FOV_H_ = 80.0f, FOV_V_ = 60.0f;//deg alpha
		//x方向是向前 z是向右 y是向上 右手坐标系//
		double dx_ = 0.0, dy_ = 1.8, dz_ = 0.0; //position of camera in world 
		double gamma_ = 0.0, theta_ = 0.0; //heading pitch
		double m_ = 720, n_ = 1280; //height width

		cv::Mat M_;
    };
}



#endif //BIRD_VIEW_H
