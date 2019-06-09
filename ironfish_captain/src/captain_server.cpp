#include "ros/ros.h"
#include "ironfish_captain/captain_command.h"
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

// define move_base action client
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
MoveBaseClient *ac_pointer;

// define infoMap as nx7 2d string array
std::vector<std::vector<std::string> > parsedCsv;


// service captain_service judging if command match the map
bool add(ironfish_captain::captain_command::Request  &req,
         ironfish_captain::captain_command::Response &res)
{
  for(int i=0; i<parsedCsv.size(); i++){
    // if goal is registed
    if(parsedCsv[i][1] == req.object && parsedCsv[i][2] == req.location){
      res.task_flag = true;

      //we'll send a goal to the robot to move 1 meter forward
      move_base_msgs::MoveBaseGoal goal;
      goal.target_pose.header.frame_id = "map";
      goal.target_pose.header.stamp = ros::Time::now();
      goal.target_pose.pose.position.x = ::atof(parsedCsv[i][3].c_str());
      goal.target_pose.pose.position.y = ::atof(parsedCsv[i][4].c_str());
      goal.target_pose.pose.orientation.z= ::atof(parsedCsv[i][5].c_str());
      goal.target_pose.pose.orientation.w = ::atof(parsedCsv[i][6].c_str());
      cout << "Sending goal (x,y,0),(0,0,z,w):" <<parsedCsv[i][3].c_str()<<","<<parsedCsv[i][4].c_str()<<","<<parsedCsv[i][5].c_str()<<","<<parsedCsv[i][6].c_str();
      ac_pointer->sendGoal(goal);
      /*
      ac_pointer->waitForResult();
      if(ac_pointer->getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
        ROS_INFO("Succed in moving to map x: %f y: %f",goal.target_pose.pose.position.x,goal.target_pose.pose.position.y);
      else
        ROS_INFO("Failed");
      */

      break;
    // if goal is NOT registed
    }else
      res.task_flag = false;
  }return true;
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "captain_server");
  ros::NodeHandle n;
  ros::ServiceServer service = n.advertiseService("captain_service", add);
  //tell the action client that we want to spin a thread by default
  MoveBaseClient ac("move_base", true);
  ac_pointer = &ac;

  //get infoMap from ros parameter
  string infoMap_path;
  n.getParam("infoMap_path", infoMap_path);
  ROS_INFO("get the infoMap at: %s",infoMap_path.c_str());

  //readfile, read map
  std::ifstream input(infoMap_path.c_str());
  std::string line;
  while(std::getline(input,line))
  {
      std::stringstream lineStream(line);
      std::string cell;
      std::vector<std::string> parsedRow;
      while(std::getline(lineStream,cell,','))
      {
          parsedRow.push_back(cell);
      }
      parsedCsv.push_back(parsedRow);
  }

  // print 2d array
  for(int i=0; i<parsedCsv.size(); i++){
    for(int j=0; j<parsedCsv[0].size(); j++){
      std::cout << parsedCsv[i][j]<<" ";
    }
    cout << endl;
  }
  ROS_INFO("InfoMap is loaded successfully...");


  //wait for the action server to come up
  while(!ac.waitForServer(ros::Duration(5.0))){
    ROS_INFO("Waiting for the move_base action server to come up");
  }
  ROS_INFO("Ready to server...");
  ros::spin();

  return 0;
}
