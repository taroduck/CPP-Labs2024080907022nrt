#include "Executor.h"

#include <iostream>
#include <string_view>

namespace {
void PrintState(std::string_view label, const Executor& car) {
    const Pose pose = car.Query();
    std::cout << label << ": (" << pose.x << ", " << pose.y << ", "
              << pose.heading << "), F=" << std::boolalpha
              << car.IsAccelerated() << ", B=" << car.IsReversing() << '\n';
}
}

int main() {
    for (const std::string_view commands : {"TR", "BTR", "FTR", "FBTR"}) {
        Executor car;
        car.Execute(commands);
        PrintState(commands, car);
    }

    Executor car;
    car.Execute("FBTR");
    car.Execute('M');
    PrintState("FBTR then M", car);
    car.Initialize();
    car.Execute("TRR");
    PrintState("TRR", car);
    return 0;
}
