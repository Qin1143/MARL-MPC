#include "time.h"
#include "math.h"
#include "std_msgs/msg/u_int8.hpp"
#include "rclcpp/rclcpp.hpp"
#include "mpc_controller/mpc_controller.hpp"
#include "motor_interfaces/msg/motor.hpp"
#include "traj_interfaces/msg/bspline.hpp"
#include "action_interfaces/msg/action.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/quaternion.hpp"
#include "geometry_msgs/msg/pose.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "nav_msgs/msg/path.hpp"
#include "mpc_controller/uniform_bspline.h"
#include "record.cpp"
#include "deque"

#define PI 3.1415926
#define yaw_error_max 90.0/180*PI
#define N 15

const double HALF_DISTANCE_BETWEEN_WHEELS = 0.2230;
const double WHEEL_RADIUS = 0.0625;

MPC_controller mpc_controller;
RobotPoseRecorder recorder;
tf2::Quaternion quat;
motor_interfaces::msg::Motor motor_cmd;

clock_t start_clock, end_clock;
double duration;
double kx, ky, kth, interval;
std::string control_mode;

bool is_orientation_init = false;
bool display = false;
std::vector<rclcpp::TimerBase::SharedPtr> timers;
// 获取当前的年月日时分秒
std::time_t t = std::time(nullptr);
std::tm* now = std::localtime(&t);
int year = now->tm_year + 1900;
int month = now->tm_mon + 1;
int day = now->tm_mday;
int hour = now->tm_hour;
int minute = now->tm_min;
int second = now->tm_sec;
std::string file_time = std::to_string(year) + "_" + 
                        std::to_string(month) + "_" + 
                        std::to_string(day) + "_" + 
                        std::to_string(hour) + "_" + 
                        std::to_string(minute) + "_" + 
                        std::to_string(second);
// 数据保存标志位
bool data_saved = false;
bool save_data;

enum DIRECTION {POSITIVE=0,NEGATIVE=1};

class control_node : public rclcpp::Node
{
public:
    int robot_num;
    std::vector<std_msgs::msg::UInt8> stop_command;
    std::vector<bool> receive_mission;
    bool receive_action;
    double t_step = 0.03;

    control_node(std::string node_name) : Node(node_name)
    {        
        // 声明参数
        this->declare_parameter<int>("robot_num", 1);
        this->declare_parameter<double>("v_max", 1.8);
        this->declare_parameter<double>("w_max", 1.2);
        this->declare_parameter<double>("omega0", 1.0);
        this->declare_parameter<double>("omega1", 0.1);
        this->declare_parameter<double>("kx", 2.5);
        this->declare_parameter<double>("ky", 20.0);
        this->declare_parameter<double>("kth", 8.0);
        this->declare_parameter<double>("interval", 0.1);
        this->declare_parameter<std::string>("control_mode", "mpc");
        this->declare_parameter<bool>("save_data", false);

        // 打印robot_num参数
        this->get_parameter("robot_num", robot_num);
        std::cout << "########## robot_num: " << robot_num << " ##########" << std::endl;

        // 获取Backstepping参数
        this->get_parameter("kx", kx);
        this->get_parameter("ky", ky);
        this->get_parameter("kth", kth);

        // 获取字符串参数
        this->get_parameter("control_mode", control_mode);
        std::cout << "########## control_mode: " << control_mode << " ##########" << std::endl;

        // 获取数据保存标志位
        this->get_parameter("save_data", save_data);
        std::cout << "########## save_data: " << save_data << " ##########" << std::endl;

        this->get_parameter("interval", interval);

        stop_command.resize(robot_num);
        receive_mission.resize(robot_num);
        odom_pos.resize(robot_num);
        ref_pose.resize(robot_num);
        pre_pose.resize(robot_num);
        odom_vel.resize(robot_num);
        odom_orient.resize(robot_num);
        odom_orient.resize(robot_num);
        yaw.resize(robot_num);
        dir.resize(robot_num);
        traj.resize(robot_num);
        start_time.resize(robot_num);
        time_e.resize(robot_num);
        traj_duration.resize(robot_num);

        if (control_mode == "train")
        {
            sub_action_msg = this->create_subscription<action_interfaces::msg::Action>("/actions", 10, std::bind(&control_node::action_callback, this, std::placeholders::_1));
            receive_action = false;
            for (int i = 0; i < robot_num; ++i)
            {
                auto sub_robot_pose = this->create_subscription<geometry_msgs::msg::Pose>("/traj_pub/pose_" + std::to_string(i + 1), 10, [this, i](const geometry_msgs::msg::Pose::SharedPtr msg)
                                                                                         { this->poses_callback(msg, i); });
                sub_robot_poses.push_back(sub_robot_pose);
            }
        }
        else
        {
            for (int i = 0; i < robot_num; ++i)
            {
                auto sub_robot_dir = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/direction_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg)
                                                                                     { this->dir_callback(msg, i); });
                // this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/direction_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg) {this->dir_callback(msg, i);});
                sub_robot_dirs.push_back(sub_robot_dir);
                // std::cout << "sub_robot_dir:" << i+1 << std::endl;

                auto sub_robot_bspline = this->create_subscription<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj_" + std::to_string(i + 1), 10, [this, i](const traj_interfaces::msg::Bspline::SharedPtr msg)
                                                                                                  { this->traj_callback(msg, i); });
                // this->create_subscription<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj_" + std::to_string(i + 1), 10, [this, i](const traj_interfaces::msg::Bspline::SharedPtr msg) {this->traj_callback(msg, i);});
                sub_robot_bsplines.push_back(sub_robot_bspline);
                // std::cout << "sub_robot_bspline:" << i+1 << std::endl;

