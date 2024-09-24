#include "time.h"
#include "math.h"
#include "Eigen/Dense"
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
#include "record.cpp"

#define PI 3.1415926
#define yaw_error_max 90.0/180*PI
#define N 15

const double HALF_DISTANCE_BETWEEN_WHEELS = 0.2230;
const double WHEEL_RADIUS = 0.0625;

MPC_controller mpc_controller;
RobotPoseRecorder recorder;
Eigen::Vector3d odom_pos_, odom_vel_;
Eigen::Quaterniond odom_orient_;
tf2::Quaternion quat;
std_msgs::msg::UInt8 dir;
std_msgs::msg::UInt8 stop_command;
motor_interfaces::msg::Motor motor_cmd;

rclcpp::Time start_time_, time_s, time_e;
clock_t start_clock, end_clock;
double duration;
double traj_duration;
double linear_v, angular_v;
double kx, ky, kth;

int robot_num;
std::string control_mode;
bool display;
int traj_id_;
double roll, pitch, yaw;
vector<bspline_planner::UniformBspline> traj_;
bool receive_traj_ = false;
bool is_orientation_init = false;
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

enum DIRECTION {POSITIVE=0,NEGATIVE=1};

class control_node : public rclcpp::Node
{
public:
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
        this->declare_parameter<std::string>("control_mode", "mpc");
        this->declare_parameter<bool>("display", false);
        
        // 打印robot_num参数
        this->get_parameter("robot_num", robot_num);
        std::cout << "########## robot_num: " << robot_num << " ##########" << std::endl;

        // 打印display参数
        this->get_parameter("display", display);
        std::cout << "########## display: " << display << " ##########" << std::endl;

        // 获取Backstepping参数
        this->get_parameter("kx", kx);
        this->get_parameter("ky", ky);
        this->get_parameter("kth", kth);

        // 获取字符串参数
        this->get_parameter("control_mode", control_mode);
        std::cout << "########## control_mode: " << control_mode << " ##########" << std::endl;

        pub_motor_cmd = this->create_publisher<motor_interfaces::msg::Motor>("motor_cmd", 10);
        bspline_visualization = this->create_publisher<nav_msgs::msg::Path>("/bspline_visualization", 10);
        sub_odom_msg = this->create_subscription<nav_msgs::msg::Odometry>("odom", 10, std::bind(&control_node::odom_callback, this, std::placeholders::_1));
        sub_dir_msg = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/direction", 10, std::bind(&control_node::dir_callback, this, std::placeholders::_1));
        sub_stop_msg = this->create_subscription<std_msgs::msg::UInt8>("/traj_pub/stop", 10, std::bind(&control_node::stop_callback, this, std::placeholders::_1));
        sub_bspline_msg = this->create_subscription<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj", 10, std::bind(&control_node::traj_callback, this, std::placeholders::_1));
    }

    void cmdCallback()
    {   
        // std::cout << "########## cmdCallback ##########" << std::endl;
        /* no publishing before receive traj_ */
        if (stop_command.data == 1)
        {
            motor_cmd.left_speed = 0;
            motor_cmd.right_speed = 0;
            pub_motor_cmd->publish(motor_cmd);
            std::cout << "########## Mission Finished !!! ##########" << std::endl;
            if (!data_saved)
            {
                recorder.savePosesToFile(file_time + "_" + control_mode + "_poses.txt");
                data_saved = true;
            }
            return;
        }

        if (!receive_traj_)
            return;
        
        // RCLCPP_WARN(this->get_logger(), "Run here !");
        auto time_s = this->get_clock()->now();
        double t_cur = (time_s - start_time_).seconds();
        static rclcpp::Time time_last = this->get_clock()->now();

        if (t_cur < traj_duration && t_cur >= 0.0)
        {
            start_clock = clock();
            if (control_mode == "mpc")
            {
                // std::cout << "########## MPC ##########" << std::endl;
                MPC_calculate(t_cur);
                publish_bspline_tracking_points(t_cur);
            }
            else if (control_mode == "backstepping")
            {
                // std::cout << "########## Backstepping ##########" << std::endl;
                backstepping_calculate(t_cur);
                publish_bspline_tracking_points(t_cur);
            }
            // MPC_calculate(t_cur);
            // publish_bspline_tracking_points(t_cur);
            end_clock = clock();
            duration = (double)(end_clock - start_clock) / CLOCKS_PER_SEC * 1000;
            // ROS_INFO("Control times : %f ms",duration);
        }
        else if (t_cur >= traj_duration)
        {
            std::cout << "########## Time Finished!!! ##########" << std::endl;
            motor_cmd.left_speed = 0;
            motor_cmd.right_speed = 0;
            pub_motor_cmd->publish(motor_cmd);
            is_orientation_init = false;
            if (!data_saved)
            {
                recorder.savePosesToFile(file_time + "_" + control_mode + "_poses.txt");
                data_saved = true;
            }
        }
        else
        {   
            RCLCPP_INFO(this->get_logger(), "[Traj server]: invalid time.");
        }
        time_last = time_s;

        pub_motor_cmd->publish(motor_cmd);
    }
