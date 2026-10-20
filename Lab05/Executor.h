#ifndef LAB05_EXECUTOR_H
#define LAB05_EXECUTOR_H

#include <cstdint>
#include <string_view>

enum class VehicleType {
    SportsCar,
    Bus
};

struct Pose {
    std::int32_t x;
    std::int32_t y;
    char heading;
};

class Executor {
public:
    explicit Executor(VehicleType type, std::int32_t x = 0,
                      std::int32_t y = 0, char heading = 'N') noexcept;

    // Reset the pose while keeping the selected vehicle type.
    void Initialize(std::int32_t x = 0, std::int32_t y = 0,
                    char heading = 'N') noexcept;
    void Execute(char command) noexcept;
    void Execute(std::string_view commands) noexcept;
    [[nodiscard]] Pose Query() const noexcept;
    [[nodiscard]] VehicleType Type() const noexcept;

private:
    void MoveOneGrid() noexcept;
    void RotateQuarterTurn(bool left) noexcept;

    VehicleType type_;
    Pose pose_;
};

#endif
