import matplotlib.pyplot as plt
import matplotlib.cm as cm


all_data_valid: bool = True

def read_poses(file_path):
    reference_trajectories = dict()
    actual_trajectories = dict()
    timestamp = dict()
    
    with open(file_path, 'r') as file:
        for line in file:
            data = line.strip().split()
            data = [float(item) for item in data] # Convert all items to float
            robot_id = int(data[0])
            if robot_id not in reference_trajectories:
                reference_trajectories[robot_id] = []
                actual_trajectories[robot_id] = []
                timestamp[robot_id] = []
            xr, yr, yawr = data[1], data[2], data[3]
            x, y, yaw, time = data[4], data[5], data[6], data[7]
            reference_trajectories[robot_id].append((xr, yr, yawr))
            actual_trajectories[robot_id].append((x, y, yaw))
            timestamp[robot_id].append(time)
    return reference_trajectories, actual_trajectories, timestamp

def read_trained_poses(file_path):
    reference_trajectories = dict()
    timestamp = dict()
    
    with open(file_path, 'r') as file:
        for line in file:
            data = line.strip().split()
            data = [float(item) for item in data] # Convert all items to float
            robot_id = int(data[0])
            if robot_id not in reference_trajectories:
                reference_trajectories[robot_id] = []
                timestamp[robot_id] = []
            xr, yr, yawr, time = data[1], data[2], data[3], data[4]
            reference_trajectories[robot_id].append((xr, yr, yawr))
            timestamp[robot_id].append(time)
    return reference_trajectories, timestamp

def plot_trained_traj_xy(reference_trajectories, timestamp):
    plt.figure()
    # 获取机器人数量
    num_robots = len(reference_trajectories.keys())
    # 生成颜色列表
    colors = cm.rainbow([i / num_robots for i in range(num_robots)])
    for color_index, key in enumerate(reference_trajectories.keys()):
        color = colors[color_index]
        ref_x, ref_y, ref_yaw = zip(*reference_trajectories[key])
        title = 'Trajectories of ' + str(key+1) + ' Robots'
        plt.plot(ref_x, ref_y, label=f'Ref Traj {key+1}', color=color, linewidth=2)
        plt.xlabel('X')
        plt.ylabel('Y')
        plt.title(title)
        plt.legend()
        plt.grid(True)
    plt.show()

def plot_trained_x_y_yaw(reference_trajectories, timestamp):
    num_robots = len(reference_trajectories.keys())
    colors = cm.rainbow([i / num_robots for i in range(num_robots)])
    for color_index, key in enumerate(reference_trajectories.keys()):
        fig, axs = plt.subplots(3, 1, figsize=(8, 12))
        ref_x, ref_y, ref_yaw = zip(*reference_trajectories[key])
        color = colors[color_index]
        # 绘制 x 曲线
        axs[0].plot(timestamp[key], ref_x, label=f'Ref X (Robot:{key+1})', color=color, linestyle='-', linewidth=2)
        axs[0].set_xlabel('Time')
        axs[0].set_ylabel('X')
        axs[0].set_title('X Trajectory')
        axs[0].legend()
        axs[0].grid(True)
        
        # 绘制 y 曲线
        axs[1].plot(timestamp[key], ref_y, label=f'Ref Y (Robot:{key+1})', color=color, linestyle='-', linewidth=2)
        axs[1].set_xlabel('Time')
        axs[1].set_ylabel('Y')
        axs[1].set_title('Y Trajectory')
        axs[1].legend()
        axs[1].grid(True)
        
        # 绘制 yaw 曲线
        axs[2].plot(timestamp[key], ref_yaw, label=f'Ref Yaw (Robot:{key+1})', color=color, linestyle='-', linewidth=2)
        axs[2].set_xlabel('Time')
        axs[2].set_ylabel('Yaw')
        axs[2].set_title('Yaw Trajectory')
        axs[2].legend()
        axs[2].grid(True)
    
        plt.tight_layout()  # 调整子图间距
    plt.show()


def plot_trajectories_xy(reference_trajectories, actual_trajectories, timestamp):
    for key in reference_trajectories.keys():
        ref_x, ref_y, ref_yaw = zip(*reference_trajectories[key])
        act_x, act_y, act_yaw = zip(*actual_trajectories[key])
        title = 'Trajectories of Robot ' + str(key)
        
        plt.figure()
        plt.plot(ref_x, ref_y, label='Reference Trajectory', color='blue')
        plt.plot(act_x, act_y, label='Actual Trajectory', color='red')
        plt.xlabel('X')
        plt.ylabel('Y')
        plt.title(title)
        plt.legend()
        plt.grid(True)
    plt.show()

