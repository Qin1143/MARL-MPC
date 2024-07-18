#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "Eigen/Eigen"
#include "vector"


class PlanningVisualization : public rclcpp::Node
{
    public:
        explicit PlanningVisualization(const rclcpp::NodeOptions & options);

        typedef std::shared_ptr<PlanningVisualization> Ptr;

        void displayMarkerList(rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr pub, const std::vector<Eigen::Vector3d> &list, double scale,
                               Eigen::Vector4d color, int id);
        void displayTraj(Eigen::MatrixXd optimal_pts, int id);
    private:
        rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr traj_rviz_pub_;
};