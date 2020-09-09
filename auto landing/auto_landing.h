//
// Created by limz on 2020/8/21.
// 自主着陆的主要的节点
//

#ifndef AUTO_LANDING_H
#define AUTO_LANDING_H

#include <iostream>
#include <vector>

#include "XPlaneUDPClient.h"
#include "image_process.h"

namespace autolanding_lmz{
    class autoLanding {
    public:
		autoLanding();
        ~autoLanding();

		int test();

		int launch();
    private:
		int init();
		int taxiing();
		int climb();
		int autoPilot();

		void adjustDirection();

		void receiverCallbackFloat(std::string dataref, float value);
		void receiverCallbackString(std::string dataref, std::string value);

        std::shared_ptr<XPlaneUDPClient> pXplaneUDPClient_;
		std::shared_ptr<imageProcess> pimageProcess_;
		/*
        std::shared_ptr<cloudOperate> pcloudOperate_;
        std::shared_ptr<optimize> poptimize_;*/

		//status of plane
		float init_altitude_ = 0.0;
		float curr_altitude_ = 0.0;
		float fuel_quantity_L_ = 0.0, fuel_quantity_R_ = 0.0;
		float air_speed_ = 0.0;
		float curr_heading_ = 0.0, curr_pitch_ = 0.0, curr_roll_ = 0.0;
    };
}



#endif //AUTO_LANDING_H
