import os
import torch
import numpy as np
from pathlib import Path
import platform
from rl_rvo_nav.policy.policy_rnn_ac import rnn_ac # 用于加载预训练模型的模块
from math import pi, sin, cos, sqrt
import time 
"""
主要用于模型测试，并记录结果
"""
class post_train:
    def __init__(self, env, num_episodes=100, max_ep_len=150, acceler_vel = 1.0, reset_mode=3, render=True, save=False, neighbor_region=4, neighbor_num=5, args=None, print_info=False, **kwargs):

        self.env = env # 环境
        self.num_episodes=num_episodes # 测试次数
        self.max_ep_len = max_ep_len # 每个回合的最大步数，超过该步数则回合结束
        self.acceler_vel = acceler_vel # 智能体的加速度
        self.reset_mode = reset_mode # 重置模式
        self.render=render # 是否渲染
        self.save=save # 是否保存
        self.robot_number = self.env.ir_gym.robot_number # 机器人数量
        self.step_time = self.env.ir_gym.step_time # 步长

        self.inf_print = kwargs.get('inf_print', True) # 是否打印信息
        self.std_factor = kwargs.get('std_factor', 0.001) # 标准差因子
        # self.show_traj = kwargs.get('show_traj', False)
        self.show_traj = False # 是否显示轨迹
        self.traj_type = ''
        self.figure_format = kwargs.get('figure_format', 'png')

        self.nr = neighbor_region
        self.nm = neighbor_num
        self.args = args

        # 存放机器人的state
        self.robot_state_list = {i: [] for i in range(self.robot_number)}
        self.robot_time_list = {i: [] for i in range(self.robot_number)}

        self.print_info = print_info # 是否打印信息

    def policy_test(self, policy_type='drl', policy_path=None, policy_name='policy', result_path=None, result_name='/result.txt', figure_save_path=None, ani_save_path=None, policy_dict=False, once=False):
        # 策略测试函数
        if policy_type == 'drl':
            model_action = self.load_policy(policy_path, self.std_factor, policy_dict=policy_dict)
        # o: 观察值，r: 奖励值，d: 是否结束，ep_ret: 回合奖励，ep_len: 回合步数，n: 回合数
        o, r, d, ep_ret, ep_len, n = self.env.reset(mode=self.reset_mode), 0, False, 0, 0, 0
        # ep_ret_list: 回合奖励列表，speed_list: 速度列表，mean_speed_list: 平均速度列表，ep_len_list: 回合步数列表，sn: 成功次数
        ep_ret_list, speed_list, mean_speed_list, ep_len_list, sn = [], [], [], [], 0

        print('Policy Test Start !')

        figure_id = 0
        while n < self.num_episodes:

            # if n == 1:
            #     self.show_traj = True

            action_time_list = []

            if self.render or self.save:
                self.env.render(save=self.save, path=figure_save_path, i = figure_id, show_traj=self.show_traj, traj_type=self.traj_type)
            
            if policy_type == 'drl': 
                abs_action_list =[]
                for i in range(self.robot_number):

                    start_time = time.time()
                    a_inc = np.round(model_action(o[i]), 2) # 获取动作，保留两位小数
                    end_time = time.time()

                    temp = end_time - start_time
                    action_time_list.append(temp)
                    self.robot_time_list[i].append(0.1)
                    cur_time = np.sum(self.robot_time_list[i])

                    cur_state = self.env.ir_gym.robot_list[i].state
                    self.save_state_list(i, cur_state, cur_time)
                    # print('cur_state:', cur_state)
                    cur_vel = self.env.ir_gym.robot_list[i].vel_omni # 当前速度
                    abs_action = self.acceler_vel * a_inc + np.squeeze(cur_vel) # 计算绝对速度
                    abs_action_list.append(abs_action)
            # 在环境中执行动作，返回观察值、奖励值、完成标志、额外信息
            o, r, d, info = self.env.step_ir(abs_action_list, vel_type = 'omni')
            if self.print_info:
                print("Action:", abs_action_list)
                print("State:", o)
            # 列表推导式，用于生成一个新的列表
            # robot.vel_omni 用于获取全向速度，np.linalg.norm 用于计算速度的范数
            robot_speed_list = [np.linalg.norm(robot.vel_omni) for robot in self.env.ir_gym.robot_list]
            avg_speed = np.average(robot_speed_list) # 计算所有机器人的平均速度
            speed_list.append(avg_speed)

            ep_ret += r[0]
            ep_len += 1
            figure_id += 1
            # np.max(d) 检查是否有任何一个机器人到达了终止状态
            # ep_len == self.max_ep_len 检查当前回合的长度是否达到了最大长度
            # np.min(info) 检查是否有任何额外信息指示回合结束
            if np.max(d) or (ep_len == self.max_ep_len) or np.min(info):
                speed = np.mean(speed_list)
                figure_id = 0
                if np.min(info): # 任务成功
                    self.save_file()
                    ep_len_list.append(ep_len)
                    # ep_ret：累计奖励；ep_len：累计步长
                    if self.inf_print: print('Successful, Episode %d \t EpRet %.3f \t EpLen %d \t EpSpeed  %.3f'%(n, ep_ret, ep_len, speed))
                else: # 任务失败
                    if self.inf_print: print('Fail, Episode %d \t EpRet %.3f \t EpLen %d \t EpSpeed  %.3f'%(n, ep_ret, ep_len, speed))
    
                ep_ret_list.append(ep_ret)
                mean_speed_list.append(speed)
                speed_list = []

                o, r, d, ep_ret, ep_len = self.env.reset(mode=self.reset_mode), 0, False, 0, 0

                n += 1

                if np.min(info):
                    sn+=1
                    
                    # if n == 2: 
                        
                    if once:
                        self.env.ir_gym.world_plot.save_gif_figure(figure_save_path, 0, format='eps') # 保存gif
                        break
                        
                    if self.save:
                        self.env.ir_gym.save_ani(figure_save_path, ani_save_path, ani_name=policy_name) # 保存动画
                        break

        # 计算回合平均长度和标准差
        mean_len = 0 if len(ep_len_list) == 0 else np.round(np.mean(ep_len_list), 2)
        std_len = 0 if len(ep_len_list) == 0 else np.round(np.std(ep_len_list), 2)

        # 计算平均速度和标准差
        average_speed = np.round(np.mean(mean_speed_list),2)
        std_speed = np.round(np.std(mean_speed_list), 2)

        # 将结果写入文件
        f = open( result_path + result_name, 'a')
        print( 'policy_name: '+ policy_name, ' successful rate: {:.2%}'.format(sn/self.num_episodes), "average EpLen:", mean_len, "std length", std_len, 'average speed:', average_speed, 'std speed', std_speed, file = f)
        f.close() 
        # 结果打印到控制台
        print( 'policy_name: '+ policy_name, ' successful rate: {:.2%}'.format(sn/self.num_episodes), "average EpLen:", mean_len, 'std length', std_len, 'average speed:', average_speed, 'std speed', std_speed)


    def load_policy(self, filename, std_factor=1, policy_dict=False):
        # policy_dict：是否从检查点加载模型状态
        if policy_dict == True: # 从检查点加载模型状态
            # 模型初始化
            # print("observation_space:", self.env.observation_space)
            # print("action_space:", self.env.action_space)
            model = rnn_ac(self.env.observation_space, self.env.action_space, self.args.state_dim, self.args.rnn_input_dim, self.args.rnn_hidden_dim, self.args.hidden_sizes_ac, self.args.hidden_sizes_v, self.args.activation, self.args.output_activation, self.args.output_activation_v, self.args.use_gpu, self.args.rnn_mode)
            # 加载检查点
            check_point = torch.load(filename)
            # 加载模型状态
            model.load_state_dict(check_point['model_state'], strict=True)
            # 设置模型为评估模式
            model.eval()

        else: # 直接加载整个模型文件
            model = torch.load(filename)
            model.eval()

        # model.train()
        def get_action(x):
            with torch.no_grad(): # 不进行梯度计算
                x = torch.as_tensor(x, dtype=torch.float32) # 将输入数据转换为张量
                action = model.act(x, std_factor) # 获取动作
            return action

        return get_action
    
    def dis(self, p1, p2):
        # 距离计算函数
        return sqrt( (p2.py - p1.py)**2 + (p2.px - p1.px)**2 )
    
    def save_state_list(self, robot_id, state, time):
        state_with_time = np.vstack([state, np.array([[time]])])
        self.robot_state_list[robot_id].append(state_with_time)

    def save_file(self):
        # 获取webots_ws目录路径
        webots_ws_path = os.path.expanduser('~/webots_ws')
        # 构建文件路径
        file_path = os.path.join(webots_ws_path, 'robot_state_list.txt')
        # 将robot_state_list存储到txt文件中
        with open(file_path, 'w') as f:
            for robot_id, states in self.robot_state_list.items():
                for state in states:
                    f.write(f"{robot_id} {state[0][0]} {state[1][0]} {state[2][0]} {state[3][0]}\n")
        
