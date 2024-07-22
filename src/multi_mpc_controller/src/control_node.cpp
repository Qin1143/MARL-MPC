#include "time.h"
#include "math.h"
#include "std_msgs/msg/u_int8.hpp"
#include "rclcpp/rclcpp.hpp"
#include "mpc_controller/mpc_controller.hpp"
#include "motor_interfaces/msg/motor.hpp"
#include "traj_interfaces/msg/bspline.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/quaternion.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "nav_msgs/msg/path.hpp"
#include "mpc_controller/uniform_bspline.h"

#define PI 3.1415926
#define yaw_error_max 90.0/180*PI
#define N 15

const double HALF_DISTANCE_BETWEEN_WHEELS = 0.2230;
const double WHEEL_RADIUS = 0.0625;

MPC_controller mpc_controller;
// Eigen::Vector3d odom_pos_, odom_vel_;
// Eigen::Quaterniond odom_orient_;
tf2::Quaternion quat;
// std_msgs::msg::UInt8 dir;
// std_msgs::msg::UInt8 stop_command;
motor_interfaces::msg::Motor motor_cmd;

// rclcpp::Time start_time_, time_s, time_e;
clock_t start_clock, end_clock;
double duration;
// double traj_duration_;
// double t_step;

// int traj_id_;
// double roll, pitch, yaw;
// vector<bspline_planner::UniformBspline> traj_;
// bool receive_traj_ = false;
bool is_orientation_init = false;
std::vector<rclcpp::TimerBase::SharedPtr> timers;

enum DIRECTION {POSITIVE=0,NEGATIVE=1};

class control_node : public rclcpp::Node
{
public:
    const int robot_num = 3;
    std::vector<std_msgs::msg::UInt8> stop_command;
    std::vector<bool> receive_traj;
    double t_step = 0.03;

    control_node(std::string node_name) : Node(node_name)
    {        
        // 声明参数
        this->declare_parameter("robot_num", 1);
        this->declare_parameter("v_max", 1.8);
        this->declare_parameter("w_max", 1.2);
        this->declare_parameter("omega0", 1.0);
        this->declare_parameter("omega1", 0.1);
        
        stop_command.resize(robot_num);
        receive_traj.resize(robot_num);
        odom_pos.resize(robot_num);
        odom_vel.resize(robot_num);
        odom_orient.resize(robot_num);
        odom_orient.resize(robot_num);
        yaw.resize(robot_num);
        dir.resize(robot_num);
        traj.resize(robot_num);
        start_time.resize(robot_num);
        time_e.resize(robot_num);
        traj_duration.resize(robot_num);

        // 打印robot_num参数
        // this->get_parameter("robot_num", robot_num);
        std::cout << "robot_num: " << robot_num << std::endl;
        for (int i = 0; i < robot_num; ++i)
        {
            auto pub_robot_cmd = this->create_publisher<motor_interfaces::msg::Motor>("/motor_cmd_" + std::to_string(i + 1), 10);
            pub_robot_cmds.push_back(pub_robot_cmd);
            // std::cout << "pub_robot_cmd:" << i+1 << std::endl;
            auto bspline_visualization = this->create_publisher<nav_msgs::msg::Path>("/bspline_visualization_" + std::to_string(i + 1), 10);
            bspline_visualizations.push_back(bspline_visualization);
            // std::cout << "bspline_visualizations:" << i+1 << std::endl;

            auto sub_robot_odom = this->create_subscription<nav_msgs::msg::Odometry>("/odom_" + std::to_string(i + 1), 10, [this, i](const nav_msgs::msg::Odometry::SharedPtr msg) {this->odom_callback(msg, i);});
            // this->create_subscription<nav_msgs::msg::Odometry>("/odom_" + std::to_string(i + 1), 10, [this, i](const nav_msgs::msg::Odometry::SharedPtr msg) {this->odom_callback(msg, i);});
            sub_robot_odoms.push_back(sub_robot_odom);
            // std::cout << "sub_robot_odom:" << i+1 << std::endl;

            auto sub_robot_dir = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/direction_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg) {this->dir_callback(msg, i);});
            // this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/direction_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg) {this->dir_callback(msg, i);});
            sub_robot_dirs.push_back(sub_robot_dir);
            // std::cout << "sub_robot_dir:" << i+1 << std::endl;

