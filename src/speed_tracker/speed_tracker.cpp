/* includes //{ */

#include <ros/ros.h>

#include <geometry_msgs/PoseStamped.h>
#include <nav_msgs/Odometry.h>

#include <mrs_msgs/TrackerPointStamped.h>
#include <mrs_uav_manager/Tracker.h>

#include <tf/transform_datatypes.h>
#include <mutex>

#include <commons.h>

#include <mrs_lib/ParamLoader.h>
#include <mrs_lib/Profiler.h>

#include <mrs_msgs/SpeedTrackerCommand.h>

//}

#define STOP_THR 1e-3

namespace mrs_trackers
{

namespace speed_tracker
{

/* //{ class SpeedTracker */

class SpeedTracker : public mrs_uav_manager::Tracker {
public:
  virtual void initialize(const ros::NodeHandle &parent_nh, mrs_uav_manager::SafetyArea_t const *safety_area,
                          mrs_uav_manager::Transformer_t const *transformer);
  virtual bool activate(const mrs_msgs::PositionCommand::ConstPtr &cmd);
  virtual void deactivate(void);

  virtual const mrs_msgs::PositionCommand::ConstPtr update(const mrs_msgs::UavState::ConstPtr &msg);
  virtual const mrs_msgs::TrackerStatus             getStatus();
  virtual const std_srvs::SetBoolResponse::ConstPtr enableCallbacks(const std_srvs::SetBoolRequest::ConstPtr &cmd);
  virtual void                                      switchOdometrySource(const mrs_msgs::UavState::ConstPtr &msg);

  virtual const mrs_msgs::Vec4Response::ConstPtr goTo(const mrs_msgs::Vec4Request::ConstPtr &cmd);
  virtual const mrs_msgs::Vec4Response::ConstPtr goToRelative(const mrs_msgs::Vec4Request::ConstPtr &cmd);
  virtual const mrs_msgs::Vec1Response::ConstPtr goToAltitude(const mrs_msgs::Vec1Request::ConstPtr &cmd);
  virtual const mrs_msgs::Vec1Response::ConstPtr setYaw(const mrs_msgs::Vec1Request::ConstPtr &cmd);
  virtual const mrs_msgs::Vec1Response::ConstPtr setYawRelative(const mrs_msgs::Vec1Request::ConstPtr &cmd);

  virtual bool goTo(const mrs_msgs::TrackerPointStampedConstPtr &msg);
  virtual bool goToRelative(const mrs_msgs::TrackerPointStampedConstPtr &msg);
  virtual bool goToAltitude(const std_msgs::Float64ConstPtr &msg);
  virtual bool setYaw(const std_msgs::Float64ConstPtr &msg);
  virtual bool setYawRelative(const std_msgs::Float64ConstPtr &msg);

  virtual const mrs_msgs::TrackerConstraintsResponse::ConstPtr setConstraints(const mrs_msgs::TrackerConstraintsRequest::ConstPtr &cmd);

  virtual const std_srvs::TriggerResponse::ConstPtr hover(const std_srvs::TriggerRequest::ConstPtr &cmd);

private:
  bool callbacks_enabled = true;

  std::string uav_name_;

  double external_command_timeout_;

private:
  mrs_msgs::UavState uav_state;
  bool               got_uav_state = false;
  std::mutex         mutex_uav_state;

  double uav_x;
  double uav_y;
  double uav_z;
  double uav_yaw;
  double uav_roll;
  double uav_pitch;

private:
  // tracker's inner states
  int    tracker_loop_rate_;
  double tracker_dt_;
  bool   is_initialized = false;
  bool   is_active      = false;
  bool   first_iter     = false;

private:
  // dynamical constraints
  double     yaw_rate_;
  std::mutex mutex_constraints;

private:
  // desired goal
  double                        got_command = false;
  std::mutex                    mutex_command;
  mrs_msgs::SpeedTrackerCommand external_command;
  ros::Time                     external_command_time;

