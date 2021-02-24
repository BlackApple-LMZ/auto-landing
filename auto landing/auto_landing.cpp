//
// Created by limz on 2020/8/21.
//

#include "auto_landing.h"
#include <thread>

#include <iostream>

#include <sstream>
#include <string.h>
#include <functional>

#include "XPUtils.h"

#define WIN_32

#ifdef WIN_32
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN_32

namespace autolanding_lmz {
const int INDEX = 60;

// our callback for changed values.

void autoLanding::receiverCallbackFloat(std::string dataref, float value) {
	if (dataref == "sim/cockpit2/gauges/indicators/altitude_ft_pilot[0]") {
		curr_altitude_ = value;
		if (abs(init_altitude_) < 0.000001)
			init_altitude_ = curr_altitude_;
	}
	else if (dataref == "laminar/c172/fuel/fuel_quantity_L[0]")
		fuel_quantity_L_ = value;
	else if (dataref == "laminar/c172/fuel/fuel_quantity_R[0]")
		fuel_quantity_R_ = value;
	else if (dataref == "sim/flightmodel/position/indicated_airspeed[0]")
		air_speed_ = value;
	else if (dataref == "sim/cockpit2/gauges/indicators/heading_AHARS_deg_mag_pilot[0]")
		curr_heading_ = value;
	else if (dataref == "sim/cockpit2/gauges/indicators/pitch_AHARS_deg_pilot[0]")
		curr_pitch_ = value;
	else if (dataref == "sim/cockpit2/gauges/indicators/roll_AHARS_deg_pilot[0]")
		curr_roll_ = value;
	else if (dataref == "sim/network/misc/network_time_sec[0]")
		curr_time_ = value;
		
	std::cout << "receiverCallbackFloat got [" << dataref << "] and [" << value << "]" << std::endl;
}

void autoLanding::receiverCallbackString(std::string dataref, std::string value) {
	std::cout << "receiverCallbackString got [" << dataref << "] and [" << value << "]" << std::endl;
}

autoLanding::autoLanding()
{
	std::cout << "hello." << std::endl;

	std::string host = "127.0.0.1";
	uint16_t port = 49000;

	//如果采用传递对象的方式：
	//auto fun1 = std::bind(&autoLanding::receiverCallbackFloat, *this, std::placeholders::_1, std::placeholders::_2);
	//会调用5次析构函数？？？？？ 但只调用了一次这个显示构造函数，有点迷
	//todo bind内部是怎么实现的 有时间看一下 
	auto fun1 = std::bind(&autoLanding::receiverCallbackFloat, this, std::placeholders::_1, std::placeholders::_2);
	auto fun2 = std::bind(&autoLanding::receiverCallbackString, this, std::placeholders::_1, std::placeholders::_2);
	pXplaneUDPClient_.reset(new XPlaneUDPClient(host, port, fun1, fun2));

	pXplaneUDPClient_->setDebug(0);

	//pimageProcess_.reset(new imageProcess("E:\\Games\\X-Plane 11 Global Scenery\\Output\\Cessna_172SP_"));
	pimageProcess_.reset(new imageProcess("E:\\ProgramData\\heading dataset\\"));
	pdataCollect_.reset(new dataCollect());
	pvisualize_.reset(new visualize());

	pbirdView_.reset(new birdView());
	pdigitalRecg_.reset(new digitalRecg());
	pCPSocket_.reset(new CPSocket());
	plineDetect_.reset(new lineDetect("E:\\ProgramData\\heading dataset\\"));
}

autoLanding::~autoLanding() {
	std::cout << "bye..." << std::endl;
}

int autoLanding::init() {
	pXplaneUDPClient_->subscribeDataRef("sim/aircraft/view/acf_descrip[0][40]", 1);

	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot[0]", 10); //get altitude in feet
	pXplaneUDPClient_->sendCommand("sim/autopilot/hsi_select_gps"); //select gps mode

	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/heading_AHARS_deg_mag_pilot[0]", 20); //get heading in deg
	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/pitch_AHARS_deg_pilot[0]", 10); //get pitch in deg
	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/roll_AHARS_deg_pilot[0]", 10); //get roll in deg

	pXplaneUDPClient_->subscribeDataRef("sim/network/misc/network_time_sec[0]", 20); //get current time

	//check the status of the plane
	pXplaneUDPClient_->subscribeDataRef("laminar/c172/fuel/fuel_quantity_L[0]", 10); //get fuel quantity in kg
//	pXplaneUDPClient_->subscribeDataRef("laminar/c172/fuel/fuel_quantity_R[0]", 10); //right fuel are same as the left

	return 0;
}
void autoLanding::adjustDirection() {
	//adjust the direction according to image

	//first Take a screenshot
	pXplaneUDPClient_->sendCommand("sim/operation/screenshot");

	double angle_left, angle_right;
	//pimageProcess_->adjustDirection(angle_left, angle_right);
	//std::cout << angle_left << " " << angle_right << std::endl;


	Sleep(200);
}
int autoLanding::taxiing() {
	//pXplaneUDPClient_->sendCommand("sim/flight_controls/brakes_toggle_max"); //brake
	pXplaneUDPClient_->setDataRef("sim/multiplayer/controls/engine_throttle_request[0]", 0.2); //max engine

	pXplaneUDPClient_->subscribeDataRef("sim/flightmodel/position/indicated_airspeed[0]", 10); //get indicated airspeed in kias

	while (air_speed_ < 60) {
		adjustDirection();
	}
	std::cout << "init altitude: "<< init_altitude_ <<". Prepare to take off." << std::endl;

	return 0;
}
int autoLanding::climb() {
	float r_pitch = 0.05, r_yaw = 0.0;
	float i = 0.001; 
	
	//keep climbing
	while (curr_altitude_ - init_altitude_ < 700) {
		pXplaneUDPClient_->setDataRef("sim/joystick/yoke_pitch_ratio[0]", r_pitch); //elevator r [-1, 1]
//		pXplaneUDPClient_->setDataRef("sim/joystick/yoke_roll_ratio[0]", r_yaw);
		
//		pXplaneUDPClient_->setDataRef("sim/flightmodel/position/vh_ind_fpm[0]", 3.0);
		
		if (curr_pitch_ > 8)
			r_pitch -= i * (curr_pitch_ - 8);
		else 
			r_pitch += i * (8 - curr_pitch_);
		
		Sleep(50); //50ms
	}
	std::cout << "current altitude: " << curr_altitude_ << ". Prepare to auto flight." << std::endl;

	return 0;
}
int autoLanding::autoPilot() {
	pXplaneUDPClient_->setDataRef("sim/cockpit/autopilot/autopilot_mode[0]", 2); //turn on AP
//	pXplaneUDPClient_->sendCommand("sim/autopilot/heading"); //choose HDG
	pXplaneUDPClient_->sendCommand("sim/autopilot/altitude_hold"); //choose ALT

	while (air_speed_ < 100);
	pXplaneUDPClient_->sendCommand("sim/autopilot/NAV"); //choose NAV

	//自动驾驶一次爬升太高会失速
	while (abs(curr_roll_) > 5.0);
	pXplaneUDPClient_->setDataRef("sim/cockpit/autopilot/current_altitude[0]", 2000); //altitude hold 2000

	while (curr_altitude_ < 1995 || abs(curr_roll_) > 5.0);
	pXplaneUDPClient_->setDataRef("sim/cockpit/autopilot/current_altitude[0]", 2500); //altitude hold 2500

	while (curr_altitude_ < 2495 || abs(curr_roll_) > 5.0);
	pXplaneUDPClient_->setDataRef("sim/cockpit/autopilot/current_altitude[0]", 3000); //altitude hold 3000

	return 0;
}
int autoLanding::launch() {
	init();
	taxiing();

	climb();
	autoPilot();

	return 0;
}
int autoLanding::collectData() {

	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/heading_AHARS_deg_mag_pilot[0]", 20);
	//	pXplaneUDPClient_->subscribeDataRef("sim/network/misc/network_time_sec[0]", 20); //get current time
	std::cout << "start collect. Press enter to finish." << std::endl;
	int count = 35;
	
	while (1) {
		char ch = pdataCollect_->waitKey();
		if (ch == 27) {
			break;
		}
		std::cout << count << " " << curr_heading_ << std::endl;
		std::ofstream out("E:\\ProgramData\\heading dataset\\" + std::to_string(count) + "\\heading.txt");
		Sleep(2000); //等切换到xplane窗口
		float x = 32473.5, z = -20520.0;
		
		pXplaneUDPClient_->setDataRef("sim/flightmodel/position/local_x[0]", x);
		pXplaneUDPClient_->setDataRef("sim/flightmodel/position/local_z[0]", z);
		//每个角度先采61张x方向变化的图 有的位置有的角度的图不一定能用 
		for (int i = -30; i < 31; i++) {
			float temp = x + 0.5 * i;
			pXplaneUDPClient_->setDataRef("sim/flightmodel/position/local_x[0]", temp);
			Sleep(1000);
			out << curr_heading_ << std::endl;
			pdataCollect_->keyPress();

		}
		pXplaneUDPClient_->setDataRef("sim/flightmodel/position/local_x[0]", 32473.5);
		//再采61张z方向变化的图 有的位置有的角度的图不一定能用 
		for (int i = -30; i < 31; i++) {
			float temp = z + 0.5 * i;
			pXplaneUDPClient_->setDataRef("sim/flightmodel/position/local_z[0]", temp);
			Sleep(1000);
			//std::cout << temp << " " << curr_heading_ << std::endl;
			out << curr_heading_ << std::endl;
			pdataCollect_->keyPress();
		}
		pXplaneUDPClient_->setDataRef("sim/flightmodel/position/local_z[0]", -20520.0);
		
		Sleep(1000); //等一会儿时间 不然太快了 最后一张还没有保存下来
		//将图片剪切到相应的文件夹下
		for (int i = 1; i <= 122; i++) {
			std::string file1 = "E:\\Games\\X-Plane 11 Global Scenery\\Output\\Cessna_172SP_" + std::to_string(i) + ".png";
			std::string file2 = "E:\\ProgramData\\heading dataset\\" + std::to_string(count) + "\\Cessna_172SP_" + std::to_string(i) + ".png";
			if (!MoveFileA(file1.c_str(), file2.c_str()))
			{
				DWORD getlasterror;
				getlasterror = GetLastError();
				std::cout << getlasterror << std::endl;
				printf_s("剪切失败");
				return -1;
			}
		}
		
		//
		out.close();
		count++;
	}
	std::cout << "finish collect." << std::endl;
	return 0;
}
int autoLanding::writeData() {

	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/heading_AHARS_deg_mag_pilot[0]", 20);
	pXplaneUDPClient_->subscribeDataRef("sim/network/misc/network_time_sec[0]", 20); //get current time
	std::ofstream out("E:\\ProgramData\\heading dataset\\test\\data.txt");
	
	while (1) {
		/*char ch = pdataCollect_->waitKey();
		if (ch == 27) {
			break;
		}*/

		//std::cout << curr_time_ << " " << curr_heading_ << std::endl;
		out << curr_time_ << " " << curr_heading_ << std::endl;
	}
	out.close();
	std::cout << "finish collect." << std::endl;
	return 0;
}
int autoLanding::test() {

//	pXplaneUDPClient_->subscribeDataRef("sim/aircraft/view/acf_descrip[0][40]", 1);
//	pXplaneUDPClient_->subscribeDataRef("sim/cockpit/autopilot/current_altitude[0]", 10);

//	pXplaneUDPClient_->sendCommand("sim/autopilot/heading");
//	pXplaneUDPClient_->sendCommand("sim/flight_controls/flaps_down");


//	pXplaneUDPClient_->sendCommand("sim/flight_controls/brakes_toggle_max"); //brake

//	pXplaneUDPClient_->setDataRef("sim/multiplayer/controls/engine_throttle_request[0]", 1.0);
	pimageProcess_->detect();
//	pimageProcess_->collectData();
//	writeData();

	return 0;
}

int autoLanding::test_visual1() {
	int count{ 1 };

	char ch = pdataCollect_->waitKey();
	if (ch == 27) {
		return 1;
	}

	while (1) {
		pdataCollect_->keyPress();
		//wait until Image Process has stopped
		/*while (!pimageProcess_->isStopped()) {
			Sleep(1000);
		}*/

		//wait until dl has stopped
		/*while (!pdl_->isStopped()) {
			Sleep(1000);
		}*/
		if(pvisualize_->show(count))
			++count;
		Sleep(250);
	}

	return 0;
}
void readTextfile(std::string filename, double& heading) {
	std::string line;
	std::ifstream in_stream(filename.c_str());

	while (!in_stream.eof()) {
		std::getline(in_stream, line);
		std::stringstream ss(line);
		std::string buf;
		int count = 0;
		while (ss >> buf) {
			count++;
			if (count == 2) {
				;
			}
			else if (count == 4) {
				;
			}
			else if (count == 1) {
				heading = atof(buf.c_str());
			}

		}
	}
	in_stream.close();
}
int autoLanding::test_visual() {

	cv::VideoCapture capture("E:\\Games\\X-Plane 11 Global Scenery\\Output\\test.avi");
	if (!capture.isOpened())
		std::cout << "fail to open!" << std::endl;

	long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);

