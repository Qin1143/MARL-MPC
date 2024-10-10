import os
import sys
import gym
import pickle
import shutil
import gym_env
import argparse
from torch import nn
from pathlib import Path
sys.path.append("D:/GitCode/rl_rvo_nav")
from rl_rvo_nav.policy_train.multi_ppo import multi_ppo
from rl_rvo_nav.policy.policy_rnn_ac import rnn_ac

# path set
# 设置当前脚本所在目录的路径
cur_path = Path(__file__).parent
# 构建 'train_world.yaml' 文件的绝对路径，并将其转换为字符串
world_abs_path = str(cur_path/'train_world.yaml')

# default
# robot_number = 4
counter = 0

# model_path = cur_path / 'model_save' / ('r' + str(robot_number) )


# 初始化命令行参数解析器，用于解析和处理程序运行时的参数
# description：用于描述程序的功能和作用，会显示在帮助信息中
parser = argparse.ArgumentParser(description='drl rvo parameters')

# Create a group for environment parameters
# 创建一个新的参数组，用于组织和管理相关参数
# 'par env': 这是参数组的名称。'environment parameters': 这是参数组的描述。
par_env = parser.add_argument_group('par env', 'environment parameters') 
par_env.add_argument('--env_name', default='mrnav-v1')
par_env.add_argument('--world_path', default='train_world.yaml')
par_env.add_argument('--robot_number', type=int, default=4)
par_env.add_argument('--init_mode', default=3)
par_env.add_argument('--reset_mode', default=3)
par_env.add_argument('--mpi', default=False)

par_env.add_argument('--neighbors_region', default=4)
par_env.add_argument('--neighbors_num', type=int, default=5)   
par_env.add_argument('--reward_parameter', type=float, default=(3.0, 0.3, 0.0, 6.0, 0.3, 3.0, -0, 0), nargs='+')
par_env.add_argument('--env_train', default=True)
par_env.add_argument('--random_bear', default=True)
par_env.add_argument('--random_radius', default=False)
par_env.add_argument('--full', default=False)

# Create a group for policy parameters
par_policy = parser.add_argument_group('par policy', 'policy parameters') 
par_policy.add_argument('--state_dim', default=6)
par_policy.add_argument('--rnn_input_dim', default=8)
par_policy.add_argument('--rnn_hidden_dim', default=256)
par_policy.add_argument('--trans_input_dim', default=8)
par_policy.add_argument('--trans_max_num', default=10)
par_policy.add_argument('--trans_nhead', default=1)
par_policy.add_argument('--trans_mode', default='attn')
par_policy.add_argument('--hidden_sizes_ac', default=(256, 256))
par_policy.add_argument('--drop_p', type=float, default=0)
par_policy.add_argument('--hidden_sizes_v', type=tuple, default=(256, 256))  # 16 16
par_policy.add_argument('--activation', default=nn.ReLU)
par_policy.add_argument('--output_activation', default=nn.Tanh)
par_policy.add_argument('--output_activation_v', default=nn.Identity)
par_policy.add_argument('--use_gpu', action='store_true')   
par_policy.add_argument('--rnn_mode', default='biGRU')   # LSTM

