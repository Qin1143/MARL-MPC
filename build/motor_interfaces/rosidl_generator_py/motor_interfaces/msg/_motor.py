# generated from rosidl_generator_py/resource/_idl.py.em
# with input from motor_interfaces:msg/Motor.idl
# generated code does not contain a copyright notice


# Import statements for member types

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_Motor(type):
    """Metaclass of message 'Motor'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('motor_interfaces')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'motor_interfaces.msg.Motor')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__motor
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__motor
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__motor
            cls._TYPE_SUPPORT = module.type_support_msg__msg__motor
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__motor

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class Motor(metaclass=Metaclass_Motor):
    """Message class 'Motor'."""

    __slots__ = [
        '_left_speed',
        '_right_speed',
        '_left_torque',
        '_right_torque',
    ]

    _fields_and_field_types = {
        'left_speed': 'float',
        'right_speed': 'float',
        'left_torque': 'float',
        'right_torque': 'float',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.left_speed = kwargs.get('left_speed', float())
        self.right_speed = kwargs.get('right_speed', float())
        self.left_torque = kwargs.get('left_torque', float())
        self.right_torque = kwargs.get('right_torque', float())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.left_speed != other.left_speed:
            return False
        if self.right_speed != other.right_speed:
            return False
        if self.left_torque != other.left_torque:
            return False
        if self.right_torque != other.right_torque:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @property
    def left_speed(self):
        """Message field 'left_speed'."""
        return self._left_speed

    @left_speed.setter
    def left_speed(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'left_speed' field must be of type 'float'"
        self._left_speed = value

    @property
    def right_speed(self):
        """Message field 'right_speed'."""
        return self._right_speed

    @right_speed.setter
    def right_speed(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'right_speed' field must be of type 'float'"
        self._right_speed = value

    @property
    def left_torque(self):
        """Message field 'left_torque'."""
        return self._left_torque

    @left_torque.setter
    def left_torque(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'left_torque' field must be of type 'float'"
        self._left_torque = value

    @property
    def right_torque(self):
        """Message field 'right_torque'."""
        return self._right_torque

    @right_torque.setter
    def right_torque(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'right_torque' field must be of type 'float'"
        self._right_torque = value
