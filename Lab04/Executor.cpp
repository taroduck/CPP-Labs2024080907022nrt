#include "Executor.h"

namespace {
constexpr std::string_view kHeadings = "NESW";
}

Executor::Executor(std::int32_t x, std::int32_t y, char heading) noexcept
    : pose_{x, y, heading} {}

void Executor::Initialize(std::int32_t x, std::int32_t y, char heading) noexcept {
    pose_ = {x, y, heading};
    accelerated_ = false;
    reversing_ = false;
}

void Executor::MoveOneStep(bool reverse) noexcept {
    const std::int32_t step = reverse ? -1 : 1;
    switch (pose_.heading) {
    case 'N': pose_.y += step; break;
    case 'S': pose_.y -= step; break;
    case 'E': pose_.x += step; break;
    case 'W': pose_.x -= step; break;
    }
}

void Executor::Turn(bool left) noexcept {
    const auto offset = left ? 3u : 1u;
    pose_.heading = kHeadings[(kHeadings.find(pose_.heading) + offset) % 4];
}

void Executor::TurnRound() noexcept {
    if (accelerated_) {
        MoveOneStep(false);
    }
    Turn(true);
    MoveOneStep(false);
    Turn(true);
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
        MoveOneStep(reversing_);
        if (accelerated_) {
            MoveOneStep(reversing_);
        }
        break;
    case 'L':
    case 'R':
        if (accelerated_) {
            MoveOneStep(reversing_);
        }
        Turn((command == 'L') != reversing_);
        break;
    }
}

void Executor::Execute(std::string_view commands) noexcept {
    for (std::string_view::size_type index = 0; index < commands.size();) {
        // Valid input guarantees that T is immediately followed by R.
        if (commands[index] == 'T') {
            TurnRound();
            index += 2;
        } else {
            Execute(commands[index]);
            ++index;
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
