#include "mission_drone.hpp"
#include <iostream>

MissionDrone::MissionDrone(const std::string& name, const std::string& mission, float battery)
    : Drone(name, 100.0f, battery), mission_name(mission), current_waypoint_index(0) {}

void MissionDrone::add_waypoint(float x, float y, float z) {
    waypoints.push_back(std::make_tuple(x, y, z));
}

std::tuple<float, float, float> MissionDrone::next_waypoint() {
    if (mission_complete()) {
        return std::make_tuple(0.0f, 0.0f, 0.0f);
    }
    auto wp = waypoints[current_waypoint_index];
    visited_waypoints.push_back({wp, get_current_timestamp()});
    current_waypoint_index++;
    drain_battery(1.5f);
    log_event("Moved to waypoint " + std::to_string(current_waypoint_index));
    return wp;
}

void MissionDrone::skip_waypoint(const std::string& reason) {
    if (!mission_complete()) {
        log_event("Skipped waypoint " + std::to_string(current_waypoint_index + 1) + " Reason: " + reason);
        current_waypoint_index++;
    }
}

bool MissionDrone::mission_complete() const {
    return current_waypoint_index >= static_cast<int>(waypoints.size());
}

std::string MissionDrone::mission_summary() const {
    std::string summary = "Mission: " + mission_name + "\nWaypoints visited:\n";
    for (const auto& entry : visited_waypoints) {
        auto [x, y, z] = entry.first;
        summary += "- (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") at " + entry.second + "\n";
    }
    return summary;
}

std::string MissionDrone::get_info() const {
    return "MissionDrone: " + name + " | Mission: " + mission_name + 
           " | Battery: " + std::to_string(battery_level) + "% | WP: " + 
           std::to_string(current_waypoint_index) + "/" + std::to_string(waypoints.size());
}
