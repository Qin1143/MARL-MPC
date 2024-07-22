import rclpy
from geometry_msgs.msg import Twist
from motor_interfaces.msg import Motor
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Quaternion, TransformStamped
from tf2_ros import TransformBroadcaster
import tf_transformations

HALF_DISTANCE_BETWEEN_WHEELS = 0.045
WHEEL_RADIUS = 0.025

class MyRobotDriver:
    def init(self, webots_node, properties):
        self.__robot = webots_node.robot

        self.__left_motor = self.__robot.getDevice('middle_left_wheel_joint')
        self.__right_motor = self.__robot.getDevice('middle_right_wheel_joint')

        self.__left_motor.setPosition(float('inf'))
        self.__left_motor.setVelocity(0)

        self.__right_motor.setPosition(float('inf'))
        self.__right_motor.setVelocity(0)

        self.__inertial_unit = self.__robot.getDevice('inertial unit') # x_angle, y_angle, z_angle
        self.__gyro = self.__robot.getDevice('imu_data gyro') # xAxis_velocity, yAxis_velocity, zAxis_velocity
        self.__gps = self.__robot.getDevice('gps') # x, y, v

        self.__target_twist = Twist()

        self.__command_motor_left = 0
        self.__command_motor_right = 0

        rclpy.init(args=None)
        self.__node = rclpy.create_node('mir_robot_1_driver')
        # self.__node.create_subscription(Twist, 'cmd_vel', self.__cmd_vel_callback, 1)
        self.__node.create_subscription(Motor, '/motor_cmd_1', self.__motor_callback, 1)
        self.__odom_pub = self.__node.create_publisher(Odometry, '/odom_1', 10)
        self.__tf_broadcaster = TransformBroadcaster(self.__node)

    def __cmd_vel_callback(self, twist):
        self.__target_twist = twist

        forward_speed = self.__target_twist.linear.x
        angular_speed = self.__target_twist.angular.z

        self.__command_motor_left = (forward_speed - angular_speed * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS
        self.__command_motor_right = (forward_speed + angular_speed * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS

    def __motor_callback(self, motor):
        self.__command_motor_left = motor.left_speed
        self.__command_motor_right = motor.right_speed

    def step(self):
        rclpy.spin_once(self.__node, timeout_sec=0)

        x, y, v = self.__gps.getValues()
        _, _, twist_yaw = self.__gyro.getValues()
        roll, pitch, yaw = self.__inertial_unit.getRollPitchYaw()

        # odom信息
        odom_msg = Odometry()
        odom_msg.header.stamp = self.__node.get_clock().now().to_msg()
        odom_msg.header.frame_id = 'odom'  # 确保这里设置了正确的帧ID        
        odom_msg.pose.pose.position.x = x
        odom_msg.pose.pose.position.y = y
        odom_msg.pose.pose.position.z = 0.0
        odom_msg.pose.pose.orientation.x = roll
        odom_msg.pose.pose.orientation.y = pitch
        odom_msg.pose.pose.orientation.z = yaw
        quaternion = tf_transformations.quaternion_from_euler(roll, pitch, yaw)
        odom_msg.pose.pose.orientation = Quaternion(x=quaternion[0], y=quaternion[1], z=quaternion[2], w=quaternion[3])
        # Remove covariance from the message (useless)
        odom_msg.pose.covariance = [0.0] * 36
        odom_msg.twist.covariance = [0.0] * 36
        odom_msg.twist.twist.linear.x = v
        odom_msg.twist.twist.angular.z = twist_yaw
        self.__odom_pub.publish(odom_msg)

        # 发布map到odom的转换
        t = TransformStamped()
        t.header.stamp = self.__node.get_clock().now().to_msg()
        t.header.frame_id = 'map'
        t.child_frame_id = 'odom'
        t.transform.translation.x = 0.0
        t.transform.translation.y = 0.0
        t.transform.translation.z = 0.0
        q = tf_transformations.quaternion_from_euler(0, 0, 0)
        t.transform.rotation.x = q[0]
        t.transform.rotation.y = q[1]
        t.transform.rotation.z = q[2]
        t.transform.rotation.w = q[3]

        self.__tf_broadcaster.sendTransform(t)


        # self.__node.get_logger().info(f'GPS: x={x:.2f}, y={y:.2f}, v={v:.2f}')
        # self.__node.get_logger().info(f'Gyro: zAxis_velocity={twist_yaw:.2f}')
        # self.__node.get_logger().info(f'Inertial unit: roll={roll:.2f}, pitch={pitch:.2f}, yaw={yaw:.2f}')

        self.__left_motor.setVelocity(self.__command_motor_left)
        self.__right_motor.setVelocity(self.__command_motor_right)