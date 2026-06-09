#include "drone.hpp"
#include "drone_exceptions.hpp"

Drone::Drone(const std::string& name, float max_alt, float battery)
    : Vehicle(name, battery), altitude(0.0f), max_altitude(max_alt), speed(0.0f) {}

void Drone::take_off(float target_altitude) {
    if (target_altitude > max_altitude) {
        throw AltitudeError("Target altitude " + std::to_string(target_altitude) + " exceeds max altitude " + std::to_string(max_altitude));
    }
    altitude = target_altitude;
    set_status("flying");
    speed = 5.0f;
    log_event("Took off to altitude " + std::to_string(altitude));
}

void Drone::land() {
    altitude = 0.0f;
    speed = 0.0f;
    set_status("idle");
    log_event("Landed");
}

void Drone::emergency_stop() {
    log_event("EMERGENCY STOP TRIGGERED");
    drain_battery(30.0f);
    land();
}

std::string Drone::get_info() const {
    return "Drone: " + name + " | Battery: " + std::to_string(battery_level) + 
           "% | Status: " + status + " | Altitude: " + std::to_string(altitude) + "m";
}
