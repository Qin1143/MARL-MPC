// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from action_interfaces:msg/Action.idl
// generated code does not contain a copyright notice

#ifndef ACTION_INTERFACES__MSG__DETAIL__ACTION__TRAITS_HPP_
#define ACTION_INTERFACES__MSG__DETAIL__ACTION__TRAITS_HPP_

#include "action_interfaces/msg/detail/action__struct.hpp"
#include <rosidl_runtime_cpp/traits.hpp>
#include <stdint.h>
#include <type_traits>

namespace rosidl_generator_traits
{

template<>
inline const char * data_type<action_interfaces::msg::Action>()
{
  return "action_interfaces::msg::Action";
}

template<>
inline const char * name<action_interfaces::msg::Action>()
{
  return "action_interfaces/msg/Action";
}

template<>
struct has_fixed_size<action_interfaces::msg::Action>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<action_interfaces::msg::Action>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<action_interfaces::msg::Action>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // ACTION_INTERFACES__MSG__DETAIL__ACTION__TRAITS_HPP_
