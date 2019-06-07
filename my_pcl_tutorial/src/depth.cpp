#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include "darknet_ros_msgs/BoundingBoxes.h"
#include <pcl/io/pcd_io.h>
#include <math.h>
#include <geometry_msgs/Pose.h>
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "std_msgs/String.h"
#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
geometry_msgs::Pose objectpose;
sensor_msgs::PointCloud2 objectpoint;
float rx=0.0;
float ry=0.0;
double rt=0.0;
double ot=0.0;
double ct=0.0;
int oindex= 0;
class object
{
public:
geometry_msgs::Pose posi;
std_msgs::String name;
};
object objectlist[10];  
typedef pcl::PointCloud<pcl::PointXYZ> PointCloud;
bool checkobject(float x,float y){
float threshold = 0.3;
float distance = 0.0;
for(int i=0;i<oindex;i++){
distance=sqrt(pow(objectlist[i].posi.position.x-x,2)+pow(objectlist[i].posi.position.y-y,2));
if(distance<threshold)
   return false;
}//end for
return true;
}
void objectincert(float d,std_msgs::String na){
tf2::Quaternion quat_tf;
geometry_msgs::Quaternion quat_msg;
ct=rt+ot;
if(ct<-3.14)
     ct=ct+6.283;
if(ct>3.14)
     ct=ct-6.283;
quat_tf.setRPY(0.0, 0.0, ct);
tf2::convert(quat_tf, quat_msg);
if(checkobject(rx+d*cos(ct),ry+d*sin(ct))){
objectlist[oindex].posi.position.x=rx+d*cos(ct);
objectlist[oindex].posi.position.y=ry+d*sin(ct);
objectlist[oindex].posi.orientation=quat_msg;
objectlist[oindex].name=na;
oindex++;
ROS_INFO("x %f y %f orient %f ",rx+d*cos(ct),ry+d*sin(ct),ct);
}//end if

}
//--------------------------------------------------------------
void poseAMCLCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msgAMCL)
{
    rx = msgAMCL->pose.pose.position.x;
    ry = msgAMCL->pose.pose.position.y;
    //poseAMCLa = msgAMCL->pose.pose.orientation.w;
    tf::Quaternion q(
    msgAMCL->pose.pose.orientation.x,
    msgAMCL->pose.pose.orientation.y,
    msgAMCL->pose.pose.orientation.z,
    msgAMCL->pose.pose.orientation.w);
    tf::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);
    rt = yaw;
   // ROS_INFO("X %f Y %f Theta %f",rx,ry,rt);   

}
void depthCallback(const sensor_msgs::PointCloud2ConstPtr& pCloud) {
objectpoint=*pCloud;
}
float bounddepth(int x,int y) {// EXTRACT DEPTH
int width = objectpoint.width;
int height = objectpoint.height;
//int width=640;
//int height=480;
if(objectpoint.width!=0){
int arrayPosition = y*objectpoint.row_step + x*objectpoint.point_step;
int arrayPosX = arrayPosition + objectpoint.fields[0].offset; // X has an offset of 0
int arrayPosY = arrayPosition + objectpoint.fields[1].offset; // Y has an offset of 4
int arrayPosZ = arrayPosition + objectpoint.fields[2].offset; // Z has an offset of 8
float X = 0.0;
float Y = 0.0;
float Z = 0.0;
memcpy(&X, &objectpoint.data[arrayPosX], sizeof(float));
memcpy(&Y, &objectpoint.data[arrayPosY], sizeof(float));
memcpy(&Z, &objectpoint.data[arrayPosZ], sizeof(float));
//ROS_INFO("W%d H%d Rs%d ps%d ",width,height,objectpoint.row_step,objectpoint.point_step);
if(isnan(X)){
   //ROS_INFO("nan");
   return -1;
}
else{
    return Z;
}//end else
}//end if
else
   return -1;
}
void box(const darknet_ros_msgs::BoundingBoxes::ConstPtr& msg)
{
     
        /*ROS_INFO("Bouding Boxes (Class): %s",msg->bounding_boxes[0].Class.c_str());
        ROS_INFO("Bouding Boxes (probability): %f",msg->bounding_boxes[0].probability);
	ROS_INFO("Bouding Boxes (xmin): %d",msg->bounding_boxes[0].xmin);
	ROS_INFO("Bouding Boxes (xmax): %d",msg->bounding_boxes[0].xmax);
	ROS_INFO("Bouding Boxes (ymin): %d",msg->bounding_boxes[0].ymin);
	ROS_INFO("Bouding Boxes (ymax): %d", msg->bounding_boxes[0].ymax);*/
          
        int xcenter = 0;
        int ycenter = 0;
        std_msgs::String name;
        float z=0.0;
        float z1=0.0;
        for(int i=0;i<msg->bounding_boxes.size();i++)
        {
                //ROS_INFO("data %d",i);
           if(msg->bounding_boxes[0].Class=="cup"){
                  name.data="cup";
                  xcenter =(msg->bounding_boxes[0].xmin+msg->bounding_boxes[0].xmax)/2;
                  ycenter =(msg->bounding_boxes[0].ymin+msg->bounding_boxes[0].ymax)/2;
                  //ROS_INFO("object x: %d  y: %d ",xcenter,ycenter);
                  //ROS_INFO("aobject");        
                  z=bounddepth(xcenter,ycenter);
                  //ROS_INFO("bobject");
                  //z1=bounddepth(xcenter+2,ycenter);
                  if(z!=-1){
                      ot=atan2(xcenter-320,320);
                       objectincert(z,name);
                      }
                  else
                      ot=0;
                  
                  //ROS_INFO("object x: %d  y: %d depth %f orient %f",xcenter,ycenter,z,(ot*180)/3.14159);
          }//end if
         }

        
         

}
/**
 * Node main function
 */
int main(int argc, char** argv) {
    ros::init(argc, argv, "psubscriber");
    ros::NodeHandle n;
    ros::Subscriber sub_amcl = n.subscribe("amcl_pose",1, poseAMCLCallback);
    ros::Subscriber subDepth = n.subscribe("/camera/depth_registered/points", 1, depthCallback);
    ros::Subscriber sub = n.subscribe("/darknet_ros/bounding_boxes", 1, box);

    ros::spin();
    return 0;
}