private:
    rclcpp::Publisher<motor_interfaces::msg::Motor>::SharedPtr pub_motor_cmd;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr bspline_visualization;
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr sub_stop_msg;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_odom_msg;
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr sub_dir_msg;
    rclcpp::Subscription<traj_interfaces::msg::Bspline>::SharedPtr sub_bspline_msg;

    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
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

        tf2::fromMsg(msg->pose.pose.orientation, quat);
        tf2::Matrix3x3(quat).getRPY(roll, pitch, yaw);

        if (dir.data == NEGATIVE)
        {
            if (yaw > 0)
            {
                yaw -= PI;
            }
            else if (yaw < 0)
            {
                yaw += PI;
            }
        }
    }


    void dir_callback(const std_msgs::msg::UInt8::SharedPtr msg)
    {
        dir = *msg;
    }

    void stop_callback(const std_msgs::msg::UInt8::SharedPtr msg)
    {
        stop_command = *msg;
    }

    void traj_callback(const traj_interfaces::msg::Bspline::SharedPtr msg)
    {   
        if (receive_traj_ == true)
        {
            return;
        }

        Eigen::MatrixXd pos_pts(3, msg->pos_pts.size());

        Eigen::VectorXd knots(msg->knots.size());

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

        start_time_ = msg->start_time;
        traj_id_ = msg->traj_id;

        traj_.clear();
        traj_.push_back(pos_traj);
        traj_.push_back(traj_[0].getDerivative());
        traj_.push_back(traj_[1].getDerivative());

        traj_duration = traj_[0].getTimeSum();

        receive_traj_ = true;
    }

    void publish_bspline_tracking_points(double &t_cur)
    {
        nav_msgs::msg::Path path_msg;
        path_msg.header.stamp = this->get_clock()->now();
        path_msg.header.frame_id = "map";

        for (double t = t_cur; t <= traj_duration; t += 0.1)
        {
            Eigen::Vector3d pos = traj_[0].evaluateDeBoor(t);
            geometry_msgs::msg::PoseStamped pose;
            pose.pose.position.x = pos(0);
            pose.pose.position.y = pos(1);
            pose.pose.position.z = pos(2);
            path_msg.poses.push_back(pose);
        }

        bspline_visualization->publish(path_msg);
    }

    void MPC_calculate(double &t_cur)
    {
        std::vector<Eigen::Vector3d> X_r; // 储存参考轨迹
        std::vector<Eigen::Vector2d> U_r; // 储存参考控制输入
        Eigen::MatrixXd u_k;
        Eigen::Vector3d pos_r, pos_r_1, pos_final, v_r_1, v_r_2, X_k;
        Eigen::Vector2d u_r;
        Eigen::Vector3d x_r, x_r_1, x_r_2;
        double v_linear_1, w;
        double t_k, t_k_1;

        // ROS_INFO("Run to here!");

        // Eigen::Vector3d pos_first = traj_[0].evaluateDeBoor(t_cur);
        // Eigen::Vector3d pos_second = traj_[0].evaluateDeBoor(t_cur+t_step);
        Eigen::Vector3d vel_start = traj_[1].evaluateDeBoor(t_cur); // 初始速度
        double yaw_start = atan2(vel_start(1), vel_start(0));       // 初始航向角
        bool is_orientation_adjust = false;
        double orientation_adjust = 0;
        pos_final = traj_[0].evaluateDeBoor(traj_duration);

        // std::cout << "########## pos_final: "<< pos_final[0] << " " << pos_final[1] << " ##########" << std::endl;
        // std::cout << "########## t_cur: " << t_cur << " ##########" << std::endl;
        // std::cout << "########## traj_duration: " << traj_duration << " ##########" << std::endl;
        // 检查当前位置是否到达终点
        if (abs(odom_pos_(0) - pos_final[0]) < 0.1 && abs(odom_pos_(1) - pos_final[1]) < 0.1)
        {
            // std::cout << "########## Mission Finished !!! ##########" << std::endl;
            stop_command.data = 1;
            return;
        }

        is_orientation_init = true;
        for (int i = 0; i < N; i++)
        {

            t_k = t_cur + i * t_step;
            t_k_1 = t_cur + (i + 1) * t_step;

            pos_r = traj_[0].evaluateDeBoor(t_k);
            pos_r_1 = traj_[0].evaluateDeBoor(t_k_1);

            x_r(0) = pos_r(0);
            x_r(1) = pos_r(1);

            v_r_1 = traj_[1].evaluateDeBoor(t_k);
            v_r_2 = traj_[1].evaluateDeBoor(t_k_1);
            v_r_1(2) = 0;
            v_r_2(2) = 0;
            v_linear_1 = v_r_1.norm();
            if ((t_k - traj_duration) >= 0)
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
        X_k(0) = odom_pos_(0);
        X_k(1) = odom_pos_(1);
        // 调整当前航向角
        if (yaw / X_r[0](2) < 0 && abs(yaw) > (PI * 5 / 6))
        {
            if (yaw < 0)
            {
                X_k(2) = yaw + 2 * PI;
            }
            else
            {
                X_k(2) = yaw - 2 * PI;
            }
        }
        else
        {
            X_k(2) = yaw;
        }

        // 记录当前位置与参考位置
        // cout<<"xr  : "<<X_r[0]<<endl;
        // cout<<"xk  : "<<X_k<<endl;
        int id = 0;
        recorder.recordPose(robot_num, id, X_r[0](0), X_r[0](1), X_r[0](2), X_k(0), X_k(1), X_k(2), t_cur);        

        // ROS_INFO("Run to here!");
        //  使用MPC求解器计算最优控制输入
        u_k = mpc_controller.MPC_Solve_qp(X_k, X_r, U_r, N); // 当前状态，参考状态，参考输入，预测步数

        if (dir.data == NEGATIVE)
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
                // RCLCPP_WARN(this->get_logger(), "U r :");
                for (int i = 0; i < U_r.size(); i++)
                {
                    cout << "vel ref :" << U_r[i](0) << "," << "w ref : " << U_r[i](1);
                    cout << endl;
                }
                cout << endl;
                // RCLCPP_WARN(this->get_logger(), "U k :");
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

        pub_motor_cmd->publish(motor_cmd);
    }

    void backstepping_calculate(double &t_cur)
    {
        Eigen::Matrix<float, 3, 3> T;
        Eigen::Matrix<float, 3, 1> Eg; // Error of global 全局坐标系下的误差
        Eigen::Matrix<float, 3, 1> Eb; // Error of body 车体坐标系下的误差
        Eigen::Vector3d pos_ref_1, pos_final; // 期望位置
        Eigen::Vector3d vel_ref_1, vel_ref_2; // 初始速度
        double Vr, Wr, linear_v, angular_v;
        double orientation_adjust = 0;
        bool is_orientation_adjust = false;

        pos_final = traj_[0].evaluateDeBoor(traj_duration);
        // std::cout << "########## pos_final: "<< pos_final[0] << " " << pos_final[1] << " ##########" << std::endl;
        // std::cout << "########## t_cur: " << t_cur << " ##########" << std::endl;
        // std::cout << "########## traj_duration: " << traj_duration << " ##########" << std::endl;

        // 检查当前位置是否到达终点
        if (abs(odom_pos_(0) - pos_final[0]) < 0.1 && abs(odom_pos_(1) - pos_final[1] < 0.1))
        {
            // std::cout << "########## Mission Finished !!! ##########" << std::endl;
            stop_command.data = 1;
            return;
        }

        pos_ref_1 = traj_[0].evaluateDeBoor(t_cur); // 期望位置
        vel_ref_1 = traj_[1].evaluateDeBoor(t_cur); // 初始速度
        T << 1, 0, -sin(yaw),
            0, 1, cos(yaw),
            0, 0, 1;

        double yaw_ref = atan2(vel_ref_1(1), vel_ref_1(0));       // 初始航向角

        // 计算期望角速度
        vel_ref_2 = traj_[1].evaluateDeBoor(t_cur + t_step);

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

        if (yaw*yaw_ref >= 0)
        {
            Eg << pos_ref_1(0) - odom_pos_(0), pos_ref_1(1) - odom_pos_(1), yaw_ref - yaw;
        }
        else
        {
            if (yaw > 0)
            {
                Eg << pos_ref_1(0) - odom_pos_(0), pos_ref_1(1) - odom_pos_(1), yaw_ref - yaw - 2 * PI;
            }
            else
            {
                Eg << pos_ref_1(0) - odom_pos_(0), pos_ref_1(1) - odom_pos_(1), yaw_ref - yaw + 2 * PI;
            }
        }

        // 记录数据
        int id = 0;
        recorder.recordPose(robot_num, id, pos_ref_1(0), pos_ref_1(1), yaw_ref, odom_pos_(0), odom_pos_(1), yaw, t_cur);

        Eb = T * Eg;
        Vr = sqrt(pow(vel_ref_1(0), 2) + pow(vel_ref_1(1), 2));
        linear_v = Vr * cos(Eb[2]) + kx * Eb[0];
        angular_v = Wr + ky * Eb[1] + kth * sin(Eb[2]);

        motor_cmd.left_speed = (linear_v - angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;
        motor_cmd.right_speed = (linear_v + angular_v * HALF_DISTANCE_BETWEEN_WHEELS) / WHEEL_RADIUS;

        pub_motor_cmd->publish(motor_cmd);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    // 创建一个名为control_node的节点，并将其储存在一个共享指针node中
    auto node = std::make_shared<control_node>("control_node");
    mpc_controller.MPC_init(node);
    stop_command.data = 0;
    rclcpp::TimerBase::SharedPtr cmd_timer = node->create_wall_timer(std::chrono::milliseconds(30), std::bind(&control_node::cmdCallback, node));

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}