def plot_x_y_yaw(reference_trajectories, actual_trajectories, timestamp):
    robot_num = len(reference_trajectories.keys())
    for key in reference_trajectories.keys():
        fig, axs = plt.subplots(robot_num, 1, figsize=(8, 12))  # 创建一个具有子图的大图

        ref_x, ref_y, ref_yaw = zip(*reference_trajectories[key])
        act_x, act_y, act_yaw = zip(*actual_trajectories[key])

        # 绘制 x 曲线
        axs[0].plot(timestamp[key], ref_x, label=f'Reference X (Robot:{key})', color='blue', linestyle='-')
        axs[0].plot(timestamp[key], act_x, label=f'Actual X (Robot:{key})', color='red', linestyle='-')
        axs[0].set_xlabel('Time')
        axs[0].set_ylabel('X')
        axs[0].set_title('X Trajectory')
        axs[0].legend()
        axs[0].grid(True)
        
        # 绘制 y 曲线
        axs[1].plot(timestamp[key], ref_y, label=f'Reference Y (Robot:{key})', color='blue', linestyle='-')
        axs[1].plot(timestamp[key], act_y, label=f'Actual Y (Robot:{key})', color='red', linestyle='-')
        axs[1].set_xlabel('Time')
        axs[1].set_ylabel('Y')
        axs[1].set_title('Y Trajectory')
        axs[1].legend()
        axs[1].grid(True)
        
        # 绘制 yaw 曲线
        axs[2].plot(timestamp[key], ref_yaw, label=f'Reference Yaw (Robot:{key})', color='blue', linestyle='-')
        axs[2].plot(timestamp[key], act_yaw, label=f'Actual Yaw (Robot:{key})', color='red', linestyle='-')
        axs[2].set_xlabel('Time')
        axs[2].set_ylabel('Yaw')
        axs[2].set_title('Yaw Trajectory')
        axs[2].legend()
        axs[2].grid(True)
    
        plt.tight_layout()  # 调整子图间距
    plt.show()

def plot_in_one_xyfigure(reference_trajectories, actual_trajectories, timestamp):
    plt.figure()
    # 获取机器人数量
    num_robots = len(reference_trajectories.keys())
    
    # 生成颜色列表
    colors = cm.rainbow([i / num_robots for i in range(num_robots)])
    
    for color_index, key in enumerate(reference_trajectories.keys()):
        # * 操作符用于解包列表，将列表中的每个元组作为单独的参数传递给 zip 函数
        # zip 函数将多个元组的对应元素组合在一起，形成新的元组
        ref_x, ref_y, ref_yaw = zip(*reference_trajectories[key])
        act_x, act_y, act_yaw = zip(*actual_trajectories[key])

        label1 = f'Ref Traj (Robot:{key})'
        label2 = f'Act Traj (Robot:{key})'

        # 获取当前机器人的颜色
        color = colors[color_index]

        plt.plot(ref_x, ref_y, label=label1, color=color, alpha=0.8, linestyle='-.')  # 参考轨迹透明度为0.5
        plt.plot(act_x, act_y, label=label2, color=color, alpha=1.0)  # 实际轨迹透明度为1.0
        plt.xlabel('X')
        plt.ylabel('Y')
        plt.title('Trajectories of All Robots')
        plt.legend()
        plt.grid(True)

        # 清空数据
        ref_x, ref_y, ref_yaw = None, None, None
        act_x, act_y, act_yaw = None, None, None
    plt.show()
    

if __name__ == "__main__":
    # file_path = '/home/tony/webots_ws/2024_9_24_15_48_38_poses.txt' # 单车数据
    file_path = '/home/tony/webots_ws/2024_10_9_15_20_6_backstepping_poses.txt' # 多车数据
    file_path = '/home/tony/webots_ws/robot_state_list.txt' # 训练数据
    # reference_trajectories, actual_trajectories, timestamp = read_poses(file_path)
    reference_trajectories, timestamp = read_trained_poses(file_path)
    plot_trained_traj_xy(reference_trajectories, timestamp)
    plot_trained_x_y_yaw(reference_trajectories, timestamp)
    # if all_data_valid:
    #     plot_in_one_xyfigure(reference_trajectories, actual_trajectories, timestamp)
    # else:
    #     plot_trajectories_xy(reference_trajectories, actual_trajectories, timestamp)
    #     plot_x_y_yaw(reference_trajectories, actual_trajectories, timestamp)