                auto sub_robot_stop = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/stop_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg)
                                                                                      { this->stop_callback(msg, i); });
                // this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/stop_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg) {this->stop_callback(msg, i);});
                sub_robot_stops.push_back(sub_robot_stop);
                // std::cout << "sub_robot_stop:" << i+1 << std::endl;
            }
        }

        for (int i = 0; i < robot_num; ++i)
        {
            auto pub_robot_cmd = this->create_publisher<motor_interfaces::msg::Motor>("/motor_cmd_" + std::to_string(i + 1), 10);
            pub_robot_cmds.push_back(pub_robot_cmd);
            // std::cout << "pub_robot_cmd:" << i+1 << std::endl;
            auto bspline_visualization = this->create_publisher<nav_msgs::msg::Path>("/bspline_visualization_" + std::to_string(i + 1), 10);
            bspline_visualizations.push_back(bspline_visualization);
            // std::cout << "bspline_visualizations:" << i+1 << std::endl;

            auto sub_robot_odom = this->create_subscription<nav_msgs::msg::Odometry>("/odom_" + std::to_string(i + 1), 10, [this, i](const nav_msgs::msg::Odometry::SharedPtr msg)
                                                                                     { this->odom_callback(msg, i); });
            // this->create_subscription<nav_msgs::msg::Odometry>("/odom_" + std::to_string(i + 1), 10, [this, i](const nav_msgs::msg::Odometry::SharedPtr msg) {this->odom_callback(msg, i);});
            sub_robot_odoms.push_back(sub_robot_odom);
            // std::cout << "sub_robot_odom:" << i+1 << std::endl;

            stop_command[i].data = 0; // 需要初始化向量大小
            receive_mission[i] = false;

            std::cout << "########初始化完成: " << i << "########" << std::endl;
        }
    }

    void cmdCallback(int robot_id)
    {   
        // std::cout << "Run to here! cmdCallback:" << robot_id << std::endl;
        /* no publishing before receive traj_ */
        if (stop_command[robot_id].data == 1)
        {
            motor_cmd.left_speed = 0;
            motor_cmd.right_speed = 0;
            pub_robot_cmds[robot_id]->publish(motor_cmd);
            if (!data_saved && save_data)
            {
                recorder.savePosesToFile(file_time + "_" + control_mode + "_poses.txt");
                data_saved = true;
            }
            return;
        }

        if (!receive_mission[robot_id])
            return;
        
        // RCLCPP_WARN(this->get_logger(), "Run here !");
        auto time_s = this->get_clock()->now();
        double t_cur = (time_s - start_time[robot_id]).seconds();
        static rclcpp::Time time_last = this->get_clock()->now();
        
        if (t_cur < traj_duration[robot_id] && t_cur >= 0.0)
        {
            // ROS_INFO("MPC_Calculate!");
            start_clock = clock();
            if (control_mode == "mpc")
            {
                // std::cout << "########## MPC ##########" << std::endl;
                MPC_calculate(t_cur, robot_id);
                publish_bspline_tracking_points(t_cur, robot_id);
            }
            else if (control_mode == "backstepping")
            {
                // std::cout << "########## Backstepping ##########" << std::endl;
                backstepping_calculate(t_cur, robot_id);
                publish_bspline_tracking_points(t_cur, robot_id);
            }

            end_clock = clock();
            duration = (double)(end_clock - start_clock) / CLOCKS_PER_SEC * 1000;
            // ROS_INFO("Control times : %f ms",duration);
        
        }
        else if (t_cur >= traj_duration[robot_id])
        {
            motor_cmd.left_speed = 0;
            motor_cmd.right_speed = 0;
            pub_robot_cmds[robot_id]->publish(motor_cmd);
            is_orientation_init = false;
            if (!data_saved && save_data)
            {
                recorder.savePosesToFile(file_time + "_" + control_mode + "_multi_poses.txt");
                data_saved = true;
            }
        }
        else
        {   
            RCLCPP_INFO(this->get_logger(), "[Traj server]: invalid time.");
        }
        time_last = time_s;

        // pub_robot_1_cmd->publish(motor_cmd);
    }

    void actionCallback(int robot_id)
    {
        if (!receive_action)
        {
            return;
        }
        else
        {
            train_control(robot_id);
        }
    }
