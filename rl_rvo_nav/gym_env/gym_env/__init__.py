from gym.envs.registration import register
# 注册一个新的自定义环境：mrnav-v1
register(
    # id: 环境的唯一标识符
    id='mrnav-v1',
    # entry_point: 环境的路径
    entry_point='gym_env.envs:mrnav',
)