  mrs_msgs::PositionCommand output;

private:
  ros::Subscriber subscriber_command;
  void            callbackCommand(const mrs_msgs::SpeedTrackerCommand &msg);

private:
  mrs_lib::Profiler *profiler;
  bool               profiler_enabled_ = false;
  bool               position_mode_    = false;
  bool               tilt_mode_        = false;
};

//}

// | -------------- tracker's interface routines -------------- |

/* //{ initialize() */

void SpeedTracker::initialize(const ros::NodeHandle &parent_nh, [[maybe_unused]] mrs_uav_manager::SafetyArea_t const *safety_area,
                              [[maybe_unused]] mrs_uav_manager::Transformer_t const *transformer) {

  ros::NodeHandle nh_(parent_nh, "speed_tracker");

  ros::Time::waitForValid();

  // --------------------------------------------------------------
  // |                       load parameters                      |
  // --------------------------------------------------------------

  mrs_lib::ParamLoader param_loader(nh_, "SpeedTracker");

  param_loader.load_param("uav_name", uav_name_);

  param_loader.load_param("command_timeout", external_command_timeout_);

  param_loader.load_param("enable_profiler", profiler_enabled_);

  // --------------------------------------------------------------
  // |                          profiler                          |
  // --------------------------------------------------------------

  profiler = new mrs_lib::Profiler(nh_, "SpeedTracker", profiler_enabled_);

  // --------------------------------------------------------------
  // |                         subscribers                        |
  // --------------------------------------------------------------

  subscriber_command = nh_.subscribe("command_in", 1, &SpeedTracker::callbackCommand, this, ros::TransportHints().tcpNoDelay());

  if (!param_loader.loaded_successfully()) {
    ROS_ERROR("[SpeedTracker]: Could not load all parameters!");
    ros::shutdown();
  }

  is_initialized = true;

  ROS_INFO("[SpeedTracker]: initialized");
}

//}

/* //{ activate() */

bool SpeedTracker::activate([[maybe_unused]] const mrs_msgs::PositionCommand::ConstPtr &cmd) {

  if (!got_uav_state) {
    ROS_ERROR("[SpeedTracker]: can't activate(), odometry not set");
    return false;
  }

  std::scoped_lock lock(mutex_command);

  if (!got_command) {

    ROS_ERROR("[SpeedTracker]: cannot activate, missing command");
    return false;
  }

  // timeout the external command
  if ((ros::Time::now() - external_command_time).toSec() > external_command_timeout_) {
    ROS_ERROR("[SpeedTracker]: cannot activate, the command is too old");
    return false;
  }

  // --------------------------------------------------------------
  // |              yaw initial condition  prediction             |
  // --------------------------------------------------------------

  is_active = true;

  ROS_INFO("[SpeedTracker]: activated");

  return true;
}

//}

/* //{ deactivate() */

void SpeedTracker::deactivate(void) {

  is_active   = false;

  ROS_INFO("[SpeedTracker]: deactivated");
}

//}

/* //{ update() */

const mrs_msgs::PositionCommand::ConstPtr SpeedTracker::update(const mrs_msgs::UavState::ConstPtr &msg) {

  mrs_lib::Routine profiler_routine = profiler->createRoutine("update");

  {
    std::scoped_lock lock(mutex_uav_state);

    uav_state = *msg;
    uav_x     = uav_state.pose.position.x;
    uav_y     = uav_state.pose.position.y;
    uav_z     = uav_state.pose.position.z;

    // calculate the euler angles
    tf::Quaternion quaternion_odometry;
    quaternionMsgToTF(uav_state.pose.orientation, quaternion_odometry);
    tf::Matrix3x3 m(quaternion_odometry);
    m.getRPY(uav_roll, uav_pitch, uav_yaw);

    got_uav_state = true;
  }

  // up to this part the update() method is evaluated even when the tracker is not active
  if (!is_active) {
    return mrs_msgs::PositionCommand::Ptr();
  }

  // timeout the external command
  if (got_command && (ros::Time::now() - external_command_time).toSec() > external_command_timeout_) {
    ROS_ERROR("[SpeedTracker]: command timeouted, returning nil");
    return mrs_msgs::PositionCommand::Ptr();
  }

  output.header.stamp    = ros::Time::now();
  output.header.frame_id = uav_state.header.frame_id;

  {
    std::scoped_lock lock(mutex_uav_state, mutex_command);

    output.position.x = uav_state.pose.position.x;
    output.position.y = uav_state.pose.position.y;

    if (external_command.use_horizontal_velocity) {
      output.velocity.x              = external_command.velocity.x;
      output.velocity.y              = external_command.velocity.y;
      output.use_velocity_horizontal = true;
    } else {
      output.velocity.x              = uav_state.velocity.linear.x;
      output.velocity.y              = uav_state.velocity.linear.y;
      output.use_velocity_horizontal = false;
    }

    if (external_command.use_vertical_velocity) {
      output.velocity.z            = external_command.velocity.z;
      output.use_velocity_vertical = true;
    } else {
      output.velocity.z            = uav_state.velocity.linear.z;
      output.use_velocity_vertical = false;
    }

    if (external_command.use_height) {
      output.position.z            = external_command.height;
      output.use_position_vertical = true;
    } else {
      output.position.z            = uav_state.pose.position.z;
      output.use_position_vertical = false;
    }

    if (external_command.use_acceleration) {
      output.acceleration.x   = external_command.acceleration.x;
      output.acceleration.y   = external_command.acceleration.y;
      output.acceleration.z   = external_command.acceleration.z;
      output.use_acceleration = true;
    } else {
      output.acceleration.x   = uav_state.acceleration.linear.x;
      output.acceleration.y   = uav_state.acceleration.linear.y;
      output.acceleration.z   = uav_state.acceleration.linear.z;
      output.use_acceleration = false;
    }

    if (external_command.use_yaw) {
      output.yaw     = external_command.yaw;
      output.use_yaw = true;
    } else {
      output.yaw     = uav_yaw;
      output.use_yaw = false;
    }

    if (external_command.use_yaw_dot) {
      output.yaw_dot     = external_command.yaw_dot;
      output.use_yaw_dot = true;
    } else {
      output.yaw_dot     = uav_state.velocity.angular.z;
      output.use_yaw_dot = false;
    }
  }

  return mrs_msgs::PositionCommand::ConstPtr(new mrs_msgs::PositionCommand(output));
}

//}

/* //{ getStatus() */

const mrs_msgs::TrackerStatus SpeedTracker::getStatus() {

  mrs_msgs::TrackerStatus tracker_status;

  tracker_status.active            = is_active;
  tracker_status.callbacks_enabled = callbacks_enabled;

  return tracker_status;
}

//}

/* //{ enableCallbacks() */

const std_srvs::SetBoolResponse::ConstPtr SpeedTracker::enableCallbacks(const std_srvs::SetBoolRequest::ConstPtr &cmd) {

  char                      message[100];
  std_srvs::SetBoolResponse res;

  if (cmd->data != callbacks_enabled) {

    callbacks_enabled = cmd->data;

    sprintf((char *)&message, "Callbacks %s", callbacks_enabled ? "enabled" : "disabled");

    ROS_INFO("[SpeedTracker]: %s", message);

  } else {

    sprintf((char *)&message, "Callbacks were already %s", callbacks_enabled ? "enabled" : "disabled");
  }

  res.message = message;
  res.success = true;

  return std_srvs::SetBoolResponse::ConstPtr(new std_srvs::SetBoolResponse(res));
}

//}

/* switchOdometrySource() //{ */

void SpeedTracker::switchOdometrySource([[maybe_unused]] const mrs_msgs::UavState::ConstPtr &msg) {
}

//}

// | -------------- setpoint topics and services -------------- |

/* //{ goTo() service */

const mrs_msgs::Vec4Response::ConstPtr SpeedTracker::goTo([[maybe_unused]] const mrs_msgs::Vec4Request::ConstPtr &cmd) {

  return mrs_msgs::Vec4Response::Ptr();
}

//}

/* //{ goTo() topic */

bool SpeedTracker::goTo([[maybe_unused]] const mrs_msgs::TrackerPointStampedConstPtr &msg) {

  return false;
}

//}

/* //{ goToRelative() service */

const mrs_msgs::Vec4Response::ConstPtr SpeedTracker::goToRelative([[maybe_unused]] const mrs_msgs::Vec4Request::ConstPtr &cmd) {

  return mrs_msgs::Vec4Response::Ptr();
}

//}

/* //{ goToRelative() topic */

bool SpeedTracker::goToRelative([[maybe_unused]] const mrs_msgs::TrackerPointStampedConstPtr &msg) {

  return false;
}

//}

/* //{ goToAltitude() service */

const mrs_msgs::Vec1Response::ConstPtr SpeedTracker::goToAltitude([[maybe_unused]] const mrs_msgs::Vec1Request::ConstPtr &cmd) {

  return mrs_msgs::Vec1Response::Ptr();
}

//}

/* //{ goToAltitude() topic */

bool SpeedTracker::goToAltitude([[maybe_unused]] const std_msgs::Float64ConstPtr &msg) {

  return false;
}

//}

/* //{ setYaw() service */

const mrs_msgs::Vec1Response::ConstPtr SpeedTracker::setYaw([[maybe_unused]] const mrs_msgs::Vec1Request::ConstPtr &cmd) {

  return mrs_msgs::Vec1Response::Ptr();
}

//}

/* //{ setYaw() topic */

bool SpeedTracker::setYaw([[maybe_unused]] const std_msgs::Float64ConstPtr &msg) {

  return false;
}

//}

/* //{ setYawRelative() service */

const mrs_msgs::Vec1Response::ConstPtr SpeedTracker::setYawRelative([[maybe_unused]] const mrs_msgs::Vec1Request::ConstPtr &cmd) {

  return mrs_msgs::Vec1Response::Ptr();
}

//}

/* //{ setYawRelative() topic */

bool SpeedTracker::setYawRelative([[maybe_unused]] const std_msgs::Float64ConstPtr &msg) {

  return false;
}

//}

/* //{ hover() service */

const std_srvs::TriggerResponse::ConstPtr SpeedTracker::hover([[maybe_unused]] const std_srvs::TriggerRequest::ConstPtr &cmd) {

  return std_srvs::TriggerResponse::Ptr();
}

//}

/* //{ setConstraints() service */

const mrs_msgs::TrackerConstraintsResponse::ConstPtr SpeedTracker::setConstraints([[maybe_unused]] const mrs_msgs::TrackerConstraintsRequest::ConstPtr &cmd) {

  return mrs_msgs::TrackerConstraintsResponse::Ptr();
}

//}

// | --------------------- custom methods --------------------- |

/* callbackCommand() //{ */

void SpeedTracker::callbackCommand(const mrs_msgs::SpeedTrackerCommand &msg) {

  if (!is_initialized)
    return;

  std::scoped_lock lock(mutex_command);

  mrs_lib::Routine profiler_routine = profiler->createRoutine("callbackCommand");

  external_command = msg;

  got_command = true;

  external_command_time = ros::Time::now();

  if (!is_active) {
    ROS_INFO_ONCE("[SpeedTracker]: getting command");
  } else {
    ROS_INFO_THROTTLE(5.0, "[SpeedTracker]: getting command");
  }
}

//}

}  // namespace speed_tracker

}  // namespace mrs_trackers

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(mrs_trackers::speed_tracker::SpeedTracker, mrs_uav_manager::Tracker)
