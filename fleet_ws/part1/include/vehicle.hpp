#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

class Vehicle {
protected:
    std::string name;
    float battery_level;
    std::string status;
    std::vector<std::string> flight_log;

    void log_event(const std::string& event);
    std::string get_current_timestamp();

public:
    Vehicle(const std::string& name, float battery = 100.0f);
    virtual ~Vehicle() = default;

    virtual std::string get_info() const = 0;

    void drain_battery(float amount);
    void charge_battery(float amount, int duration_seconds);
    bool is_critical() const;
    std::string get_flight_log() const;

    // Getters
    std::string get_name() const { return name; }
    float get_battery_level() const { return battery_level; }
    std::string get_status() const { return status; }

    void set_status(const std::string& new_status);
};

#endif // VEHICLE_HPP