            auto sub_robot_bspline = this->create_subscription<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj_" + std::to_string(i + 1), 10, [this, i](const traj_interfaces::msg::Bspline::SharedPtr msg) {this->traj_callback(msg, i);});
            // this->create_subscription<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj_" + std::to_string(i + 1), 10, [this, i](const traj_interfaces::msg::Bspline::SharedPtr msg) {this->traj_callback(msg, i);});            
            sub_robot_bsplines.push_back(sub_robot_bspline);
            // std::cout << "sub_robot_bspline:" << i+1 << std::endl;

            auto sub_robot_stop = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/stop_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg) {this->stop_callback(msg, i);});
            // this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/stop_" + std::to_string(i + 1), 10, [this, i](const std_msgs::msg::UInt8::SharedPtr msg) {this->stop_callback(msg, i);});
            sub_robot_stops.push_back(sub_robot_stop);
            // std::cout << "sub_robot_stop:" << i+1 << std::endl;

            stop_command[i].data = 0; // 需要初始化向量大小
            receive_traj[i] = false;

            std::cout << "########初始化完成: " << i << "########" << std::endl;
        }

        // pub_robot_1_cmd = this->create_publisher<motor_interfaces::msg::Motor>("/mir_robot_1/motor_cmd", 10);
        // pub_robot_2_cmd = this->create_publisher<motor_interfaces::msg::Motor>("/mir_robot_2/motor_cmd", 10);
        // pub_robot_3_cmd = this->create_publisher<motor_interfaces::msg::Motor>("/mir_robot_3/motor_cmd", 10);

        // bspline_visualization_ = this->create_publisher<nav_msgs::msg::Path>("/bspline_visualization", 10);
        // sub_odom_msg = this->create_subscription<nav_msgs::msg::Odometry>("/mir_robot_1/odom", 10, std::bind(&control_node::odom_callback, this, std::placeholders::_1));
        // sub_dir_msg = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/direction_1", 10, std::bind(&control_node::dir_callback, this, std::placeholders::_1));
        // sub_stop_msg = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/stop", 10, std::bind(&control_node::stop_callback, this, std::placeholders::_1));
        // sub_bspline_msg = this->create_subscription<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj_1", 10, std::bind(&control_node::traj_callback, this, std::placeholders::_1));
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
            return;
        }

        if (!receive_traj[robot_id])
            return;
        
        // RCLCPP_WARN(this->get_logger(), "Run here !");
        auto time_s = this->get_clock()->now();
        double t_cur = (time_s - start_time[robot_id]).seconds();
        static rclcpp::Time time_last = this->get_clock()->now();

        if (t_cur < traj_duration[robot_id] && t_cur >= 0.0)
        {
            // ROS_INFO("MPC_Calculate!");
            start_clock = clock();
            MPC_calculate(t_cur, robot_id);
            publish_bspline_tracking_points(t_cur, robot_id);
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
        }
        else
        {   
            RCLCPP_INFO(this->get_logger(), "[Traj server]: invalid time.");
        }
        time_last = time_s;

        // pub_robot_1_cmd->publish(motor_cmd);
    }
