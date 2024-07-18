// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from motor_interfaces:msg/Motor.idl
// generated code does not contain a copyright notice

#ifndef MOTOR_INTERFACES__MSG__DETAIL__MOTOR__STRUCT_H_
#define MOTOR_INTERFACES__MSG__DETAIL__MOTOR__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Struct defined in msg/Motor in the package motor_interfaces.
typedef struct motor_interfaces__msg__Motor
{
  float left_speed;
  float right_speed;
  float left_torque;
  float right_torque;
} motor_interfaces__msg__Motor;

// Struct for a sequence of motor_interfaces__msg__Motor.
typedef struct motor_interfaces__msg__Motor__Sequence
{
  motor_interfaces__msg__Motor * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} motor_interfaces__msg__Motor__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MOTOR_INTERFACES__MSG__DETAIL__MOTOR__STRUCT_H_
