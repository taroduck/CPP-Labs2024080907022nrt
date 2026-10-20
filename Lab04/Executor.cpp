#include "Executor.h"

Executor::Executor(std::int32_t x, std::int32_t y, char heading) noexcept
    : pose_{x, y, heading} {}

void Executor::Initialize(std::int32_t x, std::int32_t y, char heading) noexcept {
    pose_ = {x, y, heading};
    accelerated_ = false;
    reversing_ = false;
}

void Executor::MoveOneGrid(bool backward) noexcept {
    const std::int32_t distance = backward ? -1 : 1;
    switch (pose_.heading) {
    case 'N': pose_.y += distance; break;
    case 'E': pose_.x += distance; break;
    case 'S': pose_.y -= distance; break;
    case 'W': pose_.x -= distance; break;
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

void Executor::TurnRound() noexcept {
    // TR always moves physically forward and turns physically left.
    if (accelerated_) {
        MoveOneGrid(false);
    }
    RotateQuarterTurn(true);
    MoveOneGrid(false);
    RotateQuarterTurn(true);
}

void Executor::Execute(char command) noexcept {
    switch (command) {
    case 'F':
        accelerated_ = !accelerated_;
        break;
    case 'B':
        reversing_ = !reversing_;
        break;
    case 'M':
        MoveOneGrid(reversing_);
        if (accelerated_) {
            MoveOneGrid(reversing_);
        }
        break;
    case 'L':
    case 'R':
        if (accelerated_) {
            MoveOneGrid(reversing_);
        }
        RotateQuarterTurn((command == 'L') != reversing_);
        break;
    }
}

void Executor::Execute(std::string_view commands) noexcept {
    for (std::string_view::size_type i = 0; i < commands.size(); ++i) {
        // Consume TR as one token within this string call.
        if (commands.substr(i, 2) == "TR") {
            TurnRound();
            ++i;
        } else {
            Execute(commands[i]);
        }
    }
}

Pose Executor::Query() const noexcept {
    return pose_;
}

bool Executor::IsAccelerated() const noexcept {
    return accelerated_;
}

bool Executor::IsReversing() const noexcept {
    return reversing_;
}
