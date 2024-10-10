// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from action_interfaces:msg/Action.idl
// generated code does not contain a copyright notice

#ifndef ACTION_INTERFACES__MSG__DETAIL__ACTION__BUILDER_HPP_
#define ACTION_INTERFACES__MSG__DETAIL__ACTION__BUILDER_HPP_

#include "action_interfaces/msg/detail/action__struct.hpp"
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <utility>


namespace action_interfaces
{

namespace msg
{

namespace builder
{

class Init_Action_action
{
public:
  Init_Action_action()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::action_interfaces::msg::Action action(::action_interfaces::msg::Action::_action_type arg)
  {
    msg_.action = std::move(arg);
    return std::move(msg_);
  }

private:
  ::action_interfaces::msg::Action msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::action_interfaces::msg::Action>()
{
  return action_interfaces::msg::builder::Init_Action_action();
}

}  // namespace action_interfaces

#endif  // ACTION_INTERFACES__MSG__DETAIL__ACTION__BUILDER_HPP_
