#include "vrep_interface/vrep_server.h"
#include <rosgraph_msgs/Clock.h>
#include <vrep_interface/vrep_server.h>

using namespace vrep_interface;

VREPServer::VREPServer(SimInterface *sim_interface)
{
  sim = sim_interface;

  int argc = 0; char **argv = NULL;
  ros::init(argc, argv, "vrep");
  if (!ros::master::check())
  {
    simAddStatusbarMessage("[WARN]: Unable to start, ros master isn't running");
    throw std::runtime_error("Unable to start, ros master isn't running");
  }
  nh = new ros::NodeHandle("~");
  std::cout << "Starting server..." << std::endl;

  std::string description_path = ros::package::getPath("description");
  if (description_path.empty())
  {
    sim->error("Unable to find the description package path, have you sourced your workspace?");
    throw std::runtime_error("Unable to find the description package path, have you sourced your workspace?");
  }

  std::cout << "Initialized" << std::endl;

  spawn_robot_server = nh->advertiseService("spawn_robot", &VREPServer::spawnRobotService, this);
  spawn_robot_random_server = nh->advertiseService("spawn_robot_random", &VREPServer::spawnRobotRandomService, this);
  shutdown_vrep_server = nh->advertiseService("shutdown", &VREPServer::shutdownService, this);
  tf_broadcaster = new tf::TransformBroadcaster();
  clock_publisher = new ros::Publisher;
  (*clock_publisher) = nh->advertise<rosgraph_msgs::Clock>("/clock", 10, true);

  sim->loadScene(description_path + "/vrep_models/arena.ttt");

  robot = new VREPRobot(nullptr);
  robot->initialize(description_path + "/vrep_models/robot.ttm");
  robot->spawnRobot();

  sim_running = false;
  sim->info("Server started");
}

VREPServer::~VREPServer()
{
  spawn_robot_server.shutdown();
  spawn_robot_random_server.shutdown();
  shutdown_vrep_server.shutdown();
  robot->shutdown();
  nh->shutdown();
  ros::shutdown();
}

void VREPServer::spinOnce()
{
  // Disable error reporting (it is enabled in the service processing part, but
  // we don't want error reporting for publishers/subscribers)
  sim->disableErrorReporting();

  // Process all requested services and topic subscriptions
  try
  {
    if (sim_running)
    {
      rosgraph_msgs::Clock sim_clock = sim->getSimulationTime();
      clock_publisher->publish(sim_clock);

      robot->spinOnce();

      tf::Transform robot_position;
      robot->getPosition(&robot_position);
      tf_broadcaster->sendTransform(tf::StampedTransform(robot_position, ros::Time::now(), "map", "base_link"));
    }
    ros::spinOnce();
  }
  catch (const std::exception &e)
  {
    sim->error(e.what());
  }

  sim->resumeErrorReporting();
}

void VREPServer::simulationAboutToStart()
{
  sim_running = true;
}

void VREPServer::simulationEnded()
{
  sim_running = false;
}

bool VREPServer::spawnRobotRandomService(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res)
{
  try
  {
    robot->spawnRobot();
    return true;
  }
  catch (const std::runtime_error &e)
  {
    sim->error(e.what());
    return false;
  }
}

bool VREPServer::spawnRobotService(vrep_msgs::SpawnRobot::Request &req,
                                   vrep_msgs::SpawnRobot::Response &res)
{
  try
  {
    robot->spawnRobot(req.x, req.y, req.omega);
  }
  catch (const std::runtime_error &e)
  {
    sim->error(e.what());
    return false;
  }
}

bool VREPServer::shutdownService(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res)
{
  sim->info("[shutdownService] Trying to shutdown");
  res.success = 1;
  res.message = "Trying to shutdown...";
  simQuitSimulator(1);
  return true;
}