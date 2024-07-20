#include "traj_publish/traj_pub.hpp"

PlanningVisualization::Ptr visualization_;

class TrajectoryPublisher : public rclcpp::Node
{
public:
    TrajectoryPublisher(std::string node_name)  : Node(node_name)
    {   
        visualization_.reset(new PlanningVisualization(get_node_options()));
        bspline_publisher_ = this->create_publisher<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj", 10);
        // traj_publisher_ = this->create_publisher<nav_msgs::msg::Path>("/traj_pub/trajectory", 10);
        dir_publisher_ = this->create_publisher<std_msgs::msg::UInt8>("/traj_pub/direction", 10);
        timer_ = this->create_wall_timer(std::chrono::milliseconds(1000), std::bind(&TrajectoryPublisher::publishBspline, this));
        // 单位毫秒 10Hz 的频率
    }

private:
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr traj_publisher_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr dir_publisher_;
    rclcpp::Publisher<traj_interfaces::msg::Bspline>::SharedPtr bspline_publisher_;
    enum DIRECTION {POSITIVE=0,NEGATIVE=1};
    DIRECTION dir = POSITIVE;

    void publishBspline()
    {
        traj_interfaces::msg::Bspline bspline;
        std_msgs::msg::UInt8 dir_new;

        dir_new.data = dir;
        dir_publisher_->publish(dir_new);

        bspline.order = 3;
        bspline.start_time = this->get_clock()->now();
        bspline.traj_id = 1;

        Eigen::MatrixXd pos_pts(3, 7);
        pos_pts << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,// 第一行：x坐标
           0.0, 0.0, 1.0, 3.0, 3.0, 3.0, 2.0,// 第二行：y坐标
           0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;// 第三行：z坐标
        bspline.pos_pts.reserve(pos_pts.cols()); // 预留足够的空间来存储控制点
        // Eigen::Vector3d point_temp;
        for (int i = 0; i < pos_pts.cols(); ++i)
        {
            geometry_msgs::msg::Point pt;
            pt.x = pos_pts(0, i);
            pt.y = pos_pts(1, i);
            pt.z = pos_pts(2, i);
            bspline.pos_pts.push_back(pt);
            // point_temp << pt.x, pt.x, pt.x;
            // cout<<"point : "<<point_temp<<endl;
        }

        int m_k = pos_pts.cols() + 1 + bspline.order; // 控制点个数
        int mid = m_k - bspline.order*2;
        Eigen::VectorXd mid_knots = Eigen::VectorXd::LinSpaced(mid, 0.0, pos_pts.cols());

        // Eigen::VectorXd knots = Eigen::VectorXd::LinSpaced(8, 0.0, 1.0); // knot vector 节点向量 (数量=n+1+p+1)

        Eigen::VectorXd knots(m_k); // knot vector 节点向量
        // knots << 0.0, 0.0, 0.0, 0.0, 3.0, 3.0, 3.0, 3.0;
        for (int i = 0; i < bspline.order; ++i)
        {
            knots(i) = 0.0;
        }

        for (int i = 0; i < bspline.order; ++i)
        {
            knots(m_k - 1 - i) = pos_pts.cols();
        }

        for (int i = 0; i < mid; ++i)
        {
            knots(bspline.order + i) = mid_knots(i);
        }
        // 输出knots的数值
        // std::cout<<"knots: "<<knots<<std::endl;

        bspline.knots.reserve(knots.cols());

        for (int i = 0; i < knots.size(); ++i)
        {
            bspline.knots.push_back(knots(i));
        }

        // 设置偏航角点和偏航角时间间隔
        bspline.yaw_pts = {0.0, 0.5, 1.0, 1.5}; // 示例偏航角点
        bspline.yaw_dt = 0.5;                   // 示例偏航角时间间隔

        // 发布Bspline消息
        bspline_publisher_->publish(bspline);
        Eigen::MatrixXd control_points = pos_pts;
        // // 打印控制点信息
        // std::cout << "Control Points Size: " << control_points.size() << std::endl;
        // for (int i = 0; i < control_points.cols(); ++i)
        // {
        //     std::cout << "Control Point " << i + 1 << ": (" << control_points(0, i) << ", " << control_points(1, i) << ")" << std::endl;
        // }

        visualization_->displayTraj(control_points, 0);

    }

    void publishTrajectory()
    {
        nav_msgs::msg::Path trajectory;
        std_msgs::msg::UInt8 dir_new;

        dir_new.data = dir;
        dir_publisher_->publish(dir_new);

        trajectory.header.frame_id = "map";
        trajectory.header.stamp = this->get_clock()->now();

        // 定义四个控制点 (x, y)
        std::vector<std::pair<double, double>> control_points = {
            {0.0, 0.0}, // 控制点1
            {0.0, 2.0}, // 控制点2
            {4.0, 2.0}, // 控制点3
            {4.0, 0.0}  // 控制点4
        };

        // Generate Bezier curve points
        for (double t = 0.0; t <= 1.0; t += 0.1)
        {
            double x = pow(1 - t, 3) * control_points[0].first +
                       3 * t * pow(1 - t, 2) * control_points[1].first +
                       3 * pow(t, 2) * (1 - t) * control_points[2].first +
                       pow(t, 3) * control_points[3].first;

            double y = pow(1 - t, 3) * control_points[0].second +
                       3 * t * pow(1 - t, 2) * control_points[1].second +
                       3 * pow(t, 2) * (1 - t) * control_points[2].second +
                       pow(t, 3) * control_points[3].second;

            geometry_msgs::msg::PoseStamped pose;
            pose.header.frame_id = "map";
            pose.header.stamp = trajectory.header.stamp;
            pose.pose.position.x = x;
            pose.pose.position.y = y;
            pose.pose.position.z = 0.0;
            pose.pose.orientation.x = 0.0;
            pose.pose.orientation.y = 0.0;
            pose.pose.orientation.z = 0.0;
            pose.pose.orientation.w = 1.0;

            trajectory.poses.push_back(pose);
        }

        traj_publisher_->publish(trajectory);
    }

    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TrajectoryPublisher>("traj_pub"));
    rclcpp::shutdown();
    return 0;
}