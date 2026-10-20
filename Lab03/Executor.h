#ifndef LAB03_EXECUTOR_H
#define LAB03_EXECUTOR_H

#include <cstdint>
#include <string_view>

struct Pose {
    std::int32_t x;
    std::int32_t y;
    char heading;
};

class Executor {
public:
    explicit Executor(std::int32_t x = 0, std::int32_t y = 0,
                      char heading = 'N') noexcept;

    // Initialization restores normal speed and forward movement.
    void Initialize(std::int32_t x = 0, std::int32_t y = 0,
                    char heading = 'N') noexcept;
    void Execute(char command) noexcept;
    void Execute(std::string_view commands) noexcept;
    [[nodiscard]] Pose Query() const noexcept;
    [[nodiscard]] bool IsAccelerated() const noexcept;
    [[nodiscard]] bool IsReversing() const noexcept;

private:
    void MoveOneStep() noexcept;

    Pose pose_;
    bool accelerated_ = false;
    bool reversing_ = false;
};

#endif
