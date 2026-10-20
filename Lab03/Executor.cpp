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

void Executor::MoveOneStep() noexcept {
    const std::int32_t step = reversing_ ? -1 : 1;
    switch (pose_.heading) {
    case 'N': pose_.y += step; break;
    case 'S': pose_.y -= step; break;
    case 'E': pose_.x += step; break;
    case 'W': pose_.x -= step; break;
    }
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
        MoveOneStep();
        if (accelerated_) {
            MoveOneStep();
        }
        break;
    case 'L':
    case 'R': {
        if (accelerated_) {
            MoveOneStep();
        }
        // Reversing swaps the meanings of left and right.
        const bool turnLeft = (command == 'L') != reversing_;
        const auto offset = turnLeft ? 3u : 1u;
        pose_.heading = kHeadings[(kHeadings.find(pose_.heading) + offset) % 4];
        break;
    }
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

bool Executor::IsAccelerated() const noexcept {
    return accelerated_;
}

bool Executor::IsReversing() const noexcept {
    return reversing_;
}
