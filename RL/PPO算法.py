import gym
import torch
import torch.nn.functional as F
import numpy as np
import matplotlib.pyplot as plt
import rl_utils
"""PPO时TRPO的一个变种和升级，使用截断方式来更新策略函数"""
"""本质是一个off policy算法，使用重要性采样来更新策略函数，使用theta_old和theta_new来计算比值比例，并交替进行更新"""

"""
在一个iteration中，首先使用一个actor网络（take_action函数）来采样一条轨迹，然后使用这条轨迹来更新actor和critic网络
在更新网络过程在，update函数接受一个transition_dict，包含了上一iteration中所有信息，包括状态、动作、奖励等
进入update函数后，首先计算时序差分目标和时序差分误差，然后计算优势函数A的（广义优势估计GAE）
进入一个while，更新网络参数：
依据旧的网络参数和循环中新的网络参数，计算新的动作概率，然后计算比值比例
最后在一个范围内计算actor和critic的损失函数，然后使用梯度下降法更新两个网络的参数
"""

# 策略网络：输入层、隐藏层、输出层
class PolicyNet(torch.nn.Module):
    def __init__(self, state_dim, hidden_dim, action_dim):
        super(PolicyNet, self).__init__()
        self.fc1 = torch.nn.Linear(state_dim, hidden_dim)
        self.fc2 = torch.nn.Linear(hidden_dim, action_dim)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        return F.softmax(self.fc2(x), dim=1)

# 价值网络：输入层、隐藏层、输出层
class ValueNet(torch.nn.Module):
    def __init__(self, state_dim, hidden_dim):
        super(ValueNet, self).__init__()
        self.fc1 = torch.nn.Linear(state_dim, hidden_dim)
        self.fc2 = torch.nn.Linear(hidden_dim, 1)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        return self.fc2(x)


class PPO:
    ''' PPO算法,采用截断方式 '''
    def __init__(self, state_dim, hidden_dim, action_dim, actor_lr, critic_lr, lmbda, epochs, eps, gamma, device):
        
        # 初始化策略网络
        self.actor = PolicyNet(state_dim, hidden_dim, action_dim).to(device)
        # 初始化价值网络
        self.critic = ValueNet(state_dim, hidden_dim).to(device)
        # 初始化策略网络优化器
        self.actor_optimizer = torch.optim.Adam(self.actor.parameters(), lr=actor_lr)
        # 初始化价值网络优化器
        self.critic_optimizer = torch.optim.Adam(self.critic.parameters(), lr=critic_lr)
        
        self.gamma = gamma
        self.lmbda = lmbda
        self.epochs = epochs  # 一条序列的数据用来训练轮数
        self.eps = eps  # PPO中截断范围的参数
        self.device = device

    def take_action(self, state):
        # 将状态state转换为张量
        state = torch.tensor([state], dtype=torch.float).to(self.device)
        # 获取状态s下的动作概率分布
        probs = self.actor(state)
        # 定义一个类别分布，表示动作概率分布
        action_dist = torch.distributions.Categorical(probs)
        # 从动作概率分布中采样一个动作
        action = action_dist.sample()
        # 返回采样的动作
        return action.item()

    def update(self, transition_dict): # transition_dict为当前策略的一整条轨迹，包含了状态、动作、奖励等信息（如命名所示，这是一个字典）
        # 将transition_dict中的数据转换为张量
        states = torch.tensor(transition_dict['states'], dtype=torch.float).to(self.device)
        actions = torch.tensor(transition_dict['actions']).view(-1, 1).to(self.device)
        rewards = torch.tensor(transition_dict['rewards'], dtype=torch.float).view(-1, 1).to(self.device)
        next_states = torch.tensor(transition_dict['next_states'], dtype=torch.float).to(self.device)
        dones = torch.tensor(transition_dict['dones'], dtype=torch.float).view(-1, 1).to(self.device)
        # 时序差分目标：r+γV(s')，如果是终止状态，目标值为r
        td_target = rewards + self.gamma * self.critic(next_states) * (1 - dones)
        # 时序差分误差
        td_delta = td_target - self.critic(states)
        # 计算优势函数GAE（广义优势估计）
        advantage = rl_utils.compute_advantage(self.gamma, self.lmbda, td_delta.cpu()).to(self.device)
        # 保存一条数据的原始动作概率
        old_log_probs = torch.log(self.actor(states).gather(1, actions)).detach()

        for _ in range(self.epochs):
            # 计算新的动作概率
            # 从策略网络中获取特定动作的概率
            log_probs = torch.log(self.actor(states).gather(1, actions))
            # 计算比值比例
            # ratio = exp(new_log_probs - old_log_probs)，这是一个KL散度的估计！！！
            ratio = torch.exp(log_probs - old_log_probs)
            # surr1：未截断的PPO目标函数
            surr1 = ratio * advantage
            # surr2：截断的PPO目标函数
            # torch.clamp：将ratio限制在[1-eps, 1+eps]之间
            surr2 = torch.clamp(ratio, 1 - self.eps, 1 + self.eps) * advantage  # 截断
            actor_loss = torch.mean(-torch.min(surr1, surr2))  # PPO损失函数
            critic_loss = torch.mean(F.mse_loss(self.critic(states), td_target.detach()))
            self.actor_optimizer.zero_grad()
            self.critic_optimizer.zero_grad()
            actor_loss.backward()
            critic_loss.backward()
            self.actor_optimizer.step()
            self.critic_optimizer.step()

actor_lr = 1e-3
critic_lr = 1e-2
num_episodes = 500
hidden_dim = 128
gamma = 0.98
lmbda = 0.95
epochs = 10
eps = 0.2
device = torch.device("cuda") if torch.cuda.is_available() else torch.device("cpu")

env_name = 'CartPole-v0'
env = gym.make(env_name)
env.seed(0)
torch.manual_seed(0)
state_dim = env..shape[0]
action_dim = env.action_space.n
agent = PPO(state_dim, hidden_dim, action_dim, actor_lr, critic_lr, lmbda, epochs, eps, gamma, device)

return_list = rl_utils.train_on_policy_agent(env, agent, num_episodes)

episodes_list = list(range(len(return_list)))
plt.plot(episodes_list, return_list)
plt.xlabel('Episodes')
plt.ylabel('Returns')
plt.title('PPO on {}'.format(env_name))
plt.show()

mv_return = rl_utils.moving_average(return_list, 9)
plt.plot(episodes_list, mv_return)
plt.xlabel('Episodes')
plt.ylabel('Returns')
plt.title('PPO on {}'.format(env_name))
plt.show()