#pragma once
 /**
 *  This file is part of tum_ardrone.
 *
 *  Copyright 2012 Jakob Engel <jajuengel@gmail.com> (Technical University of Munich)
 *  For more information see <https://vision.in.tum.de/data/software/tum_ardrone>.
 *
 *  tum_ardrone is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  tum_ardrone is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with tum_ardrone.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ESTIMATIONNODE_H
#define __ESTIMATIONNODE_H
 
#include <ardrone_autonomy/CamSelect.h>
#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/Image.h"
#include "tf/tfMessage.h"
//1. The tf package provides an implementation of a TransformBroadcaster to help make the task of publishing transforms easier. To use the TransformBroadcaster, we need to include the tf/transform_broadcaster.h header file.
/*************************************************/
#include "tf/transform_listener.h"
/*************************************************/
//1. The tf package provides an implementation of a TransformListener to help make the task of receiving transforms easier. To use the TransformListener, we need to include the tf/transform_listener.h header file. 
/*************************************************/
#include "tf/transform_broadcaster.h"
/*************************************************/
#include "std_msgs/Empty.h"
#include "std_srvs/Empty.h"
#include "ardrone_autonomy/Navdata.h"
#include "tum_ardrone/filter_state.h"
#include "std_msgs/String.h"
#include <dynamic_reconfigure/server.h>
#include "tum_ardrone/StateestimationParamsConfig.h"
#include "TooN/se3.h"


class DroneKalmanFilter;
class MapView;
class PTAMWrapper;

struct EstimationNode
{
private:
	// comm with drone
	ros::Subscriber navdata_sub; // drone navdata
	ros::Subscriber vel_sub; // to co-read contro commands sent from other thread
	ros::Subscriber vid_sub;
	ros::Time lastNavStamp;


	// comm with ptam
	//ros::Subscriber slam_info_sub; // ptam info (tracking quality) etc.
	//2.  
	
	ros::Subscriber tum_ardrone_sub;
	ros::Publisher tum_ardrone_pub;
	static pthread_mutex_t tum_ardrone_CS;

	//*****FOR FILTERING THE LOAD POSITION AND CALCULATING LOAD SPEED
	int bufferSize;
	tf::Vector3 marker_pose;
	tf::Vector3 marker_speed;
	tf::Vector3 last_pose;
	ros::Time lastPose_time;
	int tf_obs;
	float circBuffer[4][2];
	float Marker_secs;

	int selectedCam;

	/**********************************************/

	// output
	ros::Publisher dronepose_pub; //PARA PUBLICAR DRONE POSE!!
	//ros::Publisher markerpose_pub; //PARA PUBLICAR MARKER POSE!!***********

	ros::NodeHandle nh_;

	tf::TransformBroadcaster tf_broadcaster;
	//2. Here, we create a TransformListener object. Once the listener is created, it starts receiving tf transformations over the wire, and buffers them for up to 10 seconds. The TransformListener object should be scoped to persist otherwise it's cache will be unable to fill and almost every query will fail. A common method is to make the TransformListener object a member variable of a class.
/*************************************************/
	tf::TransformListener tf_sub; //HIDDEN IN ORIGINAL CODE
	tf::StampedTransform marker_transform;
/*************************************************/

	// parameters
	// every [publishFreq]ms, the node publishes the drones predicted position [predTime]ms into the future.
	// this pose can then be used to steer the drone. obviously, the larger [predTime], the worse the estimate.
	// this pose is published on /tf, and simultaneously further info is published on /ardrone/predictedPose
	ros::Duration predTime;
	int publishFreq;

	std::string navdata_channel;
	std::string control_channel;
	std::string output_channel;
	std::string video_channel_;
	std::string command_channel;



	// for navdata time-smoothing
	long lastDroneTS;
	long lastRosTS;
	long droneRosTSOffset;


	// save last navinfo received for forwarding...
	ardrone_autonomy::Navdata lastNavdataReceived;

public:
	// filter
	DroneKalmanFilter* filter;
	PTAMWrapper* ptamWrapper;
	MapView* mapView;
	std::string packagePath;

	EstimationNode();
	~EstimationNode();


	// ROS message callbacks
	void navdataCb(const ardrone_autonomy::NavdataConstPtr navdataPtr);
	void velCb(const geometry_msgs::TwistConstPtr velPtr);
	void vidCb(const sensor_msgs::ImageConstPtr img);
	void comCb(const std_msgs::StringConstPtr str);
	void dynConfCb(tum_ardrone::StateestimationParamsConfig &config, uint32_t level);

	// main pose-estimation loop
	void Loop();

	// writes a string message to "/tum_ardrone/com".
	// is thread-safe (can be called by any thread, but may block till other calling thread finishes)
	void publishCommand(std::string c);
	void reSendInfo();

	void publishTf(TooN::SE3<>, ros::Time stamp, int seq, std::string system);

	// logging stuff
	// logging stuff
	std::ofstream* logfileIMU;
	std::ofstream* logfilePTAM;
	std::ofstream* logfileFilter;
	std::ofstream* logfilePTAMRaw;
	static pthread_mutex_t logIMU_CS;
	static pthread_mutex_t logPTAM_CS;
	static pthread_mutex_t logFilter_CS;
	static pthread_mutex_t logPTAMRaw_CS;
	long currentLogID;
	long startedLogClock;

	void toogleLogging();	// switches logging on or off.
	std::string calibFile;
	int arDroneVersion;


};
#endif /* __ESTIMATIONNODE_H */