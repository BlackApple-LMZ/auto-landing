//
// Created by limz on 2020/11/10.
// 文件相关的辅助操作
//

#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>

namespace autolanding_lmz {
	class fileUtils
	{

	public:
		fileUtils() {};
		~fileUtils() {};

		void dataCombine() {
			std::string pre_name = "E:\\ProgramData\\heading dataset\\";
			std::ofstream out(pre_name + "data.txt");
			for (int i = 80; i <= 84; i++) {
				std::cout << "file: " << i << std::endl;
				std::string file_name = pre_name + std::to_string(i) + "\\Cessna_172SP_";
				std::ifstream in_angle, in_heading;
				
				in_angle.open(pre_name + std::to_string(i) + "\\angle.txt");
				in_heading.open(pre_name + std::to_string(i) + "\\heading.txt");
				for (int j = 1; j <= 122; j++) {
					std::cout << j << " ";
					std::string name = file_name + std::to_string(j) + "_1.png";
					std::string line, line_heading;

					std::getline(in_angle, line);
					std::getline(in_heading, line_heading);
					std::stringstream ss(line);
					std::string buf;
					double angle_left{ 0.0 }, angle_right{ 0.0 }, heading = atof(line_heading.c_str());

					int count = 0;
					while (ss >> buf) {
						count++;
						if (count == 1) {
							angle_left = atof(buf.c_str());
						}
						else if (count == 2) {
							angle_right = atof(buf.c_str());
						}
						
					}
					//both are not zero
					if (abs(angle_left) > 0.00001 && abs(angle_right) > 0.00001) {
						cv::Mat image = cv::imread(name);
						cv::imshow("watch", image);
						char c = cv::waitKey(0);
						if (c == 'c') {
							continue;
						}
						out << angle_left << " " << angle_right << " " << heading << std::endl;
					}	
				}
				std::cout << std::endl;
			}
			out.close();
		}
		
	};
}

