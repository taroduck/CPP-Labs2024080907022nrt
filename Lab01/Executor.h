#ifndef LAB01_EXECUTOR_H
#define LAB01_EXECUTOR_H

#include <cstdint>
#include <string_view>

struct Pose {
    std::int32_t x;
    std::int32_t y;
    char heading;
};

class Executor {
public:
    // North increases y; east increases x. Inputs are valid by contract.
    explicit Executor(std::int32_t x = 0, std::int32_t y = 0,
                      char heading = 'N') noexcept;

    void Initialize(std::int32_t x = 0, std::int32_t y = 0,
                    char heading = 'N') noexcept;
    void Execute(char command) noexcept;
    void Execute(std::string_view commands) noexcept;
    [[nodiscard]] Pose Query() const noexcept;

private:
    Pose pose_;
};

#endif
