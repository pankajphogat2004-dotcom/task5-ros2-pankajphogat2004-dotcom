#include "autonomous_drone.hpp"

AutonomousDrone::AutonomousDrone(const std::string& name, const std::string& mission, float battery)
    : MissionDrone(name, mission, battery), ai_mode("manual"), home_position(std::make_tuple(0.0f, 0.0f, 0.0f)) {}

void AutonomousDrone::set_ai_mode(const std::string& mode) {
    ai_mode = mode;
    log_event("AI Mode set to: " + ai_mode);
    if (ai_mode == "return_home") {
        waypoints.insert(waypoints.begin() + current_waypoint_index, home_position);
        log_event("Return home triggered. Inserted home as next waypoint.");
    }
}

void AutonomousDrone::detect_obstacle(std::tuple<float, float, float> position, const std::string& severity) {
    auto [x, y, z] = position;
    std::string log_msg = "Obstacle detected at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") Severity: " + severity;
    obstacle_log.push_back("[" + get_current_timestamp() + "] " + log_msg);
    log_event(log_msg);
    if (severity == "high") {
        emergency_stop();
    }
}

std::vector<std::tuple<float, float, float>> AutonomousDrone::auto_replan(const std::vector<std::tuple<float, float, float>>& obstacles) {
    log_event("Auto-replanning initiated...");
    std::vector<std::tuple<float, float, float>> new_waypoints;
    // Simple logic: add 5 to Z to avoid obstacle within 5 units (dummy logic for simulation)
    for (const auto& wp : waypoints) {
        bool collision = false;
        for (const auto& obs : obstacles) {
            float dx = std::get<0>(wp) - std::get<0>(obs);
            float dy = std::get<1>(wp) - std::get<1>(obs);
            float dz = std::get<2>(wp) - std::get<2>(obs);
            if (dx*dx + dy*dy + dz*dz < 25.0f) {
                collision = true;
                break;
            }
        }
        if (collision) {
            new_waypoints.push_back(std::make_tuple(std::get<0>(wp), std::get<1>(wp), std::get<2>(wp) + 10.0f));
        } else {
            new_waypoints.push_back(wp);
        }
    }
    waypoints = new_waypoints;
    return waypoints;
}

std::string AutonomousDrone::get_info() const {
    return "AutonomousDrone: " + name + " | Mode: " + ai_mode + 
           " | Battery: " + std::to_string(battery_level) + "% | WP: " + 
           std::to_string(current_waypoint_index) + "/" + std::to_string(waypoints.size());
}
