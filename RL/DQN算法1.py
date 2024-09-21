import random
import gym
import numpy as np
import collections
from tqdm import tqdm
import torch
import torch.nn.functional as F
import matplotlib.pyplot as plt
import rl_utils
from matplotlib import animation


class ReplayBuffer:
    ''' 经验回放池 '''
    def __init__(self, capacity):
        self.buffer = collections.deque(maxlen=capacity)  # deque双端队列,先进先出
        # 设置最大长度maxlen为capacity，若超出则删除最早加入的元素

    def add(self, state, action, reward, next_state, done):  # 将数据(s,a,r,s_,done)加入buffer
        self.buffer.append((state, action, reward, next_state, done))

    def sample(self, batch_size):  # 从buffer中采样数据,数量为batch_size
        transitions = random.sample(self.buffer, batch_size)
        # 使用了zip函数对transitions进行解包，得到state, action, reward, next_state, done
        # *为解包操作符，将transitions中的元素解包成单独的列表
        state, action, reward, next_state, done = zip(*transitions) 
        return np.array(state), action, reward, np.array(next_state), done

    def size(self):  # 目前buffer中数据的数量
        return len(self.buffer)
    
class Qnet(torch.nn.Module):
    ''' 只有一层隐藏层的Q网络 '''
    def __init__(self, state_dim, hidden_dim, action_dim): # 三层网络：输入维度、隐藏层维度、输出维度
        super(Qnet, self).__init__() # 初始化Qnet的类，这个类继承自torch.nn.Module
        self.fc1 = torch.nn.Linear(state_dim, hidden_dim) # 全连接层（线性层），输入维度为state_dim，输出维度为hidden_dim
        self.fc2 = torch.nn.Linear(hidden_dim, action_dim)

    def forward(self, x): # 定义前向传播过程，x为输入张量
        x = F.relu(self.fc1(x))  # 在全连接层后使用ReLU激活函数
        return self.fc2(x)
    
class DQN:
    ''' DQN算法 '''
    def __init__(self, state_dim, hidden_dim, action_dim, learning_rate, gamma,
                 epsilon, target_update, device): # 初始化函数，learning_rate学习率，gamma折扣因子，epsilon探索率，target_update目标网络更新频率
        self.action_dim = action_dim
        # Q网络
        self.q_net = Qnet(state_dim, hidden_dim,
                          self.action_dim).to(device)
        # 目标网络
        self.target_q_net = Qnet(state_dim, hidden_dim,
                                 self.action_dim).to(device)
        # 使用Adam优化器，计算损失相对于所有可学习参数的梯度，并更新参数；lr为学习率
        self.optimizer = torch.optim.Adam(self.q_net.parameters(),
                                          lr=learning_rate)
        self.gamma = gamma  # 折扣因子
        self.epsilon = epsilon  # epsilon-贪婪策略
        self.target_update = target_update  # 目标网络更新频率
        self.count = 0  # 计数器,记录更新次数
        self.device = device

    def take_action(self, state): # epsilon-贪婪策略采取动作
        # state = state[0]  # 提取实际的状态向量
        state = np.array(state, dtype=np.float32)  # 先将 state 转换为 NumPy 数组
        state = torch.tensor(state, dtype=torch.float32).unsqueeze(0).to(self.device)  # 再转换为 PyTorch 张量并添加批量维度
        if np.random.random() < self.epsilon:
            action = np.random.randint(self.action_dim) # 以epsilon的概率随机选择动作
        else:
            # state = torch.tensor([state], dtype=torch.float).to(self.device) # 将state转换为张量
            action = self.q_net(state).argmax().item() # 从Q网络中选择最大Q值对应的动作
        return action

    def update(self, transition_dict):
        # 从transition_dict中提取数据"states"转换为张量
        states = torch.tensor(transition_dict['states'],
                              dtype=torch.float).to(self.device)
        # 从transition_dict中提取数据"actions"转换为张量
        # view(-1, 1)：将张量调整为形状为 (-1, 1) 的二维张量。-1 表示自动计算该维度的大小，以确保总元素数量不变。这里的作用是将动作数据转换为列向量，每个动作占据一行
        actions = torch.tensor(transition_dict['actions']).view(-1, 1).to(
            self.device)
        
        rewards = torch.tensor(transition_dict['rewards'],
                               dtype=torch.float).view(-1, 1).to(self.device)
        
        next_states = torch.tensor(transition_dict['next_states'],
                                   dtype=torch.float).to(self.device)
        
        dones = torch.tensor(transition_dict['dones'],
                             dtype=torch.float).view(-1, 1).to(self.device)
        # gather 是 PyTorch 张量的一个方法，用于根据指定的索引提取值
        # 1 表示在第 1 个维度（即列）上进行操作
        # 获取state对应的所有动作的Q值，(batch_size, 1)
        q_values = self.q_net(states).gather(1, actions)
        # 下个状态的最大Q值
        # .max(1) 方法在第 1 个维度（即列）上计算最大值，返回值是一个元组，第一个元素是最大值，第二个元素是最大值的索引
        max_next_q_values = self.target_q_net(next_states).max(1)[0].view(
            -1, 1)
        # TD误差目标
        # 对于终止状态，(1 - dones) 为 0，因此公式为 rewards，表示仅考虑当前奖励，不考虑未来奖励
        q_targets = rewards + self.gamma * max_next_q_values * (1 - dones)
        dqn_loss = torch.mean(F.mse_loss(q_values, q_targets))  # 均方误差损失函数
        self.optimizer.zero_grad()  # PyTorch中默认梯度会累积,这里需要显式将梯度置为0
        dqn_loss.backward()  # 反向传播更新参数
        self.optimizer.step()

        if self.count % self.target_update == 0:
            self.target_q_net.load_state_dict(self.q_net.state_dict())  # 更新目标网络
        self.count += 1

