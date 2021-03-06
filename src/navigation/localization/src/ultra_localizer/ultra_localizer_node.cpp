#include <ros/ros.h>
#include <ros/console.h> //for debug/info
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/transform_broadcaster.h>

#include <geometry_msgs/Pose2D.h>
#include <geometry_msgs/TransformStamped.h>
#include <std_msgs/Empty.h>
#include <localization/StateVector.h>

#include <measurement_manager/measurement_manager.h>
#include <ultra_localizer/ultra_localizer.h>
#include <sensor_msgs/JointState.h>

#include <visualization_msgs/Marker.h>

#include <vrep_access/vrep_imu.h>

#include <lp_research/lpresearchimu.h>
#include <apriltag_tracker_interface/apriltag_tracker_interface.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <vector>
#include <utility>


geometry_msgs::TransformStamped createTf(double x, double y, double theta, uint seq)
{
  geometry_msgs::TransformStamped transform;
  transform.header.seq = seq;
  transform.header.stamp = ros::Time::now();
  transform.header.frame_id = "map";
  transform.child_frame_id = "base_link";
  transform.transform.translation.x = x;
  transform.transform.translation.y = y;
  transform.transform.translation.z = 0.0;
  tf2::Quaternion orientation;
  orientation.setRPY(0.0, 0.0, theta);
  transform.transform.rotation.x = orientation.x();
  transform.transform.rotation.y = orientation.y();
  transform.transform.rotation.z = orientation.z();
  transform.transform.rotation.w = orientation.w();
  return transform;
}

geometry_msgs::TransformStamped createSimTf(double x, double y, double theta)
{
  geometry_msgs::TransformStamped tfStamp;
  tfStamp.header.stamp = ros::Time::now();
  tfStamp.header.frame_id = "map";
  tfStamp.child_frame_id = "sim_base_link";
  tfStamp.transform.translation.x = x;
  tfStamp.transform.translation.y = y;
  tfStamp.transform.translation.z = 0.5;
  tf2::Quaternion q;
  q.setRPY(0, 0, theta);
  tfStamp.transform.rotation.x = q.x();
  tfStamp.transform.rotation.y = q.y();
  tfStamp.transform.rotation.z = q.z();
  tfStamp.transform.rotation.w = q.w();
  return tfStamp;
}

localization::StateVector toMsg(LocalizerInterface::StateVector sv, uint seq)
{
  localization::StateVector sv_msg;
  sv_msg.header.stamp = ros::Time::now();
  sv_msg.header.seq = seq;
  sv_msg.id = 0;
  sv_msg.x_pos = sv.x_pos;
  sv_msg.y_pos = sv.y_pos;
  sv_msg.theta = sv.theta;
  sv_msg.x_vel = sv.x_vel;
  sv_msg.y_vel = sv.y_vel;
  sv_msg.omega = sv.omega;
  sv_msg.x_accel = sv.x_accel;
  sv_msg.y_accel = sv.y_accel;
  sv_msg.alpha   = sv.alpha;
  return sv_msg;
}

int main(int argc, char **argv)
{
  // Initialize ROS and load params
  ros::init(argc, argv, "ultra_localizer_node");
  ros::NodeHandle nh("~");
  ros::Rate rate(50.0);
  tf2_ros::Buffer tfBuffer;
  tf2_ros::TransformBroadcaster tf_broadcaster;
  geometry_msgs::TransformStamped transform;
  ros::Publisher state_publisher = nh.advertise<localization::StateVector>("/state_vector", 1);

  uint seq = 0;
  bool simulating; nh.param<bool>("simulating", simulating, false);
  if (simulating) ROS_WARN("Running as simulation");
  double settling_time; nh.param<double>("settling_time", settling_time, 10);
  ROS_INFO("Localization settling time: %f", settling_time);

  // Initialize localizer
  ImuSensorInterface *imu;
  PosSensorInterface *pos;
  MeasurementManager mm;
  if (simulating)
  {
    pos = new AprilTagTrackerInterface("/vrep/pose", .1);
    imu = new VrepImu(0, .0001, 0, .0001);
  }
  else
  {
    pos = new AprilTagTrackerInterface("/tracker/pose_estimate", .1);
    imu = new LpResearchImu("imu");
  }
  mm.giveImu(imu, 0, 0, 0);
  mm.givePos(pos);
  ROS_INFO("Initialized sensors");

  UltraLocalizer localizer(UltraLocalizer_default_gains, UltraLocalizer_initial_estimate);
  LocalizerInterface::StateVector state_vector;

  // Wait for filter to settle
  ros::Time start_time = ros::Time::now();
  while (((ros::Time::now() - start_time).toSec() < settling_time) && ros::ok()) //((!superLocalizer.getIsDataGood() && ros::ok()))
  {
    if (simulating)
    {
      tf_broadcaster.sendTransform(createSimTf(pos->getX(), pos->getY(), pos->getTheta()));
    }
    localizer.updateEstimate(UltraLocalizer_zero_vector, mm.getMeasured(.02));
    ros::spinOnce();
    rate.sleep();
  }
  ROS_INFO ("Localization settled");

  // Start localization loop
  double dt = rate.expectedCycleTime().toSec();
  while (ros::ok())
  {
    seq++;
    if (simulating)
    {
      tf_broadcaster.sendTransform(createSimTf(pos->getX(), pos->getY(), pos->getTheta()));
    }
    localizer.updateEstimate(UltraLocalizer_zero_vector, mm.getMeasured(dt));
    state_vector = localizer.getStateVector();
    tf_broadcaster.sendTransform(createTf(state_vector.x_pos, state_vector.y_pos, state_vector.theta, seq));
    state_publisher.publish(toMsg(state_vector, seq));
    ros::spinOnce();
    rate.sleep();
  }
}
