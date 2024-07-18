// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from motor_interfaces:msg/Motor.idl
// generated code does not contain a copyright notice
#include "motor_interfaces/msg/detail/motor__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


bool
motor_interfaces__msg__Motor__init(motor_interfaces__msg__Motor * msg)
{
  if (!msg) {
    return false;
  }
  // left_speed
  // right_speed
  // left_torque
  // right_torque
  return true;
}

void
motor_interfaces__msg__Motor__fini(motor_interfaces__msg__Motor * msg)
{
  if (!msg) {
    return;
  }
  // left_speed
  // right_speed
  // left_torque
  // right_torque
}

bool
motor_interfaces__msg__Motor__are_equal(const motor_interfaces__msg__Motor * lhs, const motor_interfaces__msg__Motor * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // left_speed
  if (lhs->left_speed != rhs->left_speed) {
    return false;
  }
  // right_speed
  if (lhs->right_speed != rhs->right_speed) {
    return false;
  }
  // left_torque
  if (lhs->left_torque != rhs->left_torque) {
    return false;
  }
  // right_torque
  if (lhs->right_torque != rhs->right_torque) {
    return false;
  }
  return true;
}

bool
motor_interfaces__msg__Motor__copy(
  const motor_interfaces__msg__Motor * input,
  motor_interfaces__msg__Motor * output)
{
  if (!input || !output) {
    return false;
  }
  // left_speed
  output->left_speed = input->left_speed;
  // right_speed
  output->right_speed = input->right_speed;
  // left_torque
  output->left_torque = input->left_torque;
  // right_torque
  output->right_torque = input->right_torque;
  return true;
}

motor_interfaces__msg__Motor *
motor_interfaces__msg__Motor__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  motor_interfaces__msg__Motor * msg = (motor_interfaces__msg__Motor *)allocator.allocate(sizeof(motor_interfaces__msg__Motor), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(motor_interfaces__msg__Motor));
  bool success = motor_interfaces__msg__Motor__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
motor_interfaces__msg__Motor__destroy(motor_interfaces__msg__Motor * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    motor_interfaces__msg__Motor__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
motor_interfaces__msg__Motor__Sequence__init(motor_interfaces__msg__Motor__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  motor_interfaces__msg__Motor * data = NULL;

  if (size) {
    data = (motor_interfaces__msg__Motor *)allocator.zero_allocate(size, sizeof(motor_interfaces__msg__Motor), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = motor_interfaces__msg__Motor__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        motor_interfaces__msg__Motor__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
motor_interfaces__msg__Motor__Sequence__fini(motor_interfaces__msg__Motor__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      motor_interfaces__msg__Motor__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

motor_interfaces__msg__Motor__Sequence *
motor_interfaces__msg__Motor__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  motor_interfaces__msg__Motor__Sequence * array = (motor_interfaces__msg__Motor__Sequence *)allocator.allocate(sizeof(motor_interfaces__msg__Motor__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = motor_interfaces__msg__Motor__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
motor_interfaces__msg__Motor__Sequence__destroy(motor_interfaces__msg__Motor__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    motor_interfaces__msg__Motor__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
motor_interfaces__msg__Motor__Sequence__are_equal(const motor_interfaces__msg__Motor__Sequence * lhs, const motor_interfaces__msg__Motor__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!motor_interfaces__msg__Motor__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
motor_interfaces__msg__Motor__Sequence__copy(
  const motor_interfaces__msg__Motor__Sequence * input,
  motor_interfaces__msg__Motor__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(motor_interfaces__msg__Motor);
    motor_interfaces__msg__Motor * data =
      (motor_interfaces__msg__Motor *)realloc(output->data, allocation_size);
    if (!data) {
      return false;
    }
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!motor_interfaces__msg__Motor__init(&data[i])) {
        /* free currently allocated and return false */
        for (; i-- > output->capacity; ) {
          motor_interfaces__msg__Motor__fini(&data[i]);
        }
        free(data);
        return false;
      }
    }
    output->data = data;
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!motor_interfaces__msg__Motor__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
