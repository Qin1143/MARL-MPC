#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>

struct Pose {
    double xr;
    double yr;
    double yawr;
    double x;
    double y;
    double yaw;
    double time;
};

class RobotPoseRecorder {
public:
    RobotPoseRecorder(int robot_num) : robot_num_(robot_num) {}

    void recordPose(int &robot_id, double &xr, double &yr, double &yawr, double &x, double &y, double &yaw, double &time)
    {
        if (robot_id >= 0 && robot_id < robot_num_) 
        {
            Pose pose = {xr, yr, yawr, x, y, yaw, time};
            poses_[robot_id].push_back(pose);
        } 
        else 
        {
            std::cerr << "Invalid robot ID: " << robot_id << std::endl;
        }
    }

    void savePosesToFile(const std::string& filename) const 
    {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        for (const auto& entry : poses_) {
            int robot_id = entry.first;
            for (const auto& pose : entry.second) {
                file << robot_id << " " << pose.x << " " << pose.y << " " << pose.yaw << "\n";
            }
        }

        file.close();
    }

private:
    int robot_num_;
    std::unordered_map<int, std::vector<Pose>> poses_; // 定义一个unordered_map，键为int，值为vector<Pose>
};