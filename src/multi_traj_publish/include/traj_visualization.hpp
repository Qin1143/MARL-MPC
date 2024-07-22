#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "Eigen/Eigen"
#include "vector"


class PlanningVisualization : public rclcpp::Node
{
    public:
        explicit PlanningVisualization(const rclcpp::NodeOptions & options, int robot_num);

        typedef std::shared_ptr<PlanningVisualization> Ptr;

        void displayMarkerList(rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr pub, const std::vector<Eigen::Vector3d> &list, double scale,
                               Eigen::Vector4d color, int id);
        void displayTraj(Eigen::MatrixXd optimal_pts, int robot_id,int id);
    private:
        rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr traj_rviz_pub_;
        std::vector<rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr> traj_rviz_pubs_;
        int robot_num_;
};