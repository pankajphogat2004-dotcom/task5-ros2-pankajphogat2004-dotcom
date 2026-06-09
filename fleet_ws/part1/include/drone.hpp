#ifndef DRONE_HPP
#define DRONE_HPP

#include "vehicle.hpp"

class Drone : public Vehicle {
protected:
    float altitude;
    float max_altitude;
    float speed;

public:
    Drone(const std::string& name, float max_alt = 100.0f, float battery = 100.0f);
    virtual ~Drone() = default;

    void take_off(float target_altitude);
    void land();
    void emergency_stop();

    std::string get_info() const override;

    float get_altitude() const { return altitude; }
    float get_speed() const { return speed; }
};

#endif // DRONE_HPP
