import torch
import numpy as np
import gym
import matplotlib.pyplot as plt
import torch.nn.functional as F
import rl_utils
import copy

"""TRPO算法与Actor-Critic算法的区别在于策略更新的方式
TRPO算法使用线性搜索和共轭梯度法来更新策略函数
以保证策略更新后的KL距离不超过一个阈值"""

"""但TRPO算法的缺点是计算复杂度高，每一步的运算量非常大"""

# 策略网络，与A-C算法一样
class PolicyNet(torch.nn.Module):
    def __init__(self, state_dim, hidden_dim, action_dim):
        super(PolicyNet, self).__init__()
        self.fc1 = torch.nn.Linear(state_dim, hidden_dim)
        self.fc2 = torch.nn.Linear(hidden_dim, action_dim)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        return F.softmax(self.fc2(x), dim=1)

# 价值网络，与A-C算法一样
class ValueNet(torch.nn.Module):
    def __init__(self, state_dim, hidden_dim):
        super(ValueNet, self).__init__()
        self.fc1 = torch.nn.Linear(state_dim, hidden_dim)
        self.fc2 = torch.nn.Linear(hidden_dim, 1)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        return self.fc2(x)


class TRPO:
    """ TRPO算法 """
    def __init__(self, hidden_dim, state_space, action_space, lmbda,
                 kl_constraint, alpha, critic_lr, gamma, device):
        state_dim = state_space.shape[0]
        action_dim = action_space.n
        # 策略网络参数不需要优化器更新
        self.actor = PolicyNet(state_dim, hidden_dim, action_dim).to(device)
        self.critic = ValueNet(state_dim, hidden_dim).to(device)
        self.critic_optimizer = torch.optim.Adam(self.critic.parameters(), lr=critic_lr)
        self.gamma = gamma
        self.lmbda = lmbda  # GAE参数,用于计算优势函数
        self.kl_constraint = kl_constraint  # KL距离最大限制
        self.alpha = alpha  # 线性搜索参数
        self.device = device

    # 计算优势函数
    def compute_advantage(self, gamma, lmbda, td_delta):
        # 将PyTorch张量td_delta转换为NumPy数组
        td_delta = td_delta.detach().numpy()
        # 初始化优势函数列表和优势值
        advantage_list = []
        advantage = 0.0
        # 从最后一个时间步开始计算优势函数
        for delta in td_delta[::-1]:
            advantage = gamma * lmbda * advantage + delta
            advantage_list.append(advantage)
        advantage_list.reverse()
        return torch.tensor(advantage_list, dtype=torch.float)

    # 这里也和A-C算法一样
    def take_action(self, state):
        state = torch.tensor([state], dtype=torch.float).to(self.device)
        probs = self.actor(state)
        action_dist = torch.distributions.Categorical(probs)
        action = action_dist.sample()
        return action.item()

    def hessian_matrix_vector_product(self, states, old_action_dists, vector): # 计算黑塞矩阵和一个向量的乘积
        # 使用当前策略网络计算给定状态下的动作概率分布
        new_action_dists = torch.distributions.Categorical(self.actor(states))
        # 计算旧的动作分布和新的动作分布之间的平均KL散度，KL散度用于衡量两个概率分布之间的差异
        kl = torch.mean(torch.distributions.kl.kl_divergence(old_action_dists, new_action_dists))
        # 使用自动微分计算KL散度相对于策略网络参数的一阶导数（梯度），create_graph=True表示创建计算图，以便后续计算二阶导数
        kl_grad = torch.autograd.grad(kl, self.actor.parameters(), create_graph=True)
        # 将梯度展平并连接成一个向量
        kl_grad_vector = torch.cat([grad.view(-1) for grad in kl_grad])
        # KL距离的梯度先和向量进行点积运算
        kl_grad_vector_product = torch.dot(kl_grad_vector, vector)
        # 使用自动微分计算点积结果相对于策略网络参数的二阶导数（Hessian向量积）
        grad2 = torch.autograd.grad(kl_grad_vector_product, self.actor.parameters())
        # 将二阶导数展平并连接成一个向量
        grad2_vector = torch.cat([grad.view(-1) for grad in grad2])
        return grad2_vector

    # 共轭梯度法求解方程，Hx=g
    def conjugate_gradient(self, grad, states, old_action_dists):
        x = torch.zeros_like(grad) # 初始化为与梯度相同形状的零向量，表示初始解
        r = grad.clone() # 初始化为梯度的克隆，表示初始残差
        p = grad.clone() # 初始化为梯度的克隆，表示初始搜索方向
        rdotr = torch.dot(r, r) # 计算初始残差的内积，用于后续计算
        for i in range(10):  # 共轭梯度主循环
            # alpha = r^T * r / p^T * H * p
            # 只计算Hp的向量乘积，而不直接计算和存储H矩阵
            Hp = self.hessian_matrix_vector_product(states, old_action_dists, p)
            alpha = rdotr / torch.dot(p, Hp)
            x += alpha * p
            r -= alpha * Hp
            new_rdotr = torch.dot(r, r)
            if new_rdotr < 1e-10:
                break
            beta = new_rdotr / rdotr
            p = r + beta * p
            rdotr = new_rdotr
        return x # x = H^(-1)g，表示网络参数的更新方向

    # 计算策略目标，衡量策略更新的好坏
    def compute_surrogate_obj(self, states, actions, advantage, old_log_probs, actor):
        # 使用当前策略网络（actor）计算给定状态下每个动作的对数概率
        log_probs = torch.log(actor(states).gather(1, actions))
        # 计算新旧策略的概率比率（ratio），即新策略下动作的概率与旧策略下动作的概率之比
        ratio = torch.exp(log_probs - old_log_probs)
        # 计算策略目标（surrogate objective），即概率比率与优势函数（advantage）的乘积的平均值
        return torch.mean(ratio * advantage)

    # 线性搜索，用于找到一个新的策略参数，使得新的策略目标比旧的策略目标更好，同时满足KL距离的约束
    def line_search(self, states, actions, advantage, old_log_probs, old_action_dists, max_vec):
        # 将当前策略网络的参数转换为向量形式
        old_para = torch.nn.utils.convert_parameters.parameters_to_vector(self.actor.parameters())
        # 计算当前策略的更新目标
        old_obj = self.compute_surrogate_obj(states, actions, advantage, old_log_probs, self.actor)
        for i in range(15):  # 线性搜索主循环，new_para = old_para + alpha^i * max_vec
            # 找到一个最小非负数i，使得新的策略目标比旧的策略目标更好，同时KL距离的约束满足
            coef = self.alpha**i
            new_para = old_para + coef * max_vec
            new_actor = copy.deepcopy(self.actor)
            # 将新的参数向量转换回策略网络的参数形式
            torch.nn.utils.convert_parameters.vector_to_parameters(new_para, new_actor.parameters())
            # 使用新的策略网络计算给定状态下的动作概率分布
            new_action_dists = torch.distributions.Categorical(new_actor(states))
            # 计算新的动作分布和旧的动作分布之间的平均KL散度
            kl_div = torch.mean(torch.distributions.kl.kl_divergence(old_action_dists, new_action_dists))
            # 计算新的策略目标
            new_obj = self.compute_surrogate_obj(states, actions, advantage, old_log_probs, new_actor)
            if new_obj > old_obj and kl_div < self.kl_constraint: # 如果新的策略目标更好且KL距离约束满足
                return new_para
        return old_para

    # 更新策略网络
    def policy_learn(self, states, actions, old_action_dists, old_log_probs, advantage):
        # 计算策略目标
        surrogate_obj = self.compute_surrogate_obj(states, actions, advantage, old_log_probs, self.actor)
        # 计算策略目标相对于策略网络参数的梯度
        grads = torch.autograd.grad(surrogate_obj, self.actor.parameters())
        # 将梯度展平并连接成一个向量
        obj_grad = torch.cat([grad.view(-1) for grad in grads]).detach()
        # 用共轭梯度法计算x = H^(-1)g
        descent_direction = self.conjugate_gradient(obj_grad, states, old_action_dists)
        #计算黑塞矩阵和下降方向的乘积
        Hd = self.hessian_matrix_vector_product(states, old_action_dists, descent_direction)
        # 计算最大步长系数
        max_coef = torch.sqrt(2 * self.kl_constraint / (torch.dot(descent_direction, Hd) + 1e-8))
        # 用线性搜索后的参数更新策略网络
        new_para = self.line_search(states, actions, advantage, old_log_probs, old_action_dists,
                                    descent_direction * max_coef)  # 线性搜索
        # 用线性搜索后的参数更新策略网络
        torch.nn.utils.convert_parameters.vector_to_parameters(new_para, self.actor.parameters())

    def update(self, transition_dict):
        states = torch.tensor(transition_dict['states'], dtype=torch.float).to(self.device)
        actions = torch.tensor(transition_dict['actions']).view(-1, 1).to(self.device)
        rewards = torch.tensor(transition_dict['rewards'], dtype=torch.float).view(-1, 1).to(self.device)
        next_states = torch.tensor(transition_dict['next_states'], dtype=torch.float).to(self.device)
        dones = torch.tensor(transition_dict['dones'], dtype=torch.float).view(-1, 1).to(self.device)
        # 时序差分目标：r+γV(s')，如果是终止状态，目标值为r
        td_target = rewards + self.gamma * self.critic(next_states) * (1 - dones)
        td_delta = td_target - self.critic(states)
        advantage = self.compute_advantage(self.gamma, self.lmbda, td_delta.cpu()).to(self.device)
        old_log_probs = torch.log(self.actor(states).gather(1, actions)).detach()                                               
        old_action_dists = torch.distributions.Categorical(self.actor(states).detach())
        critic_loss = torch.mean(F.mse_loss(self.critic(states), td_target.detach()))
        self.critic_optimizer.zero_grad()
        critic_loss.backward()
        self.critic_optimizer.step()  # 更新价值函数
        # 更新策略函数
        self.policy_learn(states, actions, old_action_dists, old_log_probs, advantage)

# 超参数
num_episodes = 500
hidden_dim = 128
gamma = 0.98
lmbda = 0.95
critic_lr = 1e-2
kl_constraint = 0.0005
alpha = 0.5
device = torch.device("cuda") if torch.cuda.is_available() else torch.device("cpu")
# 车杆环境
env_name = 'CartPole-v0'
env = gym.make(env_name)
env.seed(0)
# env.reset(seed=0)
torch.manual_seed(0)
agent = TRPO(hidden_dim, env.observation_space, env.action_space, lmbda,
             kl_constraint, alpha, critic_lr, gamma, device)
return_list = rl_utils.train_on_policy_agent(env, agent, num_episodes)

episodes_list = list(range(len(return_list)))
plt.plot(episodes_list, return_list)
plt.xlabel('Episodes')
plt.ylabel('Returns')
plt.title('TRPO on {}'.format(env_name))
plt.show()

mv_return = rl_utils.moving_average(return_list, 9)
plt.plot(episodes_list, mv_return)
plt.xlabel('Episodes')
plt.ylabel('Returns')
plt.title('TRPO on {}'.format(env_name))
plt.show()