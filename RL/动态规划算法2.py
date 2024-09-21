import gym
import time  # 导入time模块
from 动态规划算法1 import PolicyIteration, ValueIteration, print_agent

env = gym.make("FrozenLake-v1", render_mode="human")  # 创建环境
env = env.unwrapped  # 解封装才能访问状态转移矩阵P
env.reset()  # 初始化环境状态
env.render()  # 环境渲染,通常是弹窗显示或打印出可视化的环境
# time.sleep(5)  # 让渲染的环境显示5秒

holes = set()
ends = set()
for s in env.P:
    for a in env.P[s]:
        for s_ in env.P[s][a]:
            if s_[2] == 1.0:  # 获得奖励为1,代表是目标
                ends.add(s_[1])
            if s_[3] == True:
                holes.add(s_[1])
holes = holes - ends
print("冰洞的索引:", holes)
print("目标的索引:", ends)

for a in env.P[14]:  # 查看目标左边一格的状态转移信息
    print(env.P[14][a])

# 这个动作意义是Gym库针对冰湖环境事先规定好的
action_meaning = ['<', 'v', '>', '^']
theta = 1e-5
gamma = 0.9
agent = PolicyIteration(env, theta, gamma)
agent.policy_iteration()
print_agent(agent, action_meaning, [5, 7, 11, 12], [15])

action_meaning = ['<', 'v', '>', '^']
theta = 1e-5
gamma = 0.9
agent = ValueIteration(env, theta, gamma)
agent.value_iteration()
print_agent(agent, action_meaning, [5, 7, 11, 12], [15])