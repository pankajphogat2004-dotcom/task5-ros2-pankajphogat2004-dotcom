#ifndef AUTONOMOUS_DRONE_HPP
#define AUTONOMOUS_DRONE_HPP

#include "mission_drone.hpp"

class AutonomousDrone : public MissionDrone {
private:
    std::string ai_mode; // "manual", "auto", "return_home"
    std::tuple<float, float, float> home_position;
    std::vector<std::string> obstacle_log;

public:
    AutonomousDrone(const std::string& name, const std::string& mission, float battery = 100.0f);
    virtual ~AutonomousDrone() = default;

    void set_ai_mode(const std::string& mode);
    void detect_obstacle(std::tuple<float, float, float> position, const std::string& severity);
    std::vector<std::tuple<float, float, float>> auto_replan(const std::vector<std::tuple<float, float, float>>& obstacles);

    std::string get_info() const override;
};

#endif // AUTONOMOUS_DRONE_HPP
