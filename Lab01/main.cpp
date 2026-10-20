#include "Executor.h"

#include <iostream>

namespace {
void PrintPose(const char* label, const Pose& pose) {
    std::cout << label << "(" << pose.x << ", " << pose.y << ", "
              << pose.heading << ")\n";
}
}

int main() {
    Executor car;
    PrintPose("Default position: ", car.Query());
    car.Execute("MMRMMLM");
    PrintPose("After MMRMMLM:    ", car.Query());

    car.Initialize(1, 2, 'E');
    PrintPose("Reset position:   ", car.Query());
    car.Execute("MMLM");
    PrintPose("After MMLM:       ", car.Query());
    return 0;
}
