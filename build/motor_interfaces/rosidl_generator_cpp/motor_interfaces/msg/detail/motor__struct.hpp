// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from motor_interfaces:msg/Motor.idl
// generated code does not contain a copyright notice

#ifndef MOTOR_INTERFACES__MSG__DETAIL__MOTOR__STRUCT_HPP_
#define MOTOR_INTERFACES__MSG__DETAIL__MOTOR__STRUCT_HPP_

#include <rosidl_runtime_cpp/bounded_vector.hpp>
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>


#ifndef _WIN32
# define DEPRECATED__motor_interfaces__msg__Motor __attribute__((deprecated))
#else
# define DEPRECATED__motor_interfaces__msg__Motor __declspec(deprecated)
#endif

namespace motor_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Motor_
{
  using Type = Motor_<ContainerAllocator>;

  explicit Motor_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->left_speed = 0.0f;
      this->right_speed = 0.0f;
      this->left_torque = 0.0f;
      this->right_torque = 0.0f;
    }
  }

  explicit Motor_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->left_speed = 0.0f;
      this->right_speed = 0.0f;
      this->left_torque = 0.0f;
      this->right_torque = 0.0f;
    }
  }

  // field types and members
  using _left_speed_type =
    float;
  _left_speed_type left_speed;
  using _right_speed_type =
    float;
  _right_speed_type right_speed;
  using _left_torque_type =
    float;
  _left_torque_type left_torque;
  using _right_torque_type =
    float;
  _right_torque_type right_torque;

  // setters for named parameter idiom
  Type & set__left_speed(
    const float & _arg)
  {
    this->left_speed = _arg;
    return *this;
  }
  Type & set__right_speed(
    const float & _arg)
  {
    this->right_speed = _arg;
    return *this;
  }
  Type & set__left_torque(
    const float & _arg)
  {
    this->left_torque = _arg;
    return *this;
  }
  Type & set__right_torque(
    const float & _arg)
  {
    this->right_torque = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    motor_interfaces::msg::Motor_<ContainerAllocator> *;
  using ConstRawPtr =
    const motor_interfaces::msg::Motor_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<motor_interfaces::msg::Motor_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<motor_interfaces::msg::Motor_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      motor_interfaces::msg::Motor_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<motor_interfaces::msg::Motor_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      motor_interfaces::msg::Motor_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<motor_interfaces::msg::Motor_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<motor_interfaces::msg::Motor_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<motor_interfaces::msg::Motor_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__motor_interfaces__msg__Motor
    std::shared_ptr<motor_interfaces::msg::Motor_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__motor_interfaces__msg__Motor
    std::shared_ptr<motor_interfaces::msg::Motor_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Motor_ & other) const
  {
    if (this->left_speed != other.left_speed) {
      return false;
    }
    if (this->right_speed != other.right_speed) {
      return false;
    }
    if (this->left_torque != other.left_torque) {
      return false;
    }
    if (this->right_torque != other.right_torque) {
      return false;
    }
    return true;
  }
  bool operator!=(const Motor_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Motor_

// alias to use template instance with default allocator
using Motor =
  motor_interfaces::msg::Motor_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace motor_interfaces

#endif  // MOTOR_INTERFACES__MSG__DETAIL__MOTOR__STRUCT_HPP_
