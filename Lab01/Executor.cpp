#include "Executor.h"

namespace {
constexpr std::string_view kHeadings = "NESW";
}

Executor::Executor(std::int32_t x, std::int32_t y, char heading) noexcept
    : pose_{x, y, heading} {}

void Executor::Initialize(std::int32_t x, std::int32_t y, char heading) noexcept {
    pose_ = {x, y, heading};
}

void Executor::Execute(char command) noexcept {
    switch (command) {
    case 'M':
        switch (pose_.heading) {
        case 'N': ++pose_.y; break;
        case 'S': --pose_.y; break;
        case 'E': ++pose_.x; break;
        case 'W': --pose_.x; break;
        }
        break;
    case 'L':
        pose_.heading = kHeadings[(kHeadings.find(pose_.heading) + 3) % 4];
        break;
    case 'R':
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
