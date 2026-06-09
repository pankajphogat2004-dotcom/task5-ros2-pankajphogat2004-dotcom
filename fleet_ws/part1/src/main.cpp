#include "drone.hpp"
#include "mission_drone.hpp"
#include "autonomous_drone.hpp"
#include "drone_exceptions.hpp"
#include <iostream>
#include <vector>

int main() {
    try {
        // Polymorphism demonstration
        std::vector<Vehicle*> fleet;
        
        Drone* basic_drone = new Drone("Alpha-Basic", 150.0f, 90.0f);
        MissionDrone* mission_drone = new MissionDrone("Beta-Mission", "Survey", 80.0f);
        AutonomousDrone* auto_drone = new AutonomousDrone("Gamma-Auto", "Package Delivery", 100.0f);

        fleet.push_back(basic_drone);
        fleet.push_back(mission_drone);
        fleet.push_back(auto_drone);

        std::cout << "--- Polymorphism in Action ---" << std::endl;
        for (auto v : fleet) {
            std::cout << v->get_info() << std::endl;
        }

        // Attempting to access private members (explained with comments)
        // std::cout << auto_drone->battery_level << std::endl; 
        // ERROR: 'float Vehicle::battery_level' is private within this context

        std::cout << "\n--- Testing Exceptions ---" << std::endl;
        try {
            basic_drone->take_off(200.0f); // Should throw AltitudeError
        } catch (const AltitudeError& e) {
            std::cout << "Caught expected error: " << e.what() << std::endl;
        }

        try {
            basic_drone->set_status("charging");
            basic_drone->charge_battery(10.0f, 5);
            basic_drone->set_status("idle");
            basic_drone->charge_battery(10.0f, 5); // Should throw InvalidStateError
        } catch (const InvalidStateError& e) {
            std::cout << "Caught expected error: " << e.what() << std::endl;
        }

        try {
            while(true) basic_drone->drain_battery(50.0f); // Should throw BatteryDepletedError
        } catch (const BatteryDepletedError& e) {
            std::cout << "Caught expected error: " << e.what() << std::endl;
        }

        std::cout << "\n--- Full Mission on AutonomousDrone ---" << std::endl;
        auto_drone->add_waypoint(10.0f, 20.0f, 15.0f);
        auto_drone->add_waypoint(30.0f, 40.0f, 25.0f);
        auto_drone->add_waypoint(50.0f, 60.0f, 35.0f);

        auto_drone->take_off(15.0f);
        auto_drone->set_ai_mode("auto");

        while (!auto_drone->mission_complete()) {
            auto_drone->next_waypoint();
            if (auto_drone->get_battery_level() < 98.0f) { // Trigger obstacle mid-mission
                auto_drone->detect_obstacle({30.0f, 40.0f, 25.0f}, "high");
                break; // Emergency stop triggered
            }
        }

        std::cout << auto_drone->mission_summary() << std::endl;
        std::cout << "Final Info: " << auto_drone->get_info() << std::endl;

        // Cleanup
        for (auto v : fleet) delete v;

    } catch (const DroneException& e) {
        std::cerr << "Unexpected Drone Exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Standard Exception: " << e.what() << std::endl;
    }

    return 0;
}