private:
    std::vector<rclcpp::Publisher<motor_interfaces::msg::Motor>::SharedPtr> pub_robot_cmds;
    std::vector<rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr> bspline_visualizations;

    std::vector<rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr> sub_robot_odoms;
    std::vector<rclcpp::Subscription<geometry_msgs::msg::Pose>::SharedPtr> sub_robot_poses;
    std::vector<rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr> sub_robot_dirs;
    std::vector<rclcpp::Subscription<traj_interfaces::msg::Bspline>::SharedPtr> sub_robot_bsplines;
    std::vector<rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr> sub_robot_stops;
    rclcpp::Subscription<action_interfaces::msg::Action>::SharedPtr sub_action_msg;

    std::vector<Eigen::Vector3d> odom_pos, odom_vel, ref_pose, pre_pose;
    std::vector<Eigen::Quaterniond> odom_orient;
    std::vector<double> yaw;
    std::vector<std_msgs::msg::UInt8> dir;
    std::vector<vector<bspline_planner::UniformBspline>> traj;
    std::vector<rclcpp::Time> start_time, time_e;
    std::vector<double> traj_duration;
    std::vector<double> actions;
    std::vector<double> pre_actions;
    double yaw_ref, yaw_pre;
    // std::deque<std::vector<double>> action_queue;

    void poses_callback(const geometry_msgs::msg::Pose::SharedPtr msg, int robot_id)
    {
        if (!receive_action)
        {
            receive_action = true;
        }
        Eigen::Vector3d ref_pose_;
        pre_pose[robot_id] = ref_pose[robot_id];
        ref_pose_(0) = msg->position.x;
        ref_pose_(1) = msg->position.y;
        ref_pose_(2) = msg->orientation.w;
        ref_pose[robot_id] = (ref_pose_);
    }

    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg, int robot_id)
    {   
        Eigen::Vector3d odom_pos_, odom_vel_;
        Eigen::Quaterniond odom_orient_;
        double roll_, pitch_, yaw_;

        // 读取当前状态
        odom_pos_(0) = msg->pose.pose.position.x;
        odom_pos_(1) = msg->pose.pose.position.y;
        odom_pos_(2) = msg->pose.pose.position.z;

        odom_vel_(0) = msg->twist.twist.linear.x;
        odom_vel_(1) = msg->twist.twist.linear.y;
        odom_vel_(2) = msg->twist.twist.linear.z;

        odom_orient_.w() = msg->pose.pose.orientation.w;
        odom_orient_.x() = msg->pose.pose.orientation.x;
        odom_orient_.y() = msg->pose.pose.orientation.y;
        odom_orient_.z() = msg->pose.pose.orientation.z;

        odom_pos[robot_id] = (odom_pos_);
        odom_vel[robot_id] = (odom_vel_);
        odom_orient[robot_id] = (odom_orient_);

        tf2::fromMsg(msg->pose.pose.orientation, quat);
        tf2::Matrix3x3(quat).getRPY(roll_, pitch_, yaw_);

        if (dir[robot_id].data == NEGATIVE)
        {
            if (yaw_ > 0)
            {
                yaw_ -= PI;
            }
            else if (yaw_ < 0)
            {
                yaw_ += PI;
            }
        }

        yaw[robot_id] = yaw_;
    }

    void dir_callback(const std_msgs::msg::UInt8::SharedPtr msg, int robot_id)
    {
        dir[robot_id] = *msg;
    }

    void stop_callback(const std_msgs::msg::UInt8::SharedPtr msg, int robot_id)
    {
        stop_command[robot_id] = *msg;
    }

    void traj_callback(const traj_interfaces::msg::Bspline::SharedPtr msg, int robot_id)
    {   
        if (receive_mission[robot_id] == true)
        {
            return;
        }

        Eigen::MatrixXd pos_pts(3, msg->pos_pts.size());

        Eigen::VectorXd knots(msg->knots.size());

        vector<bspline_planner::UniformBspline> traj_;

        for (size_t i = 0; i < msg->knots.size(); ++i)
        {
            knots(i) = msg->knots[i];
        }

        for (size_t i = 0; i < msg->pos_pts.size(); ++i)
        {
            pos_pts(0, i) = msg->pos_pts[i].x;
            pos_pts(1, i) = msg->pos_pts[i].y;
            pos_pts(2, i) = msg->pos_pts[i].z;
        }

        bspline_planner::UniformBspline pos_traj(pos_pts, msg->order, interval);
        pos_traj.setKnot(knots);

        start_time[robot_id] = msg->start_time;
        // traj_id_ = msg->traj_id;

        traj_.clear();
        traj_.push_back(pos_traj);
        traj_.push_back(traj_[0].getDerivative());
        traj_.push_back(traj_[1].getDerivative());

        traj_duration[robot_id] = traj_[0].getTimeSum();
        std::cout << "########## traj_duration: " << traj_duration[robot_id] << " ##########" << std::endl;

        receive_mission[robot_id] = true;
        traj[robot_id] = traj_;
    }

    void action_callback(const action_interfaces::msg::Action::SharedPtr msg)
    {
        if (!receive_action)
        {
            receive_action = true;
        }
        pre_actions = actions;
        actions = msg->action;
    }

    void train_control(int &robot_id)
    {   
        // 这里是纯网络控制
        // int index = robot_id*2;
        // double vx = actions[index];
        // double vy = actions[index+1];
        // double linear_v = sqrt(vx * vx + vy * vy);
        // auto cur_angle = atan2(vy, vx);
        // double angular_v = 0;
        // if (pre_actions.size() > 0)
        // {
        //     double pre_vx = pre_actions[index];
        //     double pre_vy = pre_actions[index+1];
        //     double angle = acos((vx * pre_vx + vy * pre_vy) / (sqrt(vx * vx + vy * vy) * sqrt(pre_vx * pre_vx + pre_vy * pre_vy)));
        //     std::cout << "angle: " << angle << std::endl;
        //     angular_v = angle / t_step;
        //     // std::cout << "cur_angle: " << cur_angle << " pre_angle: " << pre_angle << std::endl;
        // }
        // // std::cout << "linear_v: " << linear_v << " angular_v: " << angular_v << std::endl;

        // 加入反步控制
        Eigen::Matrix<float, 3, 3> T;
        Eigen::Matrix<float, 3, 1> Eg; // Error of global 全局坐标系下的误差
        Eigen::Matrix<float, 3, 1> Eb; // Error of body 车体坐标系下的误差
        Eigen::Vector3d pos_ref_1, pos_pre_1; // 期望位置
        double Vr, Wr, linear_v, angular_v;

        pos_ref_1 = ref_pose[robot_id]; // 期望位置
        pos_pre_1 = pre_pose[robot_id]; // 上一时刻位置
        auto dis = sqrt(pow(pos_ref_1(0) - pos_pre_1(0), 2) + pow(pos_ref_1(1) - pos_pre_1(1), 2));
        T << 1, 0, -sin(yaw[robot_id]),
            0, 1, cos(yaw[robot_id]),
            0, 0, 1;
        yaw_pre = yaw_ref;
        yaw_ref = atan2(pos_ref_1(1)-pos_pre_1(1), pos_ref_1(0)-pos_pre_1(0));       // 初始航向角

        Vr = dis / 0.1;
        Wr = (yaw_ref - yaw_pre) / 0.1;
        

        if (yaw[robot_id]*yaw_ref >= 0)
        {
            Eg << pos_ref_1(0) - odom_pos[robot_id](0), pos_ref_1(1) - odom_pos[robot_id](1), yaw_ref - yaw[robot_id];
        }
        else
        {
            if (yaw[robot_id] > 0)
            {
                Eg << pos_ref_1(0) - odom_pos[robot_id](0), pos_ref_1(1) - odom_pos[robot_id](1), yaw_ref - yaw[robot_id] - 2 * PI;
            }
            else
            {
                Eg << pos_ref_1(0) - odom_pos[robot_id](0), pos_ref_1(1) - odom_pos[robot_id](1), yaw_ref - yaw[robot_id] + 2 * PI;
            }
        }

        // recorder.recordPose(robot_num, robot_id, pos_ref_1(0), pos_ref_1(1), yaw_ref, odom_pos[robot_id](0), odom_pos[robot_id](1), yaw[robot_id], t_cur);

        Eb = T * Eg;
        linear_v = Vr * cos(Eb[2]) + kx * Eb[0];
        angular_v = Wr + ky * Eb[1] + kth * sin(Eb[2]);

        motor_cmd.left_speed = (linear_v - angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;
        motor_cmd.right_speed = (linear_v + angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;

        pub_robot_cmds[robot_id]->publish(motor_cmd);
    }

    void publish_bspline_tracking_points(double &t_cur, int robot_id)
    {
        nav_msgs::msg::Path path_msg;
        path_msg.header.stamp = this->get_clock()->now();
        path_msg.header.frame_id = "map";

        for (double t = t_cur; t <= traj_duration[robot_id]; t += 0.1)
        {
            Eigen::Vector3d pos = traj[robot_id][0].evaluateDeBoor(t);
            geometry_msgs::msg::PoseStamped pose;
            pose.pose.position.x = pos(0);
            pose.pose.position.y = pos(1);
            pose.pose.position.z = pos(2);
            path_msg.poses.push_back(pose);
        }

        bspline_visualizations[robot_id]->publish(path_msg);
    }

    void MPC_calculate(double &t_cur, int &robot_id)
    {
        std::vector<Eigen::Vector3d> X_r; // 储存参考轨迹
        std::vector<Eigen::Vector2d> U_r; // 储存参考控制输入
        Eigen::MatrixXd u_k;
        Eigen::Vector3d pos_r, pos_r_1, pos_final, v_r_1, v_r_2, X_k;
        Eigen::Vector2d u_r;
        Eigen::Vector3d x_r, x_r_1, x_r_2;
        double v_linear_1, w;
        double t_k, t_k_1;
        double linear_v, angular_v;

        // ROS_INFO("Run to here!");

        // Eigen::Vector3d pos_first = traj_[0].evaluateDeBoor(t_cur);
        // Eigen::Vector3d pos_second = traj_[0].evaluateDeBoor(t_cur+t_step);
        Eigen::Vector3d vel_start = traj[robot_id][1].evaluateDeBoor(t_cur); // 初始速度
        double yaw_start = atan2(vel_start(1), vel_start(0));       // 初始航向角
        bool is_orientation_adjust = false;
        double orientation_adjust = 0;
        pos_final = traj[robot_id][0].evaluateDeBoor(traj_duration[robot_id]);

        is_orientation_init = true;
        for (int i = 0; i < N; i++)
        {

            t_k = t_cur + i * t_step;
            t_k_1 = t_cur + (i + 1) * t_step;

            pos_r = traj[robot_id][0].evaluateDeBoor(t_k);
            pos_r_1 = traj[robot_id][0].evaluateDeBoor(t_k_1);

            x_r(0) = pos_r(0);
            x_r(1) = pos_r(1);

            v_r_1 = traj[robot_id][1].evaluateDeBoor(t_k);
            v_r_2 = traj[robot_id][1].evaluateDeBoor(t_k_1);
            v_r_1(2) = 0;
            v_r_2(2) = 0;
            v_linear_1 = v_r_1.norm();
            if ((t_k - traj_duration[robot_id]) >= 0)
            {
                x_r(2) = atan2((pos_r - pos_final)(1), (pos_r - pos_final)(0));
            }
            else
            {
                // x_r(2) = atan2((pos_r_1-pos_r)(1),(pos_r_1-pos_r)(0));
                x_r(2) = atan2(v_r_1(1), v_r_1(0));
            }

            // 判断并调整航向角
            double yaw1 = atan2(v_r_1(1), v_r_1(0));
            double yaw2 = atan2(v_r_2(1), v_r_2(0));

            if (abs(yaw2 - yaw1) > PI)
            {
                // RCLCPP_WARN(this->get_logger(), "orientation suddenly change !");
                // cout<<"current index : "<<i+1<<endl;
                // cout<<"yaw 1 : "<<yaw1<<endl;
                // cout<<"yaw 2 : "<<yaw2<<endl;
                is_orientation_adjust = true;
                if ((yaw2 - yaw1) < 0)
                {
                    orientation_adjust = 2 * PI;
                    w = (2 * PI + (yaw2 - yaw1)) / t_step;
                }
                else
                {
                    w = ((yaw2 - yaw1) - 2 * PI) / t_step;
                    orientation_adjust = -2 * PI;
                }
            }
            else
            {
                w = (yaw2 - yaw1) / t_step;
            }

            if (is_orientation_adjust == true)
            {
                // cout<<"orientation before adjust : "<< x_r(2)<<endl;
                x_r(2) += orientation_adjust;
                // cout<<"orientation after adjust : "<< x_r(2)<<endl;
            }

            // 存储参考轨迹和控制输入
            u_r(0) = v_linear_1;
            u_r(1) = w;

            X_r.push_back(x_r);
            U_r.push_back(u_r);
        }

        // X_k(0) = odom_map.pose.pose.position.x - my_map.info.origin.position.x;
        // X_k(1) = odom_map.pose.pose.position.y - my_map.info.origin.position.y;
        //  计算当前位置
        X_k(0) = odom_pos[robot_id](0);
        X_k(1) = odom_pos[robot_id](1);
        // 调整当前航向角
        if (yaw[robot_id] / X_r[0](2) < 0 && abs(yaw[robot_id]) > (PI * 5 / 6))
        {
            if (yaw[robot_id] < 0)
            {
                X_k(2) = yaw[robot_id] + 2 * PI;
            }
            else
            {
                X_k(2) = yaw[robot_id] - 2 * PI;
            }
        }
        else
        {
            X_k(2) = yaw[robot_id];
        }
        // cout<<"xr  : "<<X_r[0]<<endl;
        // cout<<"xk  : "<<X_k<<endl;
        recorder.recordPose(robot_num, robot_id, X_r[0](0), X_r[0](1), X_r[0](2), X_k(0), X_k(1), X_k(2), t_cur);

        // ROS_INFO("Run to here!");
        //  使用MPC求解器计算最优控制输入
        u_k = mpc_controller.MPC_Solve_qp(X_k, X_r, U_r, N); // 当前状态，参考状态，参考输入，预测步数

        if (dir[robot_id].data == NEGATIVE)
        {
            linear_v = -u_k.col(0)(0);
        }
        else
        {
            linear_v = u_k.col(0)(0);
        }

        angular_v = u_k.col(0)(1);

        motor_cmd.left_speed = (linear_v - angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;
        motor_cmd.right_speed = (linear_v + angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;
        if (display == true)
        {
            static int conut1 = 0;
            conut1 += 1;
            if (conut1 % 20 == 0)
            {
                RCLCPP_WARN(this->get_logger(), "U r :");
                for (int i = 0; i < U_r.size(); i++)
                {
                    cout << "vel ref :" << U_r[i](0) << "," << "w ref : " << U_r[i](1);
                    cout << endl;
                }
                cout << endl;
                RCLCPP_WARN(this->get_logger(), "U k :");
                for (int i = 0; i < u_k.cols(); i++)
                {
                    cout << "vel optimal :" << u_k.col(i)(0) << "," << "w optimal : " << u_k.col(i)(1);
                    cout << endl;
                }
                cout << endl;
                cout << "current vel : : " << u_k.col(0)(0) << "m/s" << endl;
                cout << "current w : " << u_k.col(0)(1) << "rad/s" << endl;
                conut1 = 0;
            }
        }


        pub_robot_cmds[robot_id]->publish(motor_cmd);
    }

    void backstepping_calculate(double &t_cur, int &robot_id)
    {
        Eigen::Matrix<float, 3, 3> T;
        Eigen::Matrix<float, 3, 1> Eg; // Error of global 全局坐标系下的误差
        Eigen::Matrix<float, 3, 1> Eb; // Error of body 车体坐标系下的误差
        Eigen::Vector3d pos_ref_1; // 期望位置
        Eigen::Vector3d vel_ref_1, vel_ref_2; // 初始速度
        double Vr, Wr, linear_v, angular_v;
        double orientation_adjust = 0;
        bool is_orientation_adjust = false;

        pos_ref_1 = traj[robot_id][0].evaluateDeBoor(t_cur); // 期望位置
        vel_ref_1 = traj[robot_id][1].evaluateDeBoor(t_cur); // 初始速度
        T << 1, 0, -sin(yaw[robot_id]),
            0, 1, cos(yaw[robot_id]),
            0, 0, 1;

        double yaw_ref = atan2(vel_ref_1(1), vel_ref_1(0));       // 初始航向角

        // 计算期望角速度
        vel_ref_2 = traj[robot_id][1].evaluateDeBoor(t_cur + t_step);

        // 判断并调整航向角
        double yaw1 = atan2(vel_ref_1(1), vel_ref_1(0));
        double yaw2 = atan2(vel_ref_2(1), vel_ref_2(0));

        if (abs(yaw2 - yaw1) > PI)
        {
            is_orientation_adjust = true;
            if ((yaw2 - yaw1) < 0)
            {
                orientation_adjust = 2 * PI;
                Wr = (2 * PI + (yaw2 - yaw1)) / t_step;
            }
            else
            {
                Wr = ((yaw2 - yaw1) - 2 * PI) / t_step;
                orientation_adjust = -2 * PI;
            }
        }
        else
        {
            Wr = (yaw2 - yaw1) / t_step;
        }

        if (is_orientation_adjust == true)
        {
            pos_ref_1(2) += orientation_adjust;
        }

        if (yaw[robot_id]*yaw_ref >= 0)
        {
            Eg << pos_ref_1(0) - odom_pos[robot_id](0), pos_ref_1(1) - odom_pos[robot_id](1), yaw_ref - yaw[robot_id];
        }
        else
        {
            if (yaw[robot_id] > 0)
            {
                Eg << pos_ref_1(0) - odom_pos[robot_id](0), pos_ref_1(1) - odom_pos[robot_id](1), yaw_ref - yaw[robot_id] - 2 * PI;
            }
            else
            {
                Eg << pos_ref_1(0) - odom_pos[robot_id](0), pos_ref_1(1) - odom_pos[robot_id](1), yaw_ref - yaw[robot_id] + 2 * PI;
            }
        }

        recorder.recordPose(robot_num, robot_id, pos_ref_1(0), pos_ref_1(1), yaw_ref, odom_pos[robot_id](0), odom_pos[robot_id](1), yaw[robot_id], t_cur);

        Eb = T * Eg;
        Vr = sqrt(pow(vel_ref_1(0), 2) + pow(vel_ref_1(1), 2));
        linear_v = Vr * cos(Eb[2]) + kx * Eb[0];
        angular_v = Wr + ky * Eb[1] + kth * sin(Eb[2]);

        motor_cmd.left_speed = (linear_v - angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;
        motor_cmd.right_speed = (linear_v + angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;

        pub_robot_cmds[robot_id]->publish(motor_cmd);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<control_node>("multi_control_node");
    mpc_controller.MPC_init(node);
    for(int i = 0 ; i < node->robot_num; ++i)
    {
        auto timer_callback = [node, i](/* rclcpp::TimerBase::SharedPtr timer */)
        {   
            if (control_mode == "train")
            {
                node->actionCallback(i);
            }
            else
            {
                node->cmdCallback(i);
            }
        };
        auto timer = node->create_wall_timer(std::chrono::milliseconds(30), timer_callback);
        timers.push_back(timer);
    }

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}