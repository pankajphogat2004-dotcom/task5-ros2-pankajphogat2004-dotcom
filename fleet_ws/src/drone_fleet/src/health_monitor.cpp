#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <deque>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>

struct HealthData {
    std::deque<float> battery_history;
    rclcpp::Time last_time;
    float last_battery;
    float drain_rate = 0.0f;
};

class HealthMonitor : public rclcpp::Node {
private:
    std::map<std::string, HealthData> health_map_; 
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> telemetry_subs_; 
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr warning_pub_; 
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr summary_pub_; 
    rclcpp::TimerBase::SharedPtr diag_timer_;

public:
    HealthMonitor() : Node("health_monitor") {
        std::vector<std::string> drones = {"Alpha", "Beta", "Gamma"}; 
        for (const auto& name : drones) {
            
            telemetry_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/telemetry", 10, 
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->telemetry_callback(msg, name); }));
        }

        warning_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_warning", 10); 
        summary_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_summary", 10); 
        diag_timer_ = this->create_wall_timer(std::chrono::seconds(10), std::bind(&HealthMonitor::print_diagnostics, this));
    }

    
    void telemetry_callback(const std_msgs::msg::String::SharedPtr msg, const std::string& name) {
        std::string json = msg->data;
        size_t pos = json.find("\"battery\":");
        if (pos == std::string::npos) return;
        pos += 10;
        size_t end = json.find_first_of(",}", pos);
        float current_battery = std::stof(json.substr(pos, end - pos));

        auto& data = health_map_[name];
        auto now = this->now();

        if (data.battery_history.size() > 0) {
            double dt = (now - data.last_time).seconds();
            if (dt > 0) {
                float drain = (data.last_battery - current_battery) / dt;
                data.drain_rate = drain;
                if (drain > 1.5f) {
                    auto warn_msg = std_msgs::msg::String();
                    warn_msg.data = "WARNING: High drain rate for " + name + ": " + std::to_string(drain) + "/s";
                    warning_pub_->publish(warn_msg);
                }
            }
        }

        data.battery_history.push_back(current_battery);
        if (data.battery_history.size() > 10) data.battery_history.pop_front();
        data.last_time = now;
        data.last_battery = current_battery;
    }

    void print_diagnostics() {
        std::cout << "\n--- HEALTH DIAGNOSTICS ---" << std::endl;
        std::cout << std::left << std::setw(10) << "Drone" << std::setw(15) << "Drain Rate" 
                  << std::setw(15) << "T-Critical" << std::setw(15) << "T-Depletion" << std::endl;

        std::stringstream json_ss;
        json_ss << "{ \"health_summary\": [";

        bool first = true;
        for (auto const& [name, data] : health_map_) {
            float t_critical = (data.drain_rate > 0) ? (data.last_battery - 20.0f) / data.drain_rate : -1.0f;
            float t_deplete = (data.drain_rate > 0) ? data.last_battery / data.drain_rate : -1.0f;

            std::cout << std::left << std::setw(10) << name 
                      << std::fixed << std::setprecision(2) << std::setw(15) << data.drain_rate
                      << std::setw(15) << (t_critical > 0 ? std::to_string(t_critical) + "s" : "N/A")
                      << std::setw(15) << (t_deplete > 0 ? std::to_string(t_deplete) + "s" : "N/A") << std::endl;

            if (!first) json_ss << ", ";
            json_ss << "{\"name\":\"" << name << "\", \"drain_rate\":" << data.drain_rate << "}";
            first = false;
        }
        json_ss << "] }";
        
        auto msg = std_msgs::msg::String();
        msg.data = json_ss.str();
        summary_pub_->publish(msg);
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<HealthMonitor>());
    rclcpp::shutdown();
    return 0;
}