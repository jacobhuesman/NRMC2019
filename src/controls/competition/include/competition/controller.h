#ifndef COMPETITION_CONTROLLER_H
#define COMPETITION_CONTROLLER_H

#include <ros/ros.h>
#include <utilities/joy.h>
#include <competition/bezier_visuals.h>
#include <waypoint_control/waypoint_control_client.h>
#include <dig_control/dig_control_client.h>

#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2/transform_datatypes.h>
#include <tf2_ros/transform_listener.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Transform.h>

namespace competition
{
  using WaypointControlState = waypoint_control::ControlState;
  using DigControlState = dig_control::ControlState;
  using utilities::Joy;
  using waypoint_control::Waypoints;

  class Controller
  {
  public:
    Controller(ros::NodeHandle *nh, ros::Rate *rate, const Waypoints &path);

    void update();
    void joyCallback(const sensor_msgs::Joy::ConstPtr &joy);

  private:
    waypoint_control::WaypointControlClient waypoint_client;
    dig_control::DigControlClient dig_client;
    Joy joy;
    //Visuals visuals;
    Waypoints waypoints;

    double dt;
    ros::NodeHandle *nh;
    ros::Rate *rate;
    ros::Subscriber joy_subscriber;
    tf2_ros::Buffer tf_buffer;
    tf2_ros::TransformListener tf_listener;
    tf2::Stamped<tf2::Transform> transform;
  };
}

#endif //COMPETITION_CONTROLLER_H
