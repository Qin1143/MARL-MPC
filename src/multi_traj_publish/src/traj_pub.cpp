#include "traj_pub.hpp"

// PlanningVisualization::Ptr visualization_;

class TrajectoryPublisher : public rclcpp::Node
{
public:
    TrajectoryPublisher(std::string node_name)  : Node(node_name)
    {   
        robot_num = 3;
        Eigen::MatrixXd pos_pts_1(3, 9);
        pos_pts_1 << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, // 第一行：x坐标
            0.0, 0.0, 1.0, 3.0, 3.0, 3.0, 1.0, 0.0, 0.0,        // 第二行：y坐标
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;        // 第三行：z坐标
        pos_pts.push_back(pos_pts_1);

        Eigen::MatrixXd pos_pts_2(3, 9);
        pos_pts_2 << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, // 第一行：x坐标
            2.0, 2.0, 3.0, 5.0, 5.0, 5.0, 3.0, 2.0, 2.0,          // 第二行：y坐标
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;          // 第三行：z坐标
        pos_pts.push_back(pos_pts_2);

        Eigen::MatrixXd pos_pts_3(3, 9);
        pos_pts_3 << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, // 第一行：x坐标
            -2.0, -2.0, -1.0, 1.0, 1.0, 1.0, -1.0, -2.0, -2.0,          // 第二行：y坐标
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;          // 第三行：z坐标
        pos_pts.push_back(pos_pts_3);

        // visualization_.reset(new PlanningVisualization(get_node_options(), robot_num));
        for (int i = 0; i < robot_num; ++i)
        {
            auto bspline_publisher = this->create_publisher<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj_" + std::to_string(i+1), 10);
            bspline_publishers_.push_back(bspline_publisher);

            auto dir_publisher = this->create_publisher<std_msgs::msg::UInt8>("/traj_pub/direction_" + std::to_string(i+1), 10);
            dir_publishers_.push_back(dir_publisher);

            auto traj_rviz_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("/traj_visualization_" + std::to_string(i + 1), 10);
            traj_rviz_pubs_.push_back(traj_rviz_pub_);
        }
        // robot_1_bspline_publisher_ = this->create_publisher<traj_interfaces::msg::Bspline>("/traj_pub/bspline_traj", 10);
        // robot_1_dir_publisher_ = this->create_publisher<std_msgs::msg::UInt8>("/traj_pub/direction", 10);
        // 使用lambda表达式代替std::bind
        // timer_ = this->create_wall_timer(std::chrono::milliseconds(1000), [this, pos_pts]()
        //                                  { this->publishBspline(pos_pts); }); // 单位毫秒 10Hz 的频率
        timer_ = this->create_wall_timer(std::chrono::milliseconds(1000), std::bind(&TrajectoryPublisher::publishMultiTrajectory, this)); // 单位毫秒 10Hz 的频率
    }

private:
    std::vector<rclcpp::Publisher<traj_interfaces::msg::Bspline>::SharedPtr> bspline_publishers_;
    std::vector<rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr> dir_publishers_;
    std::vector<rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr> traj_rviz_pubs_;

    rclcpp::TimerBase::SharedPtr timer_;
    // rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr robot_1_dir_publisher_;
    // rclcpp::Publisher<traj_interfaces::msg::Bspline>::SharedPtr robot_1_bspline_publisher_;
    enum DIRECTION {POSITIVE=0,NEGATIVE=1};
    DIRECTION dir = POSITIVE;
    std::vector<Eigen::MatrixXd> pos_pts; // 定义为类的成员变量，方便在publishBspline函数中使用
    int robot_num;

    void publishMultiTrajectory()
    {
        for(int i = 0; i < robot_num; i++)
        {
            publishBspline(pos_pts[i], bspline_publishers_[i], dir_publishers_[i], i);
        }
    }

    void publishBspline(Eigen::MatrixXd pos_pts, rclcpp::Publisher<traj_interfaces::msg::Bspline>::SharedPtr bspline_publisher_, rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr dir_publisher_, int robot_id)
    {
        // std::cout << "pos_pts:" << pos_pts << std::endl;
        traj_interfaces::msg::Bspline bspline;
        std_msgs::msg::UInt8 dir_new;

        dir_new.data = dir;
        dir_publisher_->publish(dir_new);

        bspline.order = 3;
        bspline.start_time = this->get_clock()->now();
        bspline.traj_id = 1;

        // Eigen::MatrixXd pos_pts(3, 9);
        // pos_pts << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,// 第一行：x坐标
        //    0.0, 0.0, 1.0, 3.0, 3.0, 3.0, 1.0, 0.0, 0.0,// 第二行：y坐标
        //    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;// 第三行：z坐标
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
        // std::cout << "#########Publish Bspline trajectory#########" << std::endl;
        Eigen::MatrixXd control_points = pos_pts;
        // // 打印控制点信息
        // std::cout << "Control Points Size: " << control_points.size() << std::endl;
        // for (int i = 0; i < control_points.cols(); ++i)
        // {
        //     std::cout << "Control Point " << i + 1 << ": (" << control_points(0, i) << ", " << control_points(1, i) << ")" << std::endl;
        // }
        displayTraj(control_points, robot_id, 1);
    }

    void displayMarkerList(rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr pub, const std::vector<Eigen::Vector3d> &list, double scale,
                                                  Eigen::Vector4d color, int id)
    {
        visualization_msgs::msg::Marker sphere, line_strip; // 储存球体和线条的消息
        sphere.header.frame_id = line_strip.header.frame_id = "map";
        sphere.header.stamp = line_strip.header.stamp = this->get_clock()->now();
        sphere.type = visualization_msgs::msg::Marker::SPHERE_LIST;
        line_strip.type = visualization_msgs::msg::Marker::LINE_STRIP;
        sphere.action = line_strip.action = visualization_msgs::msg::Marker::ADD;
        sphere.id = id;
        line_strip.id = id + 1000;

        sphere.pose.orientation.w = line_strip.pose.orientation.w = 1.0;
        sphere.color.r = line_strip.color.r = color(0);
        sphere.color.g = line_strip.color.g = color(1);
        sphere.color.b = line_strip.color.b = color(2);
        sphere.color.a = line_strip.color.a = color(3) > 1e-5 ? color(3) : 1.0;
        sphere.scale.x = scale;
        sphere.scale.y = scale;
        sphere.scale.z = scale;
        line_strip.scale.x = scale / 2;
        geometry_msgs::msg::Point pt;
        for (int i = 0; i < int(list.size()); i++)
        {
            pt.x = list[i](0);
            pt.y = list[i](1);
            pt.z = list[i](2);
            sphere.points.push_back(pt);
            line_strip.points.push_back(pt);
        }
        pub->publish(sphere);
        pub->publish(line_strip);
    }

    void displayTraj(Eigen::MatrixXd optimal_pts, int robot_id, int id)
    {
        if (traj_rviz_pubs_[robot_id]->get_subscription_count() == 0)
        {
            return;
        }

        std::vector<Eigen::Vector3d> list;
        for (int i = 0; i < optimal_pts.cols(); i++)
        {
            Eigen::Vector3d pt = optimal_pts.col(i).transpose();
            list.push_back(pt);
        }
        Eigen::Vector4d color(1, 0, 0, 1);
        displayMarkerList(traj_rviz_pubs_[robot_id], list, 0.15, color, id);
    }
};

int main(int argc, char** argv)
{   
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TrajectoryPublisher>("traj_pub"));
    rclcpp::shutdown();
    return 0;
}