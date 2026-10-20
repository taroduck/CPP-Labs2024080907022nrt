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
    car.Execute('B');
    PrintState("After B:       ", car);
    car.Execute('F');
    PrintState("After F:       ", car);
    car.Execute('M');
    PrintState("After M:       ", car);
    car.Execute('L');
    PrintState("After L:       ", car);
    car.Execute('R');
    PrintState("After R:       ", car);
    car.Execute('B');
    PrintState("After B:       ", car);
    car.Execute('M');
    PrintState("After M:       ", car);
    car.Execute('F');
    PrintState("After F:       ", car);
    car.Execute('M');
    PrintState("After M:       ", car);

    car.Initialize();
    car.Execute("MBMFLMRBMFML");
    PrintState("Mixed batch:   ", car);
    return 0;
}
