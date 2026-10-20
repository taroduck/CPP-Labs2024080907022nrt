#include "Executor.h"

Executor::Executor(VehicleType type, std::int32_t x, std::int32_t y,
                   char heading) noexcept
    : type_(type), pose_{x, y, heading} {}

void Executor::Initialize(std::int32_t x, std::int32_t y, char heading) noexcept {
    pose_ = {x, y, heading};
}

void Executor::MoveOneGrid() noexcept {
    switch (pose_.heading) {
    case 'N': ++pose_.y; break;
    case 'E': ++pose_.x; break;
    case 'S': --pose_.y; break;
    case 'W': --pose_.x; break;
    }
}

void Executor::RotateQuarterTurn(bool left) noexcept {
    switch (pose_.heading) {
    case 'N': pose_.heading = left ? 'W' : 'E'; break;
    case 'E': pose_.heading = left ? 'N' : 'S'; break;
    case 'S': pose_.heading = left ? 'E' : 'W'; break;
    case 'W': pose_.heading = left ? 'S' : 'N'; break;
    }
}

void Executor::Execute(char command) noexcept {
    switch (command) {
    case 'M':
        MoveOneGrid();
        if (type_ == VehicleType::SportsCar) {
            MoveOneGrid();
        }
        break;
    case 'L':
    case 'R':
        if (type_ == VehicleType::SportsCar) {
            RotateQuarterTurn(command == 'L');
            MoveOneGrid();
        } else {
            MoveOneGrid();
            RotateQuarterTurn(command == 'L');
        }
        break;
    }
}

void Executor::Execute(std::string_view commands) noexcept {
    for (const char command : commands) {
        Execute(command);
    }
}

Pose Executor::Query() const noexcept {
    return pose_;
}

VehicleType Executor::Type() const noexcept {
    return type_;
}
