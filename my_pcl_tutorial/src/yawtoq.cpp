#include <ros/ros.h>
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include <geometry_msgs/Pose.h>
int main(int argc, char** argv) {
    ros::init(argc, argv, "psubscriber");
    ros::NodeHandle n;
    tf2::Quaternion quat_tf;
    quat_tf.setRPY(0, 0.0, -3.14159);
    geometry_msgs::Quaternion quat_msg;
    tf2::convert(quat_tf, quat_msg);
    ROS_INFO("x %f y %f z %f w %f ",quat_msg.x,quat_msg.y,quat_msg.z,quat_msg.w);
    return 0;
}

