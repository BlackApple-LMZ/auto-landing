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
		
//	std::cout << "receiverCallbackFloat got [" << dataref << "] and [" << value << "]" << std::endl;
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

	pimageProcess_.reset(new imageProcess("E:\\ProgramData\\image1\\"));
}

autoLanding::~autoLanding() {
	std::cout << "bye..." << std::endl;
}

int autoLanding::init() {
	pXplaneUDPClient_->subscribeDataRef("sim/aircraft/view/acf_descrip[0][40]", 1);

	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot[0]", 10); //get altitude in feet
	pXplaneUDPClient_->sendCommand("sim/autopilot/hsi_select_gps"); //select gps mode

	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/heading_AHARS_deg_mag_pilot[0]", 10); //get heading in deg
	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/pitch_AHARS_deg_pilot[0]", 10); //get pitch in deg
	pXplaneUDPClient_->subscribeDataRef("sim/cockpit2/gauges/indicators/roll_AHARS_deg_pilot[0]", 10); //get roll in deg

	//check the status of the plane
	pXplaneUDPClient_->subscribeDataRef("laminar/c172/fuel/fuel_quantity_L[0]", 10); //get fuel quantity in kg
//	pXplaneUDPClient_->subscribeDataRef("laminar/c172/fuel/fuel_quantity_R[0]", 10); //right fuel are same as the left

	return 0;
}
void autoLanding::adjustDirection() {
	//adjust the direction according to image

	//first Take a screenshot
	pXplaneUDPClient_->sendCommand("sim/operation/screenshot");

//	pimageProcess_->
}
int autoLanding::taxiing() {
	pXplaneUDPClient_->sendCommand("sim/flight_controls/brakes_toggle_max"); //brake
	pXplaneUDPClient_->setDataRef("sim/multiplayer/controls/engine_throttle_request[0]", 1.0); //max engine

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

int autoLanding::test() {

//	pXplaneUDPClient_->subscribeDataRef("sim/aircraft/view/acf_descrip[0][40]", 1);
//	pXplaneUDPClient_->subscribeDataRef("sim/cockpit/autopilot/current_altitude[0]", 10);

//	pXplaneUDPClient_->sendCommand("sim/autopilot/heading");
//	pXplaneUDPClient_->sendCommand("sim/flight_controls/flaps_down");


//	pXplaneUDPClient_->sendCommand("sim/flight_controls/brakes_toggle_max"); //brake

//	pXplaneUDPClient_->setDataRef("sim/multiplayer/controls/engine_throttle_request[0]", 1.0);
	pimageProcess_->detect();

	return 0;
}

}







