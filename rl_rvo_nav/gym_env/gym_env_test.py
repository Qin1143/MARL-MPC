import gym
import gym_env
from pathlib import Path

world_name = 'gym_test_world.yaml'
# world_name = 'dynamic_obs_test.yaml'

# init_mode: 0 manually initialize
#            1 single row
#            2 random
#            3 circular
### normal ###
env = gym.make('mrnav-v1', world_name=world_name, robot_number=3, robot_init_mode=0, random_bear=True)
### dyn_obs ###
# env = gym.make('mrnav-v1', world_name=world_name, robot_number=4, robot_init_mode=2, random_bear=True)
env.reset()

for i in range(300):

    vel_list = env.ir_gym.cal_des_omni_list()

    obs_list, reward_list, done_list, info_list = env.step_ir(vel_list, vel_type='omni')
    # print("obs_list: ", obs_list) # [Oself, Osur],Oself:cur_vel, des_vel, radian, radius, Osur:每一个机器人为一个8维度的向量
    id_list=[id for id, done in enumerate(done_list) if done==True]
    
    for id in id_list: 
        env.reset_one(id) 

    env.render()



