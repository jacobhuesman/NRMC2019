#ifndef COMPETITION_CONTROLLER_H
#define COMPETITION_CONTROLLER_H

#include <ros/ros.h>
#include <competition/joy.h>
#include <competition/visuals.h>
#include <drive_controller/drive_control_client.h>
#include <dig_control/dig_control_client.h>

namespace competition
{
  class Controller
  {
  public:
    Controller(ros::NodeHandle *nh);


  private:
    drive_controller::DriveControlClient drive_client;
    dig_control::DigControlClient dig_client;
    Joy joy;

    ros::NodeHandle *nh;
    ros::Subscriber joy_subscriber;
  };
}

#endif //COMPETITION_CONTROLLER_H
