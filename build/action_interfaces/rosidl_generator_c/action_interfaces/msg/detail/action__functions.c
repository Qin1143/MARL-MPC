// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from action_interfaces:msg/Action.idl
// generated code does not contain a copyright notice
#include "action_interfaces/msg/detail/action__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `action`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

bool
action_interfaces__msg__Action__init(action_interfaces__msg__Action * msg)
{
  if (!msg) {
    return false;
  }
  // action
  if (!rosidl_runtime_c__double__Sequence__init(&msg->action, 0)) {
    action_interfaces__msg__Action__fini(msg);
    return false;
  }
  return true;
}

void
action_interfaces__msg__Action__fini(action_interfaces__msg__Action * msg)
{
  if (!msg) {
    return;
  }
  // action
  rosidl_runtime_c__double__Sequence__fini(&msg->action);
}

bool
action_interfaces__msg__Action__are_equal(const action_interfaces__msg__Action * lhs, const action_interfaces__msg__Action * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // action
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->action), &(rhs->action)))
  {
    return false;
  }
  return true;
}

bool
action_interfaces__msg__Action__copy(
  const action_interfaces__msg__Action * input,
  action_interfaces__msg__Action * output)
{
  if (!input || !output) {
    return false;
  }
  // action
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->action), &(output->action)))
  {
    return false;
  }
  return true;
}

action_interfaces__msg__Action *
action_interfaces__msg__Action__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  action_interfaces__msg__Action * msg = (action_interfaces__msg__Action *)allocator.allocate(sizeof(action_interfaces__msg__Action), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(action_interfaces__msg__Action));
  bool success = action_interfaces__msg__Action__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
action_interfaces__msg__Action__destroy(action_interfaces__msg__Action * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    action_interfaces__msg__Action__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
action_interfaces__msg__Action__Sequence__init(action_interfaces__msg__Action__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  action_interfaces__msg__Action * data = NULL;

  if (size) {
    data = (action_interfaces__msg__Action *)allocator.zero_allocate(size, sizeof(action_interfaces__msg__Action), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = action_interfaces__msg__Action__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        action_interfaces__msg__Action__fini(&data[i - 1]);
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
action_interfaces__msg__Action__Sequence__fini(action_interfaces__msg__Action__Sequence * array)
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
      action_interfaces__msg__Action__fini(&array->data[i]);
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

action_interfaces__msg__Action__Sequence *
action_interfaces__msg__Action__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  action_interfaces__msg__Action__Sequence * array = (action_interfaces__msg__Action__Sequence *)allocator.allocate(sizeof(action_interfaces__msg__Action__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = action_interfaces__msg__Action__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
action_interfaces__msg__Action__Sequence__destroy(action_interfaces__msg__Action__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    action_interfaces__msg__Action__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
action_interfaces__msg__Action__Sequence__are_equal(const action_interfaces__msg__Action__Sequence * lhs, const action_interfaces__msg__Action__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!action_interfaces__msg__Action__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
action_interfaces__msg__Action__Sequence__copy(
  const action_interfaces__msg__Action__Sequence * input,
  action_interfaces__msg__Action__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(action_interfaces__msg__Action);
    action_interfaces__msg__Action * data =
      (action_interfaces__msg__Action *)realloc(output->data, allocation_size);
    if (!data) {
      return false;
    }
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!action_interfaces__msg__Action__init(&data[i])) {
        /* free currently allocated and return false */
        for (; i-- > output->capacity; ) {
          action_interfaces__msg__Action__fini(&data[i]);
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
    if (!action_interfaces__msg__Action__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}