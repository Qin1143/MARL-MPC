import sys
import os
sys.path.append(os.path.abspath('/home/tony/webots_ws/rl_rvo_nav/gym_env/'))
import gym
import gym_env
from pathlib import Path # 用于处理文件路径的模块
import pickle # 用于序列化和反序列化对象的模块
from rl_rvo_nav.policy_test.post_train import post_train
import argparse # !!!用于解析命令行参数的模块!!!

# 初始化命令行参数解析器，用于解析和处理程序运行时的参数
# description：用于描述程序的功能和作用，会显示在帮助信息中
parser = argparse.ArgumentParser(description='policy test')

# 添加参数：策略类型，默认为'drl'，用于指定程序运行的策略算法类型
parser.add_argument('--policy_type', default='drl')

# 添加参数：模型路径，默认为'pre_trained'，指定预训练模型所在的目录
parser.add_argument('--model_path', default='pre_trained')

# 添加参数：模型名称，默认为'pre_train_check_point_1000.pt'，指定预训练模型的文件名
parser.add_argument('--model_name', default='pre_train_check_point_1000.pt')

# 添加参数：参数名称，默认为'pre_train'，用于在特定模型下进一步指定特定的运行参数
parser.add_argument('--arg_name', default='pre_train')

# 添加参数：世界配置文件名，默认为'test_world.yaml'，指定程序运行所使用的仿真世界配置文件
parser.add_argument('--world_name', default='test_world.yaml')  # 可选环境 test_world.yaml; test_world_lines.yaml; test_world_dyna_obs.yaml

# 添加参数：渲染开关，默认关闭，通过设置该参数为True，可以开启程序运行过程中的图形渲染
parser.add_argument('--render', action='store_true')

# 添加参数：机器人数量，类型为整数，默认为4，用于指定仿真世界中参与运行的机器人数量
parser.add_argument('--robot_number', type=int, default='4')

# 添加参数：运行 episode 数量，类型为整数，默认为100，用于指定程序运行的总 episode 数量
parser.add_argument('--num_episodes', type=int, default='1')

# 添加参数：运行模式，类型为整数，默认为3，用于指定程序的运行模式，可能影响程序的行为和策略
# 1: 线形分布，2: 随机分布，3: 圆形分布， 4: 未知，5: 对称分布
# parser.add_argument('--dis_mode', type=int, default='3')
parser.add_argument('--dis_mode', type=int, default='0')

# 添加参数：保存开关，默认关闭，通过设置该参数为True，可以开启程序运行结果的保存功能
parser.add_argument('--save', action='store_true')

# 添加参数：全量开关，默认关闭，通过设置该参数为True，可以开启程序的全量运行模式，可能会影响程序的运行策略和效率
parser.add_argument('--full', action='store_true')

# 添加参数：显示轨迹开关，默认关闭，通过设置该参数为True，可以开启程序运行过程中机器人轨迹的显示
parser.add_argument('--show_traj', action='store_true')

# 添加参数：单次运行开关，默认关闭，通过设置该参数为True，程序将运行一次后结束，常用于测试和调试
parser.add_argument('--once', action='store_true')


# 解析命令行参数并返回解析结果
policy_args = parser.parse_args()


# 获取当前脚本所在的路径，用于后续构建模型和参数文件的路径
cur_path = Path(__file__).parent

# 构建模型基础路径，通过当前脚本路径和模型路径拼接得到
# 在当前路径的pre_trained目录下存放了预训练模型和参数文件
model_base_path = str(cur_path) + '/' + policy_args.model_path

# 构建参数文件路径，通过模型基础路径和参数文件名拼接得到
args_path = model_base_path + '/' + policy_args.arg_name
print('args_path:', args_path)


# args from train
# 以二进制只读模式打开指定路径的文件，r表示“读取”，b表示binary二进制，用于后续的pickle加载操作
r = open(args_path, 'rb')

# 从打开的文件中加载pickle序列化后的对象，赋值给args变量
args = pickle.load(r)


# 根据策略类型确定模型文件名和策略名称
if policy_args.policy_type == 'drl':
    # 根据模型基本路径和模型名称构造完整路径

    fname_model = model_base_path + '/' + policy_args.model_name
    # 设置策略名称为'rl_rvo'
    policy_name = 'rl_rvo'


# 初始化环境
# 根据给定的参数配置环境，包括世界名称、机器人数量、邻居区域大小、邻居数量、机器人初始化模式等
env = gym.make('mrnav-v1', world_name=policy_args.world_name, robot_number=policy_args.robot_number, neighbors_region=args.neighbors_region, neighbors_num=args.neighbors_num, robot_init_mode=policy_args.dis_mode, env_train=False, random_bear=args.random_bear, random_radius=args.random_radius, reward_parameter=args.reward_parameter, goal_threshold=0.2, full=policy_args.full)

print("Observation Space: ", env.observation_space) # 输出观察空间 State
print("Action Space ", env.action_space) # 输出动作空间 Action
# env.reset()
# env.render()

# 构建策略名称
# 根据机器人数量和分布模式生成策略名称，用于后续的策略测试和区分
policy_name = policy_name + '_' + str(policy_args.robot_number) + '_dis' + str(policy_args.dis_mode)

# 进行训练后处理
# 对训练好的策略进行后处理，包括训练集外的测试、渲染等，调整策略的参数如标准差因子和最大步长
pt = post_train(env, num_episodes=policy_args.num_episodes, reset_mode=policy_args.dis_mode, render=policy_args.render, std_factor=0.00001, acceler_vel=1.0, max_ep_len=300, neighbors_region=args.neighbors_region, neighbor_num=args.neighbors_num, args=args, save=policy_args.save, show_traj=policy_args.show_traj, figure_format='eps', print_info=False)

# 测试策略性能
# 对特定策略进行测试，包括保存测试结果、绘制轨迹图和保存动画，策略测试可以是单次或多次
pt.policy_test(policy_args.policy_type, fname_model, policy_name, result_path=str(cur_path), result_name='/result.txt', figure_save_path=cur_path / 'figure' , ani_save_path=cur_path / 'gif', policy_dict=True,  once=policy_args.once)