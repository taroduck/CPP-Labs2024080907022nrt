#include "Executor.h"

#include <iostream>
#include <string_view>

namespace {
void PrintPose(std::string_view label, const Executor& car) {
    const Pose pose = car.Query();
    std::cout << label << ": (" << pose.x << ", " << pose.y << ", "
              << pose.heading << ")\n";
}
}

int main() {
    for (const auto type : {VehicleType::SportsCar, VehicleType::Bus}) {
        const std::string_view name = type == VehicleType::SportsCar ? "Sports car" : "Bus";
        std::cout << name << '\n';
        for (const std::string_view command : {"M", "L", "R", "MLMR"}) {
            Executor car(type);
            car.Execute(command);
            PrintPose(command, car);
        }
    }
    return 0;
}