# Create a group for training parameters
par_train = parser.add_argument_group('par train', 'train parameters') 
par_train.add_argument('--pi_lr', type=float, default=4e-6)
par_train.add_argument('--vf_lr', type=float, default=5e-5)
par_train.add_argument('--train_epoch', type=int, default=250)
par_train.add_argument('--steps_per_epoch', type=int, default=500)
par_train.add_argument('--max_ep_len', default=150)
par_train.add_argument('--gamma', default=0.99)
par_train.add_argument('--lam', default=0.97)
par_train.add_argument('--clip_ratio', default=0.2)
par_train.add_argument('--train_pi_iters', default=50)
par_train.add_argument('--train_v_iters', default=50)
par_train.add_argument('--target_kl',type=float, default=0.05)
par_train.add_argument('--render', default=True)
par_train.add_argument('--render_freq', default=50)
par_train.add_argument('--con_train', action='store_true')
par_train.add_argument('--seed', default=7)
par_train.add_argument('--save_freq', default=50)
par_train.add_argument('--save_figure', default=False)
par_train.add_argument('--figure_save_path', default='figure')
par_train.add_argument('--save_path', default=str(cur_path / 'model_save') + '/')
par_train.add_argument('--save_name', default= 'r')
par_train.add_argument('--load_path', default=str(cur_path / 'model_save')+ '/')
par_train.add_argument('--load_name', default='r4_0/r4_0_check_point_250.pt') # '/r4_0/r4_0_check_point_250.pt' 
par_train.add_argument('--save_result', type=bool, default=True)
par_train.add_argument('--lr_decay_epoch', type=int, default=1000)
par_train.add_argument('--max_update_num', type=int, default=10)

# 解析命令行参数并返回解析结果
args = parser.parse_args()

# decide the model path and model name
# 构建用于检查模型路径和名称的格式化字符串
model_path_check = args.save_path + args.save_name + str(args.robot_number) + '_{}'
model_name_check = args.save_name + str(args.robot_number) +  '_{}'
# 循环检查是否存在相应的目录，如果存在则增加计数器
while os.path.isdir(model_path_check.format(counter)): # 字符串格式化操作，将字符串中的 '{}' 替换为 counter 的值
    counter+=1
# 确定最终的模型绝对路径
model_abs_path = model_path_check.format(counter) + '/'
# 确定最终的模型名称
model_name = model_name_check.format(counter)

load_fname = args.load_path + args.load_name

# 创建训练环境
env = gym.make(args.env_name, world_name=args.world_path, robot_number=args.robot_number, neighbors_region=args.neighbors_region, neighbors_num=args.neighbors_num, robot_init_mode=args.init_mode, env_train=args.env_train, random_bear=args.random_bear, random_radius=args.random_radius, reward_parameter=args.reward_parameter, full=args.full)
# 创建测试环境
test_env = gym.make(args.env_name, world_name=args.world_path, robot_number=args.robot_number, neighbors_region=args.neighbors_region, neighbors_num=args.neighbors_num, robot_init_mode=args.init_mode, env_train=False, random_bear=args.random_bear, random_radius=args.random_radius, reward_parameter=args.reward_parameter, plot=False, full=args.full)
# 创建策略网络
policy = rnn_ac(env.observation_space, env.action_space, args.state_dim, args.rnn_input_dim, args.rnn_hidden_dim, 
                    args.hidden_sizes_ac, args.hidden_sizes_v, args.activation, args.output_activation, 
                    args.output_activation_v, args.use_gpu, args.rnn_mode, args.drop_p)
# 创建多智能体PPO算法实例
ppo = multi_ppo(env, policy, args.pi_lr, args.vf_lr, args.train_epoch, args.steps_per_epoch, args.max_ep_len, args.gamma, args.lam, args.clip_ratio, args.train_pi_iters, args.train_v_iters, args.target_kl, args.render, args.render_freq, args.con_train,  args.seed, args.save_freq, args.save_figure, model_abs_path, model_name, load_fname, args.use_gpu, args.reset_mode, args.save_result, counter, test_env, args.lr_decay_epoch, args.max_update_num, args.mpi, args.figure_save_path)

# save hyparameters
# 检查路径是否存在，如果不存在则创建路径
if not os.path.exists(model_abs_path):
    os.makedirs(model_abs_path)

# 使用 pickle 序列化 args 参数并保存到文件中
f = open(model_abs_path + model_name, 'wb')
pickle.dump(args, f)
f.close()

# 将 args 对象的文本表示保存到一个 .txt 文件中
with open(model_abs_path+model_name+'.txt', 'w') as p:
    print(vars(args), file=p)
p.close()

shutil.copyfile( str(cur_path/'train_world.yaml'), model_abs_path+model_name+'_world.yaml')

# run the training loop
ppo.training_loop()

