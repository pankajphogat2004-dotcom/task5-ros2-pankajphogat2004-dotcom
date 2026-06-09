#ifndef MISSION_DRONE_HPP
#define MISSION_DRONE_HPP

#include "drone.hpp"
#include <tuple>
#include <vector>

class MissionDrone : public Drone {
protected:
    std::string mission_name;
    std::vector<std::tuple<float, float, float>> waypoints;
    int current_waypoint_index;
    std::vector<std::pair<std::tuple<float, float, float>, std::string>> visited_waypoints;

public:
    MissionDrone(const std::string& name, const std::string& mission, float battery = 100.0f);
    virtual ~MissionDrone() = default;

    std::tuple<float, float, float> next_waypoint();
    void skip_waypoint(const std::string& reason);
    bool mission_complete() const;
    std::string mission_summary() const;

    std::string get_info() const override;

    void add_waypoint(float x, float y, float z);
};

#endif // MISSION_DRONE_HPP
