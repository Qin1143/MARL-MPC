// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from motor_interfaces:msg/Motor.idl
// generated code does not contain a copyright notice

#ifndef MOTOR_INTERFACES__MSG__DETAIL__MOTOR__BUILDER_HPP_
#define MOTOR_INTERFACES__MSG__DETAIL__MOTOR__BUILDER_HPP_

#include "motor_interfaces/msg/detail/motor__struct.hpp"
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <utility>


namespace motor_interfaces
{

namespace msg
{

namespace builder
{

class Init_Motor_right_torque
{
public:
  explicit Init_Motor_right_torque(::motor_interfaces::msg::Motor & msg)
  : msg_(msg)
  {}
  ::motor_interfaces::msg::Motor right_torque(::motor_interfaces::msg::Motor::_right_torque_type arg)
  {
    msg_.right_torque = std::move(arg);
    return std::move(msg_);
  }

private:
  ::motor_interfaces::msg::Motor msg_;
};

class Init_Motor_left_torque
{
public:
  explicit Init_Motor_left_torque(::motor_interfaces::msg::Motor & msg)
  : msg_(msg)
  {}
  Init_Motor_right_torque left_torque(::motor_interfaces::msg::Motor::_left_torque_type arg)
  {
    msg_.left_torque = std::move(arg);
    return Init_Motor_right_torque(msg_);
  }

private:
  ::motor_interfaces::msg::Motor msg_;
};

class Init_Motor_right_speed
{
public:
  explicit Init_Motor_right_speed(::motor_interfaces::msg::Motor & msg)
  : msg_(msg)
  {}
  Init_Motor_left_torque right_speed(::motor_interfaces::msg::Motor::_right_speed_type arg)
  {
    msg_.right_speed = std::move(arg);
    return Init_Motor_left_torque(msg_);
  }

private:
  ::motor_interfaces::msg::Motor msg_;
};

class Init_Motor_left_speed
{
public:
  Init_Motor_left_speed()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Motor_right_speed left_speed(::motor_interfaces::msg::Motor::_left_speed_type arg)
  {
    msg_.left_speed = std::move(arg);
    return Init_Motor_right_speed(msg_);
  }

private:
  ::motor_interfaces::msg::Motor msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::motor_interfaces::msg::Motor>()
{
  return motor_interfaces::msg::builder::Init_Motor_left_speed();
}

}  // namespace motor_interfaces

#endif  // MOTOR_INTERFACES__MSG__DETAIL__MOTOR__BUILDER_HPP_
