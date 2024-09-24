import matplotlib.pyplot as plt

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
    for key in reference_trajectories.keys():
        fig, axs = plt.subplots(3, 1, figsize=(8, 12))  # 创建一个具有三个子图的大图

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
    

if __name__ == "__main__":
    # file_path = '/home/tony/webots_ws/2024_9_24_15_48_38_poses.txt' # 单车数据
    file_path = '/home/tony/webots_ws/2024_9_24_16_2_13_multi_poses.txt' # 多车数据
    reference_trajectories, actual_trajectories, timestamp = read_poses(file_path)
    plot_trajectories_xy(reference_trajectories, actual_trajectories, timestamp)
    plot_x_y_yaw(reference_trajectories, actual_trajectories, timestamp)