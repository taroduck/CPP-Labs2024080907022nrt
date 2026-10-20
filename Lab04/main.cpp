#include "Executor.h"

#include <iostream>

namespace {
void PrintState(const char* label, const Executor& car) {
    const Pose pose = car.Query();
    std::cout << label << "(" << pose.x << ", " << pose.y << ", "
              << pose.heading << "), accelerated=" << std::boolalpha
              << car.IsAccelerated() << ", reversing=" << car.IsReversing() << '\n';
}
}

int main() {
    Executor car;
    PrintState("Initial:       ", car);
    car.Execute("TR");
    PrintState("Normal TR:     ", car);
    car.Initialize();
    car.Execute("BTR");
    PrintState("Reverse TR:    ", car);
    car.Initialize();
    car.Execute("FTR");
    PrintState("Fast TR:       ", car);
    car.Initialize();
    car.Execute("BFTR");
    PrintState("Fast reverse TR:", car);
    car.Execute('M');
    PrintState("Then M:        ", car);
    car.Initialize();
    car.Execute("BMFTRBMFTR");
    PrintState("Mixed batch:   ", car);
    return 0;
}
