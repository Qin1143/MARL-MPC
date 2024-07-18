// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from traj_interfaces:msg/Bspline.idl
// generated code does not contain a copyright notice

#ifndef TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__BUILDER_HPP_
#define TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__BUILDER_HPP_

#include "traj_interfaces/msg/detail/bspline__struct.hpp"
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <utility>


namespace traj_interfaces
{

namespace msg
{

namespace builder
{

class Init_Bspline_yaw_dt
{
public:
  explicit Init_Bspline_yaw_dt(::traj_interfaces::msg::Bspline & msg)
  : msg_(msg)
  {}
  ::traj_interfaces::msg::Bspline yaw_dt(::traj_interfaces::msg::Bspline::_yaw_dt_type arg)
  {
    msg_.yaw_dt = std::move(arg);
    return std::move(msg_);
  }

private:
  ::traj_interfaces::msg::Bspline msg_;
};

class Init_Bspline_yaw_pts
{
public:
  explicit Init_Bspline_yaw_pts(::traj_interfaces::msg::Bspline & msg)
  : msg_(msg)
  {}
  Init_Bspline_yaw_dt yaw_pts(::traj_interfaces::msg::Bspline::_yaw_pts_type arg)
  {
    msg_.yaw_pts = std::move(arg);
    return Init_Bspline_yaw_dt(msg_);
  }

private:
  ::traj_interfaces::msg::Bspline msg_;
};

class Init_Bspline_pos_pts
{
public:
  explicit Init_Bspline_pos_pts(::traj_interfaces::msg::Bspline & msg)
  : msg_(msg)
  {}
  Init_Bspline_yaw_pts pos_pts(::traj_interfaces::msg::Bspline::_pos_pts_type arg)
  {
    msg_.pos_pts = std::move(arg);
    return Init_Bspline_yaw_pts(msg_);
  }

private:
  ::traj_interfaces::msg::Bspline msg_;
};

class Init_Bspline_knots
{
public:
  explicit Init_Bspline_knots(::traj_interfaces::msg::Bspline & msg)
  : msg_(msg)
  {}
  Init_Bspline_pos_pts knots(::traj_interfaces::msg::Bspline::_knots_type arg)
  {
    msg_.knots = std::move(arg);
    return Init_Bspline_pos_pts(msg_);
  }

private:
  ::traj_interfaces::msg::Bspline msg_;
};

class Init_Bspline_start_time
{
public:
  explicit Init_Bspline_start_time(::traj_interfaces::msg::Bspline & msg)
  : msg_(msg)
  {}
  Init_Bspline_knots start_time(::traj_interfaces::msg::Bspline::_start_time_type arg)
  {
    msg_.start_time = std::move(arg);
    return Init_Bspline_knots(msg_);
  }

private:
  ::traj_interfaces::msg::Bspline msg_;
};

class Init_Bspline_traj_id
{
public:
  explicit Init_Bspline_traj_id(::traj_interfaces::msg::Bspline & msg)
  : msg_(msg)
  {}
  Init_Bspline_start_time traj_id(::traj_interfaces::msg::Bspline::_traj_id_type arg)
  {
    msg_.traj_id = std::move(arg);
    return Init_Bspline_start_time(msg_);
  }

private:
  ::traj_interfaces::msg::Bspline msg_;
};

class Init_Bspline_order
{
public:
  Init_Bspline_order()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Bspline_traj_id order(::traj_interfaces::msg::Bspline::_order_type arg)
  {
    msg_.order = std::move(arg);
    return Init_Bspline_traj_id(msg_);
  }

private:
  ::traj_interfaces::msg::Bspline msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::traj_interfaces::msg::Bspline>()
{
  return traj_interfaces::msg::builder::Init_Bspline_order();
}

}  // namespace traj_interfaces

#endif  // TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__BUILDER_HPP_
