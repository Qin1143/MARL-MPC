// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from traj_interfaces:msg/Bspline.idl
// generated code does not contain a copyright notice

#ifndef TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__FUNCTIONS_H_
#define TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "traj_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "traj_interfaces/msg/detail/bspline__struct.h"

/// Initialize msg/Bspline message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * traj_interfaces__msg__Bspline
 * )) before or use
 * traj_interfaces__msg__Bspline__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
bool
traj_interfaces__msg__Bspline__init(traj_interfaces__msg__Bspline * msg);

/// Finalize msg/Bspline message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
void
traj_interfaces__msg__Bspline__fini(traj_interfaces__msg__Bspline * msg);

/// Create msg/Bspline message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * traj_interfaces__msg__Bspline__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
traj_interfaces__msg__Bspline *
traj_interfaces__msg__Bspline__create();

/// Destroy msg/Bspline message.
/**
 * It calls
 * traj_interfaces__msg__Bspline__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
void
traj_interfaces__msg__Bspline__destroy(traj_interfaces__msg__Bspline * msg);

/// Check for msg/Bspline message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
bool
traj_interfaces__msg__Bspline__are_equal(const traj_interfaces__msg__Bspline * lhs, const traj_interfaces__msg__Bspline * rhs);

/// Copy a msg/Bspline message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
bool
traj_interfaces__msg__Bspline__copy(
  const traj_interfaces__msg__Bspline * input,
  traj_interfaces__msg__Bspline * output);

/// Initialize array of msg/Bspline messages.
/**
 * It allocates the memory for the number of elements and calls
 * traj_interfaces__msg__Bspline__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
bool
traj_interfaces__msg__Bspline__Sequence__init(traj_interfaces__msg__Bspline__Sequence * array, size_t size);

/// Finalize array of msg/Bspline messages.
/**
 * It calls
 * traj_interfaces__msg__Bspline__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
void
traj_interfaces__msg__Bspline__Sequence__fini(traj_interfaces__msg__Bspline__Sequence * array);

/// Create array of msg/Bspline messages.
/**
 * It allocates the memory for the array and calls
 * traj_interfaces__msg__Bspline__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
traj_interfaces__msg__Bspline__Sequence *
traj_interfaces__msg__Bspline__Sequence__create(size_t size);

/// Destroy array of msg/Bspline messages.
/**
 * It calls
 * traj_interfaces__msg__Bspline__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
void
traj_interfaces__msg__Bspline__Sequence__destroy(traj_interfaces__msg__Bspline__Sequence * array);

/// Check for msg/Bspline message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
bool
traj_interfaces__msg__Bspline__Sequence__are_equal(const traj_interfaces__msg__Bspline__Sequence * lhs, const traj_interfaces__msg__Bspline__Sequence * rhs);

/// Copy an array of msg/Bspline messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_traj_interfaces
bool
traj_interfaces__msg__Bspline__Sequence__copy(
  const traj_interfaces__msg__Bspline__Sequence * input,
  traj_interfaces__msg__Bspline__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // TRAJ_INTERFACES__MSG__DETAIL__BSPLINE__FUNCTIONS_H_