private:
    // rclcpp::Publisher<motor_interfaces::msg::Motor>::SharedPtr pub_robot_1_cmd;
    // rclcpp::Publisher<motor_interfaces::msg::Motor>::SharedPtr pub_robot_2_cmd;
    // rclcpp::Publisher<motor_interfaces::msg::Motor>::SharedPtr pub_robot_3_cmd;

    std::vector<rclcpp::Publisher<motor_interfaces::msg::Motor>::SharedPtr> pub_robot_cmds;
    std::vector<rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr> bspline_visualizations;

    std::vector<rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr> sub_robot_odoms;
    std::vector<rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr> sub_robot_dirs;
    std::vector<rclcpp::Subscription<traj_interfaces::msg::Bspline>::SharedPtr> sub_robot_bsplines;
    std::vector<rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr> sub_robot_stops;


    // rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr bspline_visualization_;
    // rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr sub_stop_msg;
    // rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_odom_msg;
    // rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr sub_dir_msg;
    // rclcpp::Subscription<traj_interfaces::msg::Bspline>::SharedPtr sub_bspline_msg;

    std::vector<Eigen::Vector3d> odom_pos, odom_vel;
    std::vector<Eigen::Quaterniond> odom_orient;
    std::vector<double> yaw;
    std::vector<std_msgs::msg::UInt8> dir;
    std::vector<vector<bspline_planner::UniformBspline>> traj;
    std::vector<rclcpp::Time> start_time, time_e;
    std::vector<double> traj_duration;

    // void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    // {
    //     // 读取当前状态
    //     odom_pos_(0) = msg->pose.pose.position.x;
    //     odom_pos_(1) = msg->pose.pose.position.y;
    //     odom_pos_(2) = msg->pose.pose.position.z;

    //     odom_vel_(0) = msg->twist.twist.linear.x;
    //     odom_vel_(1) = msg->twist.twist.linear.y;
    //     odom_vel_(2) = msg->twist.twist.linear.z;

    //     odom_orient_.w() = msg->pose.pose.orientation.w;
    //     odom_orient_.x() = msg->pose.pose.orientation.x;
    //     odom_orient_.y() = msg->pose.pose.orientation.y;
    //     odom_orient_.z() = msg->pose.pose.orientation.z;

    //     tf2::fromMsg(msg->pose.pose.orientation, quat);
    //     tf2::Matrix3x3(quat).getRPY(roll, pitch, yaw);

    //     if (dir.data == NEGATIVE)
    //     {
    //         if (yaw > 0)
    //         {
    //             yaw -= PI;
    //         }
    //         else if (yaw < 0)
    //         {
    //             yaw += PI;
    //         }
    //     }
    // }

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

    // void dir_callback(const std_msgs::msg::UInt8::SharedPtr msg)
    // {
    //     dir = *msg;
    // }

    void dir_callback(const std_msgs::msg::UInt8::SharedPtr msg, int robot_id)
    {
        dir[robot_id] = *msg;
    }

    // void stop_callback(const std_msgs::msg::UInt8::SharedPtr msg)
    // {
    //     stop_command = *msg;
    // }

    void stop_callback(const std_msgs::msg::UInt8::SharedPtr msg, int robot_id)
    {
        stop_command[robot_id] = *msg;
    }

    // void traj_callback(const traj_interfaces::msg::Bspline::SharedPtr msg)

    // {   
    //     if (receive_traj_ == true)
    //     {
    //         return;
    //     }

    //     Eigen::MatrixXd pos_pts(3, msg->pos_pts.size());

    //     Eigen::VectorXd knots(msg->knots.size());

    //     for (size_t i = 0; i < msg->knots.size(); ++i)
    //     {
    //         knots(i) = msg->knots[i];
    //     }

    //     for (size_t i = 0; i < msg->pos_pts.size(); ++i)
    //     {
    //         pos_pts(0, i) = msg->pos_pts[i].x;
    //         pos_pts(1, i) = msg->pos_pts[i].y;
    //         pos_pts(2, i) = msg->pos_pts[i].z;
    //     }

    //     bspline_planner::UniformBspline pos_traj(pos_pts, msg->order, 0.1);
    //     pos_traj.setKnot(knots);

    //     start_time_ = msg->start_time;
    //     traj_id_ = msg->traj_id;

    //     traj_.clear();
    //     traj_.push_back(pos_traj);
    //     traj_.push_back(traj_[0].getDerivative());
    //     traj_.push_back(traj_[1].getDerivative());

    //     traj_duration_ = traj_[0].getTimeSum();

    //     receive_traj_ = true;
    // }

    void traj_callback(const traj_interfaces::msg::Bspline::SharedPtr msg, int robot_id)
    {   
        if (receive_traj[robot_id] == true)
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

        bspline_planner::UniformBspline pos_traj(pos_pts, msg->order, 0.1);
        pos_traj.setKnot(knots);

        start_time[robot_id] = msg->start_time;
        // traj_id_ = msg->traj_id;

        traj_.clear();
        traj_.push_back(pos_traj);
        traj_.push_back(traj_[0].getDerivative());
        traj_.push_back(traj_[1].getDerivative());

        traj_duration[robot_id] = traj_[0].getTimeSum();

        receive_traj[robot_id] = true;
        traj[robot_id] = traj_;
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

    void MPC_calculate(double &t_cur, int robot_id)
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

        pub_robot_cmds[robot_id]->publish(motor_cmd);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<control_node>("control_node");
    mpc_controller.MPC_init(node);
    // stop_command.data = 0;
    // t_step = 0.03;
    // rclcpp::TimerBase::SharedPtr cmd_timer = node->create_wall_timer(std::chrono::milliseconds(30), std::bind(&control_node::cmdCallback, node));
    for(int i = 0 ; i < node->robot_num; ++i)
    {
        auto timer_callback = [node, i](/* rclcpp::TimerBase::SharedPtr timer */)
        {
            node->cmdCallback(/* timer, */ i);
        };
        auto timer = node->create_wall_timer(std::chrono::milliseconds(30), timer_callback);
        timers.push_back(timer);
    }

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}