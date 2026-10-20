#include "Executor.h"

namespace {
constexpr std::string_view kHeadings = "NESW";
}

Executor::Executor(std::int32_t x, std::int32_t y, char heading) noexcept
    : pose_{x, y, heading} {}

void Executor::Initialize(std::int32_t x, std::int32_t y, char heading) noexcept {
    pose_ = {x, y, heading};
    accelerated_ = false;
}

void Executor::MoveForwardOneStep() noexcept {
    switch (pose_.heading) {
    case 'N': ++pose_.y; break;
    case 'S': --pose_.y; break;
    case 'E': ++pose_.x; break;
    case 'W': --pose_.x; break;
    }
}

void Executor::Execute(char command) noexcept {
    switch (command) {
    case 'F':
        accelerated_ = !accelerated_;
        break;
    case 'M':
        MoveForwardOneStep();
        if (accelerated_) {
            MoveForwardOneStep();
        }
        break;
    case 'L':
        if (accelerated_) {
            MoveForwardOneStep();
        }
        pose_.heading = kHeadings[(kHeadings.find(pose_.heading) + 3) % 4];
        break;
    case 'R':
        if (accelerated_) {
            MoveForwardOneStep();
        }
        pose_.heading = kHeadings[(kHeadings.find(pose_.heading) + 1) % 4];
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

bool Executor::IsAccelerated() const noexcept {
    return accelerated_;
}
