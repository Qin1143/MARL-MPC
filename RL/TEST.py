import numpy as np
import torch

list1 = [1, 2, 3, 4, 5]
list2 = [1, 2, 3, 4, 5]
list3 = [list1, list2]
state, action, reward, next_state, done = zip(list1, list2)
state, action, reward, next_state, done = zip(*list3)

print(state, action, reward, next_state, done)

# Create a tensor
tensor = torch.tensor([list1, list2])

# Print the tensor
print(tensor)

t = torch.tensor([[1, 2], [3, 4]])
f = torch.tensor([[0, 1], [1, 0]])
# 从第0维度（行）中选择第0个元素和第1个元素，从第1维度（列）中选择第1个元素和第0个元素
t_ = torch.gather(t, 1, f)
print(t_)

import gym
from torch.distributions import Categorical
print(gym.__version__)

# Import the necessary libraries

# Define the probabilities
probs = torch.tensor([0.1, 0.3, 0.6])

# Create a Categorical distribution
dist = Categorical(probs)

# Sample an action from the distribution
action = dist.sample()

# Get the log probability of the sampled action
log_prob = dist.log_prob(action) # 给定动作的对数概率

# Print the sampled action and its log probability
print("Sampled Action:", action.item())
print("Log Probability:", log_prob.item())
