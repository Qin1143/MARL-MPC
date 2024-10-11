#include <Eigen/Dense>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <fstream>

// PlanningVisualization::Ptr visualization_;

class TrajectoryPublisher : public rclcpp::Node
{
public:
    TrajectoryPublisher(std::string node_name)  : Node(node_name)
    {   
        for (int i = 0; i < robot_num; ++i)
        {
            auto pose_publisher = this->create_publisher<geometry_msgs::msg::Pose>("/traj_pub/pose_" + std::to_string(i+1), 10);
            pose_publishers_.push_back(pose_publisher);
        }
        this->readTrajectory("/home/tony/webots_ws/robot_state_list.txt");
        timer_ = this->create_wall_timer(std::chrono::milliseconds(100), std::bind(&TrajectoryPublisher::publishMultiTrajectory, this)); // 单位毫秒 10Hz 的频率
    }

private:
    std::vector<rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr> pose_publishers_;
    std::map<int, std::vector<std::tuple<double, double, double>>> trajectories_;
    rclcpp::TimerBase::SharedPtr timer_;
    const int robot_num = 4; // Adjust this number as needed
    size_t index = 0;

    void publishMultiTrajectory()
    {
        for(int i = 0; i < robot_num; i++)
        {
            geometry_msgs::msg::Pose pose;
            if (index < trajectories_[i].size()) {
                pose.position.x = std::get<0>(trajectories_[i][index]);
                pose.position.y = std::get<1>(trajectories_[i][index]);
                pose.position.z = 0.0;
                pose.orientation.w = std::get<2>(trajectories_[i][index]);
                pose_publishers_[i]->publish(pose);
            }
        }
        index++;
    }

    void readTrajectory(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        int robot_id;
        double x, y, theta, time;
        while (file >> robot_id >> x >> y >> theta >> time) {
            trajectories_[robot_id].emplace_back(x, y, theta);
        }

        file.close();
    }
};

int main(int argc, char** argv)
{   
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TrajectoryPublisher>("traj_node"));
    rclcpp::shutdown();
    return 0;
}