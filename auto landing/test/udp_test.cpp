//
// Created by limz on 2021/3/17.
//

#include "XPlaneUDPClient.h"
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



void receiverCallbackFloat(std::string dataref, float value) {
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

void receiverCallbackString(std::string dataref, std::string value) {
	std::cout << "receiverCallbackString got [" << dataref << "] and [" << value << "]" << std::endl;
}

int test_udp() {
	std::cout << "hello." << std::endl;

	std::string host = "127.0.0.1";
	uint16_t port = 49000;

	std::shared_ptr<XPlaneUDPClient> pXplaneUDPClient_;
	pXplaneUDPClient_.reset(new XPlaneUDPClient(host, port, std::bind(receiverCallbackFloat, std::placeholders::_1,
		std::placeholders::_2),
		std::bind(receiverCallbackString, std::placeholders::_1,
			std::placeholders::_2)));
	pXplaneUDPClient_->setDebug(0);

	pXplaneUDPClient_->subscribeDataRef("sim/aircraft/view/acf_descrip[0][40]", 1);
	pXplaneUDPClient_->subscribeDataRef("sim/cockpit/autopilot/current_altitude[0]", 10);
	pXplaneUDPClient_->subscribeDataRef("sim/graphics/view/projection_matrix[1]", 10);

	pXplaneUDPClient_->sendCommand("sim/autopilot/heading");
	pXplaneUDPClient_->sendCommand("sim/flight_controls/flaps_down");

	pXplaneUDPClient_->sendCommand("sim/flight_controls/brakes_toggle_max"); //brake
	pXplaneUDPClient_->setDataRef("sim/multiplayer/controls/engine_throttle_request[0]", 1.0);


	return 0;
}








