#include "vehicle.hpp"
#include "drone_exceptions.hpp"
#include <iostream>

Vehicle::Vehicle(const std::string& name, float battery) 
    : name(name), battery_level(battery), status("idle") {
    log_event("Vehicle initialized");
}

void Vehicle::log_event(const std::string& event) {
    flight_log.push_back("[" + get_current_timestamp() + "] " + event);
}

std::string Vehicle::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

void Vehicle::drain_battery(float amount) {
    if (battery_level <= 0) {
        throw BatteryDepletedError("Battery is already empty!");
    }
    battery_level -= amount;
    if (battery_level < 0) battery_level = 0;
    log_event("Battery drained by " + std::to_string(amount) + ". Current: " + std::to_string(battery_level));
}

void Vehicle::charge_battery(float amount, int duration_seconds) {
    if (status != "charging") {
        throw InvalidStateError("Cannot charge unless in 'charging' state!");
    }
    battery_level += amount;
    if (battery_level > 100.0f) battery_level = 100.0f;
    log_event("Battery charged by " + std::to_string(amount) + " over " + std::to_string(duration_seconds) + " seconds.");
}

bool Vehicle::is_critical() const {
    return battery_level < 20.0f;
}

std::string Vehicle::get_flight_log() const {
    std::string log;
    for (const auto& entry : flight_log) {
        log += entry + "\n";
    }
    return log;
}

void Vehicle::set_status(const std::string& new_status) {
    if (new_status != "idle" && new_status != "flying" && new_status != "charging") {
        log_event("Attempted invalid status change: " + new_status);
        return;
    }
    status = new_status;
    log_event("Status changed to: " + status);
}
