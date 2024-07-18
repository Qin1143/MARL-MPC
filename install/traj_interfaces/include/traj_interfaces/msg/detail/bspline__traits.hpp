// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from traj_interfaces:msg/Bspline.idl
// generated code does not contain a copyright notice

#ifndef TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__TRAITS_HPP_
#define TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__TRAITS_HPP_

#include "traj_interfaces/msg/detail/bspline__struct.hpp"
#include <rosidl_runtime_cpp/traits.hpp>
#include <stdint.h>
#include <type_traits>

// Include directives for member types
// Member 'start_time'
#include "builtin_interfaces/msg/detail/time__traits.hpp"

namespace rosidl_generator_traits
{

template<>
inline const char * data_type<traj_interfaces::msg::Bspline>()
{
  return "traj_interfaces::msg::Bspline";
}

template<>
inline const char * name<traj_interfaces::msg::Bspline>()
{
  return "traj_interfaces/msg/Bspline";
}

template<>
struct has_fixed_size<traj_interfaces::msg::Bspline>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<traj_interfaces::msg::Bspline>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<traj_interfaces::msg::Bspline>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__TRAITS_HPP_
