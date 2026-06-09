#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "mission_drone.hpp"
#include <sstream>
#include <iomanip>

class DroneNode : public rclcpp::Node {
private:
    std::shared_ptr<MissionDrone> drone_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr telemetry_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr alert_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr mission_pub_;
    rclcpp::TimerBase::SharedPtr status_timer_;
    rclcpp::TimerBase::SharedPtr telemetry_timer_;

    std::string drone_name_;
    int publish_count_ = 0;

public:
    DroneNode() : Node("drone_node") {
        this->declare_parameter("drone_name", "Alpha");
        this->declare_parameter("initial_battery", 100.0);
        this->declare_parameter("mission_name", "Survey");

        drone_name_ = this->get_parameter("drone_name").as_string();
        float initial_battery = this->get_parameter("initial_battery").as_double();
        std::string mission_name = this->get_parameter("mission_name").as_string();

        drone_ = std::make_shared<MissionDrone>(drone_name_, mission_name, initial_battery);
        
        // Add 5 waypoints
        for (int i = 1; i <= 5; ++i) {
            drone_->add_waypoint(i * 10.0f, i * 20.0f, 15.0f);
        }
        drone_->take_off(15.0f);

        
        status_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/status", 10);
        telemetry_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/telemetry", 10);
        alert_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/alert", 10);
        mission_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/mission_complete", 10);

        status_timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&DroneNode::publish_status, this));
        telemetry_timer_ = this->create_wall_timer(std::chrono::seconds(2), std::bind(&DroneNode::publish_telemetry, this));
    }

    void publish_status() {
        if (drone_->get_battery_level() <= 0) return;

        drone_->drain_battery(0.5f);
        publish_count_++;

        if (publish_count_ % 3 == 0) {
            if (!drone_->mission_complete()) {
                drone_->next_waypoint();
            } else {
                auto msg = std_msgs::msg::String();
                msg.data = "Mission Complete for " + drone_name_;
                mission_pub_->publish(msg);
            }
        }

        if (drone_->is_critical()) {
            auto msg = std_msgs::msg::String();
            msg.data = "CRITICAL BATTERY: " + std::to_string(drone_->get_battery_level()) + "%";
            alert_pub_->publish(msg);
            drone_->land();
        }

        auto msg = std_msgs::msg::String();
        std::stringstream ss;
        ss << "name:" << drone_name_ 
           << "|battery:" << std::fixed << std::setprecision(1) << drone_->get_battery_level()
           << "|altitude:" << drone_->get_altitude()
           << "|status:" << drone_->get_status()
           << "|waypoint:" << (drone_->mission_complete() ? 5 : (publish_count_ / 3) % 6) << "/5"
           << "|speed:" << drone_->get_speed();
        msg.data = ss.str();
        status_pub_->publish(msg);
    }

    void publish_telemetry() {
        auto msg = std_msgs::msg::String();
        std::stringstream ss;
        ss << "{"
           << "\"name\": \"" << drone_name_ << "\", "
           << "\"battery\": " << std::fixed << std::setprecision(2) << drone_->get_battery_level() << ", "
           << "\"altitude\": " << drone_->get_altitude() << ", "
           << "\"status\": \"" << drone_->get_status() << "\", "
           << "\"speed\": " << drone_->get_speed()
           << "}";
        msg.data = ss.str();
        telemetry_pub_->publish(msg);
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DroneNode>());
    rclcpp::shutdown();
    return 0;
}