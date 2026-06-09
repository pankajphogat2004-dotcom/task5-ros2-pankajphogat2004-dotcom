#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp" 
#include "std_srvs/srv/trigger.hpp"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

struct DroneState {
    std::string name;
    float battery;
    float altitude;
    std::string status;
    std::string waypoint;
    float speed;
};

class FleetManager : public rclcpp::Node {
private:
    std::map<std::string, DroneState> fleet_states_;
    
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> status_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> telemetry_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> alert_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> mission_subs_;
    
    rclcpp::TimerBase::SharedPtr report_timer_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr report_service_;

public:
    FleetManager() : Node("fleet_manager") {
        std::vector<std::string> drones = {"Alpha", "Beta", "Gamma"};

        for (const auto& name : drones) {
            status_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/status", 10, 
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->status_callback(msg, name); }));
            
            telemetry_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/telemetry", 10, 
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->telemetry_callback(msg, name); }));

            alert_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/alert", 10, 
                [this, name](const std_msgs::msg::String::SharedPtr msg) { 
                    RCLCPP_WARN(this->get_logger(), "[ALERT] Drone %s: %s", name.c_str(), msg->data.c_str()); 
                }));
            
            mission_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/mission_complete", 10, 
                [this, name](const std_msgs::msg::String::SharedPtr msg) { 
                    RCLCPP_INFO(this->get_logger(), "Drone %s: %s", name.c_str(), msg->data.c_str()); 
                }));
        }

        report_timer_ = this->create_wall_timer(std::chrono::seconds(5), std::bind(&FleetManager::print_report, this));
        report_service_ = this->create_service<std_srvs::srv::Trigger>(
            "/fleet/status_report", 
            std::bind(&FleetManager::handle_report_service, this, std::placeholders::_1, std::placeholders::_2));
    }


    void status_callback(const std_msgs::msg::String::SharedPtr msg, const std::string& name) {
        std::string data = msg->data;
        fleet_states_[name].name = name;
        
        auto get_val = [&](const std::string& key) {
            size_t pos = data.find(key + ":");
            if (pos == std::string::npos) return std::string("");
            pos += key.length() + 1;
            size_t end = data.find("|", pos);
            return data.substr(pos, end - pos);
        };

        fleet_states_[name].waypoint = get_val("waypoint");
    }

    void telemetry_callback(const std_msgs::msg::String::SharedPtr msg, const std::string& name) {
        std::string json = msg->data;
        
        auto get_json_val = [&](const std::string& key, bool is_string) {
            size_t pos = json.find("\"" + key + "\":");
            if (pos == std::string::npos) return std::string("");
            pos += key.length() + 3;
            if (is_string) {
                size_t start = json.find("\"", pos) + 1;
                size_t end = json.find("\"", start);
                return json.substr(start, end - start);
            } else {
                size_t end = json.find_first_of(",}", pos);
                return json.substr(pos, end - pos);
            }
        };

        fleet_states_[name].name = get_json_val("name", true);
        fleet_states_[name].battery = std::stof(get_json_val("battery", false));
        fleet_states_[name].altitude = std::stof(get_json_val("altitude", false));
        fleet_states_[name].status = get_json_val("status", true);
        fleet_states_[name].speed = std::stof(get_json_val("speed", false));
    }

    void print_report() {
        std::cout << "\n" << std::setfill('=') << std::setw(60) << "" << std::endl;
        std::cout << std::setfill(' ') << std::left << std::setw(10) << "Drone" 
                  << std::setw(10) << "Battery" << std::setw(10) << "Altitude" 
                  << std::setw(12) << "Waypoint" << std::setw(10) << "Status" << std::endl;
        
       
        std::cout << std::setfill('-') << std::setw(60) << "" << std::setfill(' ') << std::endl;

        for (auto const& [name, state] : fleet_states_) {
            std::cout << std::left << std::setw(10) << name 
                      << std::fixed << std::setprecision(1) << std::setw(10) << state.battery
                      << std::setw(10) << state.altitude 
                      << std::setw(12) << state.waypoint 
                      << std::setw(10) << state.status << std::endl;
        }
        std::cout << std::setfill('=') << std::setw(60) << "" << std::endl;
    }

    void handle_report_service(const std::shared_ptr<std_srvs::srv::Trigger::Request>,
                               std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
        print_report();
        response->success = true;
        response->message = "Fleet report generated in console.";
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FleetManager>());
    rclcpp::shutdown();
    return 0;
}