#include "ros/ros.h"
#include "ironfish_alexa/captain_server.h"

bool add(ironfish_alexa::captain_server::Request  &req,
         ironfish_alexa::captain_server::Response &res)
{
  res.sum = req.a + req.b;
  ROS_INFO("request: x=%ld, y=%ld", (long int)req.a, (long int)req.b);
  ROS_INFO("sending back response: [%ld]", (long int)res.sum);
  return true;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "captain_server");
  ros::NodeHandle n;

  ros::ServiceServer service = n.advertiseService("orderToCaptain", add);
  ROS_INFO("Ready to add two ints.");
  ros::spin();

  return 0;
}