	int frameToStart{0}, frameToStop = totalFrameNumber;
	capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);

	bool stop = false, restart = true;
	cv::Mat frame;

	int currentFrame = frameToStart;

	//std::thread visual(&imageProcess::show, pimageProcess_);
	//visual.join();

	std::thread tbirdView(&birdView::run, pbirdView_);
	tbirdView.detach();

	std::thread tdigitalRecg(&digitalRecg::run, pdigitalRecg_);
	tdigitalRecg.detach();

	std::thread tlineDetect(&lineDetect::run, plineDetect_);
	tlineDetect.detach();

	/*std::thread tCPSocket(&CPSocket::run, pCPSocket_);
	tCPSocket.detach();

	//等待连接成功
	while (!pCPSocket_->isConnected()) {
		;
	}*/

	while (currentFrame < totalFrameNumber) {
		if (!capture.read(frame)) {
			std::cout << "读取视频失败" << std::endl;
			break;
		}
		cv::imwrite("E:\\project\\auto landing\\auto landing\\auto landing\\image\\image.png", frame);

		// 激活birdview 激活其他处理的线程 然后等待处理完 然后显示
		
		pdigitalRecg_->requestStart(frame);
		plineDetect_->requestStart(frame);
		//pCPSocket_->requestStart();

		//wait until BirdView Process has stopped
		while (/*!pbirdView_->isStopped() || */!pdigitalRecg_->isStopped() || !plineDetect_->isStopped()){// || !pCPSocket_->isStopped()) {
			;
		}

		//之前这个顺序写反了 导致birdview线程开始处理 但是contour还没有来得及传入//
		pbirdView_->setContour(plineDetect_->getContour());
		pbirdView_->setPosition(pdigitalRecg_->getHeading(), pdigitalRecg_->getPitch(), pdigitalRecg_->getRoll(), pdigitalRecg_->getX(), pdigitalRecg_->getY(), pdigitalRecg_->getZ());
		pbirdView_->requestStart(frame);

		while (!pbirdView_->isStopped()) {
			;
		}

		double predictHeading{ 0.0 };
		readTextfile("E:\\project\\auto landing\\auto landing\\auto landing\\image\\data.txt", predictHeading);

		pvisualize_->setRawImage(frame);
		pvisualize_->setBirdImage(pbirdView_->getBirdView());
		//pvisualize_->setLineImage(plineDetect_->getDetectLine());
		//pvisualize_->setHeading(predictHeading, pdigitalRecg_->getHeading(), currentFrame);

		std::cout << "currentFrame: " << currentFrame << " " << pdigitalRecg_->getHeading() << " " << pdigitalRecg_->getPitch() << " " << pdigitalRecg_->getRoll();
		std::cout << " " << pdigitalRecg_->getX() << " " << pdigitalRecg_->getY() << " " << pdigitalRecg_->getZ() << std::endl;

		pvisualize_->show();
		++currentFrame;
	}

	capture.release();
	return 0;
}
}







