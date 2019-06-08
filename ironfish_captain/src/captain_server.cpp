#include "ros/ros.h"
#include "ironfish_captain/captain_command.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

// define infoMap as nx7 2d string array
std::vector<std::vector<std::string> > parsedCsv;

bool add(ironfish_captain::captain_command::Request  &req,
         ironfish_captain::captain_command::Response &res)
{
  for(int i=0; i<parsedCsv.size(); i++){
    if(parsedCsv[i][1] == req.object && parsedCsv[i][2] == req.location){
      res.task_flag = true;
      break;
    }else{
      res.task_flag = false;
    }
  }
  return true;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "captain_server");
  ros::NodeHandle n;
  ros::ServiceServer service = n.advertiseService("captain_service", add);

  //get infoMap
  string infoMap_path;
  n.getParam("infoMap_path", infoMap_path);
  ROS_INFO("get the infoMap at: %s",infoMap_path.c_str());

  //readfile
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

  ROS_INFO("Ready to server...");
  ros::spin();

  return 0;
}
