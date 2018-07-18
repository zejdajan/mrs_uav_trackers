#include <ros/ros.h>

#include <geometry_msgs/PoseStamped.h>
#include <mrs_msgs/TrackerDiagnostics.h>
#include <mrs_msgs/TrackerPointStamped.h>
#include <mrs_mav_manager/Tracker.h>
#include <nav_msgs/Odometry.h>
#include <mrs_lib/Profiler.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tf/transform_datatypes.h>
#include <mutex>

#include <commons.h>

#define STOP_THR 1e-3

namespace mrs_trackers
{

// state machine
typedef enum {

  IDLE_STATE,
  STOP_MOTION_STATE,
  HOVER_STATE,
  ACCELERATING_STATE,
  DECELERATING_STATE,
  STOPPING_STATE,

} States_t;

const char *state_names[6] = {

    "IDLING", "STOP_MOTION_STATE", "HOVERING", "ACCELERATING", "DECELERATING", "STOPPING"};

class LineTracker : public mrs_mav_manager::Tracker {
public:
  LineTracker(void);

  void initialize(const ros::NodeHandle &parent_nh);
  bool activate(const mrs_msgs::PositionCommand::ConstPtr &cmd);
  void deactivate(void);

  const mrs_msgs::PositionCommand::ConstPtr update(const nav_msgs::Odometry::ConstPtr &msg);
  const mrs_msgs::TrackerStatus::Ptr status();

  virtual const mrs_msgs::Vec4Response::ConstPtr goTo(const mrs_msgs::Vec4Request::ConstPtr &cmd);
  virtual const mrs_msgs::Vec4Response::ConstPtr goToRelative(const mrs_msgs::Vec4Request::ConstPtr &cmd);

  virtual const std_srvs::TriggerResponse::ConstPtr hover(const std_srvs::TriggerRequest::ConstPtr &cmd);

private:
  nav_msgs::Odometry odometry;
  bool               got_odometry = false;
  std::mutex         mutex_odometry;

  double odometry_x;
  double odometry_y;
  double odometry_z;
  double odometry_yaw;
  double odometry_roll;
  double odometry_pitch;

private:
  // tracker's inner states
  int    tracker_loop_rate_;
  double tracker_dt_;
  bool   is_initialized;
  bool   is_active;
  bool   first_iter;

private:
  void mainTimer(const ros::TimerEvent &event);
  ros::Timer main_timer;

private:
  States_t current_state_vertical;
  States_t previous_state_vertical;
  States_t current_state_horizontal;
  States_t previous_state_horizontal;

  void changeStateHorizontal(States_t new_state);
  void changeStateVertical(States_t new_state);
  void changeState(States_t new_state);

private:
  void stopHorizontalMotion(void);
  void stopVerticalMotion(void);
  void accelerateHorizontal(void);
  void accelerateVertical(void);
  void decelerateHorizontal(void);
  void decelerateVertical(void);
  void stopHorizontal(void);
  void stopVertical(void);

private:
  // dynamical constraints
  double horizontal_speed_;
  double vertical_speed_;
  double horizontal_acceleration_;
  double vertical_acceleration_;
  double yaw_rate_;
  double yaw_gain_;

private:
  // desired goal
  double goal_x, goal_y, goal_z, goal_yaw;
  double have_goal = false;

  // my current state
  double state_x, state_y, state_z, state_yaw;
  double speed_x, speed_y, speed_yaw;
  double current_heading, current_vertical_direction, current_vertical_speed, current_horizontal_speed;

