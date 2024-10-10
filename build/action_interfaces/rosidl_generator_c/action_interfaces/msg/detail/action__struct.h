// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from action_interfaces:msg/Action.idl
// generated code does not contain a copyright notice

#ifndef ACTION_INTERFACES__MSG__DETAIL__ACTION__STRUCT_H_
#define ACTION_INTERFACES__MSG__DETAIL__ACTION__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'action'
#include "rosidl_runtime_c/primitives_sequence.h"

// Struct defined in msg/Action in the package action_interfaces.
typedef struct action_interfaces__msg__Action
{
  rosidl_runtime_c__double__Sequence action;
} action_interfaces__msg__Action;

// Struct for a sequence of action_interfaces__msg__Action.
typedef struct action_interfaces__msg__Action__Sequence
{
  action_interfaces__msg__Action * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} action_interfaces__msg__Action__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ACTION_INTERFACES__MSG__DETAIL__ACTION__STRUCT_H_
