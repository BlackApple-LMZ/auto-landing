//
// Created by limz on 2021/01/25.
// 显示界面的相关操作
//

#ifndef VISUALIZE_H
#define VISUALIZE_H


#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

namespace autolanding_lmz {
    class visualize {
    public:
        visualize();
        ~visualize();
        
		//设置显示的鸟瞰图
		void setBirdImage(const cv::Mat& raw_image);
		void setRawImage(const cv::Mat& image);
		void setLineImage(const cv::Mat& image);

        bool getStatus(){ return breturn_;};
        bool getAdd(){ return badd_;};

		void setHeading(double predictHeading, double realHeading, int index);

		bool show();
		bool show(int index);
    private:
		void init();
		//显示heading error的图
		void drawHeadingImage();

		//灰度图转bgr图片 
		void gray2bgr(cv::Mat gray, cv::Mat& bgr);
		//将gray图片上面区域置成0
		void cutGray(cv::Mat& gray, int th);
		void drawLineImage(cv::Mat& gray, int th);

		std::string pre_name_ = "E:\\Games\\X-Plane 11 Global Scenery\\Output\\Cessna_172SP_";
        bool bnext_, bicp_, breturn_, badd_;

		cv::Mat bird_image_, raw_image_, heading_image_, line_image_;
		double predictHeading_, realHeading_;

		//动态显示误差图 通过deque维护一个数据窗口//
		std::deque<int> traj_real_, traj_predict_;

		//heading error的一些统计信息//
		int count_{ 0 }; 
		double sum_error_{ 0.0 }, sum_error_square_{ 0.0 };

		int index_{0};
    };
}



#endif //
