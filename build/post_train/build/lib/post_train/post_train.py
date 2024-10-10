import os
import rclpy
import torch
import pickle
from pathlib import Path
import numpy as np
import tf_transformations
from rclpy.node import Node
from rclpy.parameter import Parameter
from nav_msgs.msg import Odometry
from action_interfaces.msg import Action
from rl_rvo_nav.policy.policy_rnn_ac import rnn_ac
from math import sin, cos, atan2, pi, sqrt
from gym import spaces
import sys
sys.path.append('/home/tony/webots_ws/src/post_train/post_train')
from rvo_inter import rvo_inter

class PostTrainNode(Node):
    def __init__(self):
        super().__init__('post_train_node')
        
        # 声明参数
        self.declare_parameter('robot_num', 4)
        self.declare_parameter('accuracy', 4)
        self.declare_parameter('neighbors_region', 10.0)
        self.declare_parameter('neighbors_num', 10)
        self.declare_parameter('vxmax', 1.5)
        self.declare_parameter('vymax', 1.5)
        self.declare_parameter('acceler', 0.5)
        self.declare_parameter('env_train', True)
        self.declare_parameter('radius_exp', 0.2)

        self.goal_list = [[9, 1], [9, 9], [1, 9], [1, 1]]

        self.robot_num = self.get_robot_num()
        self.accuracy = self.get_accuracy()
        self.odom_pos = [None] * self.robot_num
        self.odom_linear_vel = [None] * self.robot_num
        self.odom_angular_vel = [None] * self.robot_num
        self.yaw = [None] * self.robot_num
        self.des_vel_list = [np.zeros(2) for _ in range(self.robot_num)]
        self.acceler_vel = 1.0
        neighbors_region, neighbors_num, vxmax, vymax, acceler, env_train, self.radius_exp = self.get_rvo_config()
        self.vel_max = [vxmax, vymax]
        self.goal_threshold = 0.1
        self.rvo = rvo_inter(neighbors_region, neighbors_num, vxmax, vymax, acceler, env_train, self.radius_exp)
        # 加载模型
        cur_path = '/home/tony/webots_ws/src/post_train/post_train'
        self.policy_path = str(cur_path) + '/' + 'pre_trained/pre_train_check_point_1000.pt'
        self.args_path = str(cur_path) + '/' + 'pre_trained/pre_train'
        # print("Policy Path: ", self.policy_path)
        # print("Args Path: ", self.args_path)
        self.std_factor = 0.001

        # 订阅各个机器人的odom
        self.sub_topic(self.robot_num)
        # 发布动作
        self.pub_action = self.create_publisher(Action, 'actions', 10)
        self.timer = self.create_timer(1, self.timer_callback)

    def get_robot_num(self):
        robot_num = self.get_parameter('robot_num').get_parameter_value().integer_value
        
        if robot_num is None:
            self.get_logger().error("Parameter 'robot_num' not found on the parameter server.")
        else:
            self.get_logger().info(f"Received 'robot_num': {robot_num}")
        
        return robot_num
    
    def get_accuracy(self):
        accuracy = self.get_parameter('accuracy').get_parameter_value().integer_value
        
        if accuracy is None:
            self.get_logger().error("Parameter 'accuracy' not found on the parameter server.")
        else:
            self.get_logger().info(f"Received 'accuracy': {accuracy}")
        
        return accuracy
    
    def get_rvo_config(self):
        neighbors_region = self.get_parameter('neighbors_region').get_parameter_value().double_value
        neighbors_num = self.get_parameter('neighbors_num').get_parameter_value().integer_value
        vxmax = self.get_parameter('vxmax').get_parameter_value().double_value
        vymax = self.get_parameter('vymax').get_parameter_value().double_value
        acceler = self.get_parameter('acceler').get_parameter_value().double_value
        env_train = self.get_parameter('env_train').get_parameter_value().bool_value
        radius_exp = self.get_parameter('radius_exp').get_parameter_value().double_value

        if neighbors_region is None or neighbors_num is None or vxmax is None or vymax is None or acceler is None or env_train is None or radius_exp is None:
            self.get_logger().error("One or more parameters not found on the parameter server.")
        else:
            self.get_logger().info("RVO configuration received.")
        
        return neighbors_region, neighbors_num, vxmax, vymax, acceler, env_train, radius_exp

    def sub_topic(self, robot_num):
        for i in range(robot_num):
            sub_robot_odom = self.create_subscription(
                Odometry,
                f'/odom_{i + 1}',
                lambda msg, robot_id=i: self.odom_callback(robot_id, msg),
                10
            )

    def odom_callback(self, robot_id, msg):
            odom_pos_ = np.zeros(2)
            odom_linear_vel_ = np.zeros(2)
            odom_angular_vel_ = np.zeros(1)

            # 读取当前状态
            odom_pos_[0] = round(msg.pose.pose.position.x, self.accuracy) # x
            odom_pos_[1] = round(msg.pose.pose.position.y, self.accuracy) # y

            odom_linear_vel_[0] = round(msg.twist.twist.linear.x, self.accuracy) # vx
            odom_linear_vel_[1] = round(msg.twist.twist.linear.y, self.accuracy) # vy

            odom_angular_vel_[0] = round(msg.twist.twist.angular.z, self.accuracy) # wz

            self.odom_pos[robot_id] = odom_pos_
            self.odom_linear_vel[robot_id] = odom_linear_vel_
            self.odom_angular_vel[robot_id] = odom_angular_vel_

            quat = [msg.pose.pose.orientation.x, msg.pose.pose.orientation.y, msg.pose.pose.orientation.z, msg.pose.pose.orientation.w]
            roll_, pitch_, yaw_ = tf_transformations.euler_from_quaternion(quat)
            self.yaw[robot_id] = yaw_

    def get_observation(self, robot_id, des_vel = np.zeros(2)):
        cur_vel = self.odom_linear_vel[robot_id]
        # self.get_logger().info(f"cur_vel:, {cur_vel}")
        radian = self.yaw[robot_id]
        radius = self.radius_exp
        nei_state_list = []
        propri_obs = np.concatenate([cur_vel, des_vel, np.array([radian]), np.array([radius]) ])
        for i in range(self.robot_num):
            # self.get_logger().info(f"odom_pos[i]: {self.odom_pos[i]}")
            if i != robot_id:
                nei_state = np.concatenate([self.odom_pos[i], self.odom_linear_vel[i], np.array([self.radius_exp]) ])
                nei_state_list.append(nei_state)

        robot_state = np.concatenate([self.odom_pos[robot_id], cur_vel, np.array([radius]), des_vel])
        robot_state = robot_state.reshape(-1, 1)
        # print("robot_state:", robot_state)
        # print("nei_state_list:", nei_state_list)
        obs_circular_list = np.array([])
        obs_line_list = np.array([])
        obs_vo_list, _, _, _ = self.rvo.config_vo_inf(robot_state, nei_state_list, obs_circular_list, obs_line_list)
        if len(obs_vo_list) == 0:
            exter_obs = np.zeros((8,)) 
        else:
            exter_obs = np.concatenate(obs_vo_list) # vo list
        observation = np.round(np.concatenate([propri_obs, exter_obs]), 2)
        # self.get_logger().info(f"observation:, {observation}")
        return observation

    def get_action(self, observation):
        model_action = self.load_policy(self.policy_path, self.std_factor, policy_dict=True)
        a_inc = np.round(model_action(observation), 2)
        # self.get_logger().info(f"a_inc:, {a_inc}")
        return a_inc

    def load_policy(self, filename, std_factor=1, policy_dict=False):
        if policy_dict == True:
            observation_space = spaces.Box(low=-np.inf, high=np.inf, shape=(5,), dtype=np.float32)
            action_space = spaces.Box(low=-1.0, high=1.0, shape=(2,), dtype=np.float32)
            r = open(self.args_path, 'rb')
            args = pickle.load(r)
            model = rnn_ac(observation_space, action_space, args.state_dim, args.rnn_input_dim, args.rnn_hidden_dim, args.hidden_sizes_ac, args.hidden_sizes_v, args.activation, args.output_activation, args.output_activation_v, args.use_gpu, args.rnn_mode)
            check_point = torch.load(filename)
            model.load_state_dict(check_point['model_state'], strict=True)
            model.eval()

        else:
            model = torch.load(filename)
            model.eval()

        def get_action(x):
            with torch.no_grad():
                x = torch.as_tensor(x, dtype=torch.float32)
                action = model.act(x, std_factor)
            return action

        return get_action
    
    def cal_des_vel_omni(self, state, goal):
        print("state:", state)
        print("goal:", goal)
        dis, radian = self.relative(state, goal)
        
        if dis > self.goal_threshold:
            vx = self.vel_max[0] * cos(radian)
            vy = self.vel_max[1] * sin(radian)
        else:
            vx = 0
            vy = 0

        return np.array([vx, vy])
    
    def relative(self, state1, state2):
        state1 = np.array(state1)
        state2 = np.array(state2)
        dif = state2[0:2] - state1[0:2]

        dis = np.linalg.norm(dif)
        radian = atan2(dif[1], dif[0])
        
        return dis, radian

    def timer_callback(self):
        abs_action_list_ = []
        msg = Action()

        for i in range(self.robot_num):
            cur_state = self.odom_pos[i].tolist()
            cur_goal = self.goal_list[i]
            cur_des_vel = self.cal_des_vel_omni(cur_state, cur_goal)
            observation = self.get_observation(robot_id=i, des_vel=cur_des_vel)
            a_inc = self.get_action(observation)
            cur_vel = self.odom_linear_vel[i]
            abs_action = self.acceler_vel * a_inc + np.squeeze(cur_vel)
            abs_action_list_.append(abs_action.tolist())
        # self.get_logger().info(f"abs_action_list: {abs_action_list}")
        abs_action_list = [item for sublist in abs_action_list_ for item in sublist]
        # print("abs_action_list:", self.des_vel_list)
        msg.action = abs_action_list
        self.pub_action.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = PostTrainNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()