lr = 2e-3 # 学习率，表示模型在每次更新时调整权重的步长
num_episodes = 500 # 训练的总回合数
hidden_dim = 128 # 隐藏层的维度，表示神经网络隐藏层的神经元数量
gamma = 0.98 # 折扣因子，表示未来奖励的折扣率
epsilon = 0.01 # 探索率，表示在训练过程中选择随机动作的概率
target_update = 10 # 目标网络更新频率
buffer_size = 10000 # buffer的大小
minimal_size = 500 # buffer中数据的最小数量
batch_size = 64 # 每次训练的批量大小
device = torch.device("cuda") if torch.cuda.is_available() else torch.device(
    "cpu")

env_name = 'CartPole-v0'
env = gym.make(env_name)
random.seed(0)
np.random.seed(0)
# env.reset(seed=0)
env.seed(0)
# env.action_space.seed(0)
# env.observation_space.seed(0)
torch.manual_seed(0)
replay_buffer = ReplayBuffer(buffer_size)
state_dim = env.observation_space.shape[0] # 获取环境的状态空间维度
print(f"State dimension: {state_dim}")
action_dim = env.action_space.n # env.action_space.n 表示动作空间中可用动作的数量
print(f"Action dimension: {action_dim}")
agent = DQN(state_dim, hidden_dim, action_dim, lr, gamma, epsilon,
            target_update, device) # agent受DQN算法控制

return_list = []
for i in range(10):
    with tqdm(total=int(num_episodes / 10), desc='Iteration %d' % i) as pbar:
        for i_episode in range(int(num_episodes / 10)):
            episode_return = 0
            state = env.reset()
            # print(f"Initial state: {state}")  # 打印初始状态
            done = False
            while not done:
                # print(f"State: {state}")  # 打印状态
                action = agent.take_action(state)
                # print(f"Step result: {env.step(action)}")  # 打印step的返回值
                next_state, reward, done, _ = env.step(action)
                replay_buffer.add(state, action, reward, next_state, done)
                state = next_state
                episode_return += reward
                # 当buffer数据的数量超过一定值后,才进行Q网络训练
                if replay_buffer.size() > minimal_size:
                    b_s, b_a, b_r, b_ns, b_d = replay_buffer.sample(batch_size)
                    transition_dict = {
                        'states': b_s,
                        'actions': b_a,
                        'next_states': b_ns,
                        'rewards': b_r,
                        'dones': b_d
                    }
                    agent.update(transition_dict)
            return_list.append(episode_return)
            if (i_episode + 1) % 10 == 0:
                pbar.set_postfix({
                    'episode':
                    '%d' % (num_episodes / 10 * i + i_episode + 1),
                    'return':
                    '%.3f' % np.mean(return_list[-10:])
                })
            pbar.update(1)

episodes_list = list(range(len(return_list)))
plt.plot(episodes_list, return_list)
plt.xlabel('Episodes')
plt.ylabel('Returns')
plt.title('DQN on {}'.format(env_name))
plt.show()

mv_return = rl_utils.moving_average(return_list, 9)
plt.plot(episodes_list, mv_return)
plt.xlabel('Episodes')
plt.ylabel('Returns')
plt.title('DQN on {}'.format(env_name))
plt.show()

# # 记录DQN代理在新环境中的运行结果
# frames = []
# state = env.reset()
# done = False
# while not done:
#     frames.append(env.render(mode='rgb_array'))
#     action = agent.take_action(state)
#     state, reward, done, _ = env.step(action)

# # 关闭展示环境
# env.close()

# # 使用matplotlib显示动画
# fig = plt.figure()
# patch = plt.imshow(frames[0])
# plt.axis('off')

# def animate(i):
#     patch.set_data(frames[i])

# ani = animation.FuncAnimation(fig, animate, frames=len(frames), interval=50)
# plt.show()