  mrs_msgs::PositionCommand position_output;

private:
  mrs_lib::Profiler *profiler;
  mrs_lib::Routine * routine_main_timer;
};

void LineTracker::changeStateHorizontal(States_t new_state) {

  previous_state_horizontal = current_state_horizontal;
  current_state_horizontal  = new_state;

  // just for ROS_INFO
  ROS_INFO("[LineTracker]: Switching horizontal state %s -> %s", state_names[previous_state_horizontal], state_names[current_state_horizontal]);
}

void LineTracker::changeStateVertical(States_t new_state) {

  previous_state_vertical = current_state_vertical;
  current_state_vertical  = new_state;

  // just for ROS_INFO
  ROS_INFO("[LineTracker]: Switching vertical state %s -> %s", state_names[previous_state_vertical], state_names[current_state_vertical]);
}

void LineTracker::changeState(States_t new_state) {

  previous_state_horizontal = current_state_horizontal;
  current_state_horizontal  = new_state;

  previous_state_vertical = current_state_vertical;
  current_state_vertical  = new_state;

  // just for ROS_INFO
  ROS_INFO("[LineTracker]: Switching vertical and horizontal states %s, %s -> %s", state_names[previous_state_vertical], state_names[previous_state_horizontal],
           state_names[current_state_vertical]);
}

LineTracker::LineTracker(void) : is_initialized(false), is_active(false) {
}

//{ initialize()

void LineTracker::initialize(const ros::NodeHandle &parent_nh) {

  ros::NodeHandle nh_(parent_nh, "line_tracker");

  ros::Time::waitForValid();

  // --------------------------------------------------------------
  // |                       load parameters                      |
  // --------------------------------------------------------------

  nh_.param("horizontal_tracker/horizontal_speed", horizontal_speed_, -1.0);
  nh_.param("horizontal_tracker/horizontal_acceleration", horizontal_acceleration_, -1.0);

  nh_.param("vertical_tracker/vertical_speed", vertical_speed_, -1.0);
  nh_.param("vertical_tracker/vertical_acceleration", vertical_acceleration_, -1.0);

  nh_.param("yaw_tracker/yaw_rate", yaw_rate_, -1.0);
  nh_.param("yaw_tracker/yaw_gain", yaw_gain_, -1.0);

  nh_.param("tracker_loop_rate", tracker_loop_rate_, -1);

  if (horizontal_speed_ < 0) {
    ROS_ERROR("[LineTracker]: horizontal_speed was not specified!");
    ros::shutdown();
  }

  if (vertical_speed_ < 0) {
    ROS_ERROR("[LineTracker]: vertical_speed was not specified!");
    ros::shutdown();
  }

  if (horizontal_acceleration_ < 0) {
    ROS_ERROR("[LineTracker]: horizontal_acceleration was not specified!");
    ros::shutdown();
  }

  if (vertical_acceleration_ < 0) {
    ROS_ERROR("[LineTracker]: vertical_acceleration was not specified!");
    ros::shutdown();
  }

  if (yaw_rate_ < 0) {
    ROS_ERROR("[LineTracker]: yaw_rate was not specified!");
    ros::shutdown();
  }

  if (yaw_gain_ < 0) {
    ROS_ERROR("[LineTracker]: yaw_gain was not specified!");
    ros::shutdown();
  }

  if (tracker_loop_rate_ < 0) {
    ROS_ERROR("[LineTracker]: tracker_loop_rate was not specified!");
    ros::shutdown();
  }

  tracker_dt_ = 1.0 / double(tracker_loop_rate_);

  ROS_INFO("[LineTracker]: tracker_dt: %f", tracker_dt_);

  state_x   = 0;
  state_y   = 0;
  state_z   = 0;
  state_yaw = 0;

  speed_x   = 0;
  speed_y   = 0;
  speed_yaw = 0;

  current_horizontal_speed = 0;
  current_vertical_speed   = 0;

  current_vertical_direction = 0;

  is_initialized = true;

  current_state_vertical  = IDLE_STATE;
  previous_state_vertical = IDLE_STATE;

  current_state_horizontal  = IDLE_STATE;
  previous_state_horizontal = IDLE_STATE;

  // --------------------------------------------------------------
  // |                           timers                           |
  // --------------------------------------------------------------

  main_timer = nh_.createTimer(ros::Rate(tracker_loop_rate_), &LineTracker::mainTimer, this);

  profiler = new mrs_lib::Profiler(nh_, "LineTracker");
  routine_main_timer = profiler->registerRoutine("main", tracker_loop_rate_, 0.002);

  ROS_INFO("[LineTracker]: initialized");
}

//}

//{ stopHorizontalMotion()

void LineTracker::stopHorizontalMotion(void) {

  current_horizontal_speed -= horizontal_acceleration_ * tracker_dt_;

  if (current_horizontal_speed < 0) {
    current_horizontal_speed = 0;
  }
}

//}

//{ stopVerticalMotion()

void LineTracker::stopVerticalMotion(void) {

  current_vertical_speed -= vertical_acceleration_ * tracker_dt_;

  if (current_vertical_speed < 0) {
    current_vertical_speed = 0;
  }
}

//}

//{ accelerateHorizontal()

void LineTracker::accelerateHorizontal(void) {

  current_heading = atan2(goal_y - state_y, goal_x - state_x);

  // decelerationg condition
  // calculate the time to stop and the distance it will take to stop [horizontal]
  double horizontal_t_stop    = current_horizontal_speed / horizontal_acceleration_;
  double horizontal_stop_dist = (horizontal_t_stop * current_horizontal_speed) / 2;
  double stop_dist_x          = cos(current_heading) * horizontal_stop_dist;
  double stop_dist_y          = sin(current_heading) * horizontal_stop_dist;

  current_horizontal_speed += horizontal_acceleration_ * tracker_dt_;

  if (current_horizontal_speed >= horizontal_speed_) {
    current_horizontal_speed = horizontal_speed_;
  }

  if (sqrt(pow(state_x + stop_dist_x - goal_x, 2) + pow(state_y + stop_dist_y - goal_y, 2)) < (2 * (horizontal_speed_ * tracker_dt_))) {
    changeStateHorizontal(DECELERATING_STATE);
  }
}

//}

//{ accelerateVertical()

void LineTracker::accelerateVertical(void) {

  // set the right heading
  double tar_z = goal_z - state_z;

  // set the right vertical direction
  current_vertical_direction = sign(tar_z);

  // calculate the time to stop and the distance it will take to stop [vertical]
  double vertical_t_stop    = current_vertical_speed / vertical_acceleration_;
  double vertical_stop_dist = (vertical_t_stop * current_vertical_speed) / 2;
  double stop_dist_z        = current_vertical_direction * vertical_stop_dist;

  current_vertical_speed += vertical_acceleration_ * tracker_dt_;

  if (current_vertical_speed >= vertical_speed_) {
    current_vertical_speed = vertical_speed_;
  }

  if (fabs(state_z + stop_dist_z - goal_z) < (2 * (vertical_speed_ * tracker_dt_))) {
    changeStateVertical(DECELERATING_STATE);
  }
}

//}

//{ decelerateHorizontal()

void LineTracker::decelerateHorizontal(void) {

  current_horizontal_speed -= horizontal_acceleration_ * tracker_dt_;

  if (current_horizontal_speed < 0) {
    current_horizontal_speed = 0;
  }

  if (current_horizontal_speed == 0) {
    changeStateHorizontal(STOPPING_STATE);
  }
}

//}

//{ decelerateVertical()

void LineTracker::decelerateVertical(void) {

  current_vertical_speed -= vertical_acceleration_ * tracker_dt_;

  if (current_vertical_speed < 0) {
    current_vertical_speed = 0;
  }

  if (current_vertical_speed == 0) {
    changeStateVertical(STOPPING_STATE);
  }
}

//}

//{ stopHorizontal()

void LineTracker::stopHorizontal(void) {

  state_x = 0.95 * state_x + 0.05 * goal_x;
  state_y = 0.95 * state_y + 0.05 * goal_y;
}

//}

//{ stopVertical()

void LineTracker::stopVertical(void) {

  state_z = 0.95 * state_z + 0.05 * goal_z;
}

//}

//{ mainTimer()

void LineTracker::mainTimer(const ros::TimerEvent &event) {

  if (!is_active) {
    return; 
  }

  routine_main_timer->start(event);

  switch (current_state_horizontal) {

    case IDLE_STATE:

      break;

    case HOVER_STATE:

      break;

    case STOP_MOTION_STATE:

      stopHorizontalMotion();

      break;

    case ACCELERATING_STATE:

      accelerateHorizontal();

      break;

    case DECELERATING_STATE:

      decelerateHorizontal();

      break;

    case STOPPING_STATE:

      stopHorizontal();

      break;
  }

  switch (current_state_vertical) {

    case IDLE_STATE:

      break;

    case HOVER_STATE:

      break;

    case STOP_MOTION_STATE:

      stopVerticalMotion();

      break;

    case ACCELERATING_STATE:

      accelerateVertical();

      break;

    case DECELERATING_STATE:

      decelerateVertical();

      break;

    case STOPPING_STATE:

      stopVertical();

      break;
  }

  if (current_state_horizontal == STOP_MOTION_STATE && current_state_vertical == STOP_MOTION_STATE) {

    if (current_vertical_speed == 0 && current_horizontal_speed == 0) {
      if (have_goal) {
        changeState(ACCELERATING_STATE);
      } else {
        changeState(STOPPING_STATE);
      }
    }
  }

  if (current_state_horizontal == STOPPING_STATE && current_state_vertical == STOPPING_STATE) {

    if (fabs(state_x - goal_x) < 1e-3 && fabs(state_y - goal_y) < 1e-3 && fabs(state_z - goal_z) < 1e-3) {

      state_x = goal_x;
      state_y = goal_y;
      state_z = goal_z;

      changeState(HOVER_STATE);
    }
  }

  state_x += cos(current_heading) * current_horizontal_speed * tracker_dt_;
  state_y += sin(current_heading) * current_horizontal_speed * tracker_dt_;
  state_z += current_vertical_direction * current_vertical_speed * tracker_dt_;

  // --------------------------------------------------------------
  // |                        yaw tracking                        |
  // --------------------------------------------------------------

  // compute the desired yaw rate
  double current_yaw_rate;
  if (fabs(goal_yaw - state_yaw) > PI)
    current_yaw_rate = -yaw_gain_ * (goal_yaw - state_yaw);
  else
    current_yaw_rate = yaw_gain_ * (goal_yaw - state_yaw);

  if (current_yaw_rate > yaw_rate_) {
    current_yaw_rate = yaw_rate_;
  } else if (current_yaw_rate < -yaw_rate_) {
    current_yaw_rate = -yaw_rate_;
  }

  // flap the resulted state_yaw aroud PI
  state_yaw += current_yaw_rate * tracker_dt_;

  if (state_yaw > PI) {
    state_yaw -= 2 * PI;
  } else if (state_yaw < -PI) {
    state_yaw += 2 * PI;
  }

  if (fabs(state_yaw - goal_yaw) < (2 * (yaw_rate_ * tracker_dt_))) {
    state_yaw = goal_yaw;
  }

  routine_main_timer->end();
}

//}

//{ activate()

bool LineTracker::activate(const mrs_msgs::PositionCommand::ConstPtr &cmd) {

  if (!got_odometry) {
    ROS_ERROR("[LineTracker]: can't activate(), odometry not set");
    return false;
  }

  mutex_odometry.lock();
  {
    if (mrs_msgs::PositionCommand::Ptr() != cmd) {

      // the last command is usable
      state_x   = odometry.pose.pose.position.x;
      state_y   = odometry.pose.pose.position.y;
      state_z   = odometry.pose.pose.position.z;
      state_yaw = cmd->yaw;

      speed_x         = cmd->velocity.x;
      speed_y         = cmd->velocity.y;
      current_heading = atan2(speed_y, speed_x);

      current_horizontal_speed = sqrt(pow(speed_x, 2) + pow(speed_y, 2));
      current_vertical_speed   = cmd->velocity.z;

      goal_yaw = cmd->yaw;

      ROS_INFO("[LineTracker]: activated with setpoint x: %2.2f, y: %2.2f, z: %2.2f, yaw: %2.2f", cmd->position.x, cmd->position.y, cmd->position.z, cmd->yaw);

    } else {

      ROS_WARN("[LineTracker]: activated, the previous command is not usable for activation, using Odometry instead.");

      state_x   = odometry.pose.pose.position.x;
      state_y   = odometry.pose.pose.position.y;
      state_z   = odometry.pose.pose.position.z;
      state_yaw = odometry_yaw;

      speed_x                  = odometry.twist.twist.linear.x;
      speed_y                  = odometry.twist.twist.linear.y;
      current_heading          = atan2(speed_y, speed_x);
      current_horizontal_speed = sqrt(pow(speed_x, 2) + pow(speed_y, 2));

      current_vertical_speed = odometry.twist.twist.linear.z;

      goal_yaw = odometry_yaw;

      ROS_INFO("[LineTracker]: state_x = %f, state_y = %f, state_z = %f", state_x, state_y, state_z);
      ROS_INFO("[LineTracker]: speed_x = %f, speed_y = %f, speed_z = %f", speed_x, speed_y, current_vertical_speed);
    }
  }
  mutex_odometry.unlock();

  // --------------------------------------------------------------
  // |          horizontal initial conditions prediction          |
  // --------------------------------------------------------------

  double horizontal_t_stop    = current_horizontal_speed / horizontal_acceleration_;
  double horizontal_stop_dist = (horizontal_t_stop * current_horizontal_speed) / 2;
  double stop_dist_x          = cos(current_heading) * horizontal_stop_dist;
  double stop_dist_y          = sin(current_heading) * horizontal_stop_dist;

  // --------------------------------------------------------------
  // |           vertical initial conditions prediction           |
  // --------------------------------------------------------------

  double vertical_t_stop    = current_vertical_speed / vertical_acceleration_;
  double vertical_stop_dist = (vertical_t_stop * current_vertical_speed) / 2;

  // --------------------------------------------------------------
  // |              yaw initial condition  prediction             |
  // --------------------------------------------------------------

  goal_x = state_x + stop_dist_x;
  goal_y = state_y + stop_dist_y;
  goal_z = state_z + vertical_stop_dist;

  is_active = true;

  ROS_INFO("[LineTracker]: activated");

  changeState(STOP_MOTION_STATE);

  return true;
}

//}

//{ deactivate()

void LineTracker::deactivate(void) {

  is_active = false;

  ROS_INFO("[LineTracker]: deactivated");
}

//}

//{ update()

const mrs_msgs::PositionCommand::ConstPtr LineTracker::update(const nav_msgs::Odometry::ConstPtr &msg) {

  mutex_odometry.lock();
  {
    odometry   = *msg;
    odometry_x = odometry.pose.pose.position.x;
    odometry_y = odometry.pose.pose.position.y;
    odometry_z = odometry.pose.pose.position.z;

    // calculate the euler angles
    tf::Quaternion quaternion_odometry;
    quaternionMsgToTF(odometry.pose.pose.orientation, quaternion_odometry);
    tf::Matrix3x3 m(quaternion_odometry);
    m.getRPY(odometry_roll, odometry_pitch, odometry_yaw);

    got_odometry = true;
  }
  mutex_odometry.unlock();

  if (!is_active) {

    return mrs_msgs::PositionCommand::Ptr();
  }

  position_output.header.stamp    = ros::Time::now();
  position_output.header.frame_id = "local_origin";

  position_output.position.x = state_x;
  position_output.position.y = state_y;
  position_output.position.z = state_z;
  position_output.yaw        = state_yaw;

  position_output.velocity.x = cos(current_heading) * current_horizontal_speed;
  position_output.velocity.y = sin(current_heading) * current_horizontal_speed;
  position_output.velocity.z = current_vertical_direction * current_vertical_speed;
  position_output.yaw_dot    = speed_yaw;

  position_output.acceleration.x = 0;
  position_output.acceleration.y = 0;
  position_output.acceleration.z = 0;

  return mrs_msgs::PositionCommand::ConstPtr(new mrs_msgs::PositionCommand(position_output));
}

//}

//{ status()

const mrs_msgs::TrackerStatus::Ptr LineTracker::status() {

  if (is_initialized) {

    mrs_msgs::TrackerStatus::Ptr tracker_status(new mrs_msgs::TrackerStatus);

    if (is_active) {
      tracker_status->active = mrs_msgs::TrackerStatus::ACTIVE;
    } else {
      tracker_status->active = mrs_msgs::TrackerStatus::NONACTIVE;
    }

    return tracker_status;
  } else {

    return mrs_msgs::TrackerStatus::Ptr();
  }
}

//}

//{ goTo()

const mrs_msgs::Vec4Response::ConstPtr LineTracker::goTo(const mrs_msgs::Vec4Request::ConstPtr &cmd) {

  mrs_msgs::Vec4Response res;

  goal_x   = cmd->goal[0];
  goal_y   = cmd->goal[1];
  goal_z   = cmd->goal[2];
  goal_yaw = validateYawSetpoint(cmd->goal[3]);

  ROS_INFO("[LineTracker]: received new setpoint %f, %f, %f, %f", goal_x, goal_y, goal_z, goal_yaw);

  have_goal = true;

  res.success = true;
  res.message = "setpoint set";

  changeState(STOP_MOTION_STATE);

  return mrs_msgs::Vec4Response::ConstPtr(new mrs_msgs::Vec4Response(res));
}

//}

//{ goToRelative()

const mrs_msgs::Vec4Response::ConstPtr LineTracker::goToRelative(const mrs_msgs::Vec4Request::ConstPtr &cmd) {

  mrs_msgs::Vec4Response res;

  mutex_odometry.lock();
  {
    goal_x   = state_x + cmd->goal[0];
    goal_y   = state_y + cmd->goal[1];
    goal_z   = state_z + cmd->goal[2];
    goal_yaw = validateYawSetpoint(state_yaw + cmd->goal[3]);
  }
  mutex_odometry.unlock();

  ROS_INFO("[LineTracker]: received new relative setpoint, flying to %f, %f, %f, %f", goal_x, goal_y, goal_z, goal_yaw);

  have_goal = true;

  res.success = true;
  res.message = "setpoint set";

  changeState(STOP_MOTION_STATE);

  return mrs_msgs::Vec4Response::ConstPtr(new mrs_msgs::Vec4Response(res));
}

//}

//{ hover()

const std_srvs::TriggerResponse::ConstPtr LineTracker::hover(const std_srvs::TriggerRequest::ConstPtr &cmd) {

  std_srvs::TriggerResponse res;

  // --------------------------------------------------------------
  // |          horizontal initial conditions prediction          |
  // --------------------------------------------------------------
  mutex_odometry.lock();
  {
    current_horizontal_speed = sqrt(pow(odometry.twist.twist.linear.x, 2) + pow(odometry.twist.twist.linear.y, 2));
    current_vertical_speed   = odometry.twist.twist.linear.z;
  }
  mutex_odometry.unlock();

  double horizontal_t_stop    = current_horizontal_speed / horizontal_acceleration_;
  double horizontal_stop_dist = (horizontal_t_stop * current_horizontal_speed) / 2;
  double stop_dist_x          = cos(current_heading) * horizontal_stop_dist;
  double stop_dist_y          = sin(current_heading) * horizontal_stop_dist;

  // --------------------------------------------------------------
  // |           vertical initial conditions prediction           |
  // --------------------------------------------------------------

  double vertical_t_stop    = current_vertical_speed / vertical_acceleration_;
  double vertical_stop_dist = (vertical_t_stop * current_vertical_speed) / 2;

  // --------------------------------------------------------------
  // |                        set the goal                        |
  // --------------------------------------------------------------

  goal_x = state_x + stop_dist_x;
  goal_y = state_y + stop_dist_y;
  goal_z = state_z + vertical_stop_dist;

  res.message = "Hover initiated.";
  res.success = true;

  return std_srvs::TriggerResponse::ConstPtr(new std_srvs::TriggerResponse(res));
}

//}
}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(mrs_trackers::LineTracker, mrs_mav_manager::Tracker)
