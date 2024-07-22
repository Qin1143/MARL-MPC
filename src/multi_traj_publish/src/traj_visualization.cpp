#include "traj_visualization.hpp"

PlanningVisualization::PlanningVisualization(const rclcpp::NodeOptions & options, int robot_num) : Node("planning_visualization", options), robot_num_(robot_num)
{
    // traj_rviz_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("/traj_visualization", 10);
    // std::vector<rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr> traj_rviz_pubs_;
    for (int i = 0; i < robot_num; ++i)
    {
      auto traj_rviz_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("/traj_visualization_" + std::to_string(i + 1), 10);
      traj_rviz_pubs_.push_back(traj_rviz_pub_);
    }
}

void PlanningVisualization::displayMarkerList(rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr pub, const std::vector<Eigen::Vector3d> &list, double scale,
                                               Eigen::Vector4d color, int id)
{
  visualization_msgs::msg::Marker sphere, line_strip; //储存球体和线条的消息
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

void PlanningVisualization::displayTraj(Eigen::MatrixXd optimal_pts, int robot_id, int id)
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
