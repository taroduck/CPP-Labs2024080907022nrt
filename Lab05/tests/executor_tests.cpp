#include "../Executor.h"

#include <cstddef>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr VehicleType kTypes[] = {VehicleType::SportsCar, VehicleType::Bus};
constexpr std::string_view kHeadings = "NESW";
constexpr Pose kSportsMove[] = {
    {0, 2, 'N'}, {2, 0, 'E'}, {0, -2, 'S'}, {-2, 0, 'W'}
};
constexpr Pose kBusMove[] = {
    {0, 1, 'N'}, {1, 0, 'E'}, {0, -1, 'S'}, {-1, 0, 'W'}
};
constexpr Pose kSportsLeft[] = {
    {-1, 0, 'W'}, {0, 1, 'N'}, {1, 0, 'E'}, {0, -1, 'S'}
};
constexpr Pose kSportsRight[] = {
    {1, 0, 'E'}, {0, -1, 'S'}, {-1, 0, 'W'}, {0, 1, 'N'}
};
constexpr Pose kBusLeft[] = {
    {0, 1, 'W'}, {1, 0, 'N'}, {0, -1, 'E'}, {-1, 0, 'S'}
};
constexpr Pose kBusRight[] = {
    {0, 1, 'E'}, {1, 0, 'S'}, {0, -1, 'W'}, {-1, 0, 'N'}
};

void Require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

std::string Describe(Pose pose) {
    return "(" + std::to_string(pose.x) + ", " + std::to_string(pose.y)
           + ", " + std::string(1, pose.heading) + ")";
}

void ExpectPose(const Executor& car, Pose expected) {
    const Pose actual = car.Query();
    Require(actual.x == expected.x && actual.y == expected.y
                && actual.heading == expected.heading,
            "expected " + Describe(expected) + ", got " + Describe(actual));
}

void ExpectSame(const Executor& actual, const Executor& expected) {
    ExpectPose(actual, expected.Query());
    Require(actual.Type() == expected.Type(), "vehicle type differs");
}

void DefaultSportsCar() {
    const Executor car(VehicleType::SportsCar);
    ExpectPose(car, {0, 0, 'N'});
    Require(car.Type() == VehicleType::SportsCar, "incorrect sports car type");
}

void DefaultBus() {
    const Executor car(VehicleType::Bus);
    ExpectPose(car, {0, 0, 'N'});
    Require(car.Type() == VehicleType::Bus, "incorrect bus type");
}

void CustomInitialization() {
    for (const auto type : kTypes) {
        for (const char heading : kHeadings) {
            const Executor car(type, -8, 15, heading);
            ExpectPose(car, {-8, 15, heading});
            Require(car.Type() == type, "incorrect custom vehicle type");
        }
    }
}

void CheckDirections(VehicleType type, char command, const Pose* offsets) {
    for (std::size_t i = 0; i < kHeadings.size(); ++i) {
        Executor car(type, 3, -4, kHeadings[i]);
        car.Execute(command);
        ExpectPose(car, {3 + offsets[i].x, -4 + offsets[i].y, offsets[i].heading});
    }
}

void SportsMove() { CheckDirections(VehicleType::SportsCar, 'M', kSportsMove); }
void BusMove() { CheckDirections(VehicleType::Bus, 'M', kBusMove); }
void SportsLeft() { CheckDirections(VehicleType::SportsCar, 'L', kSportsLeft); }
void SportsRight() { CheckDirections(VehicleType::SportsCar, 'R', kSportsRight); }
void BusLeft() { CheckDirections(VehicleType::Bus, 'L', kBusLeft); }
void BusRight() { CheckDirections(VehicleType::Bus, 'R', kBusRight); }

void RepeatedMoves() {
    for (const auto type : kTypes) {
        Executor car(type);
        car.Execute("MMMMM");
        ExpectPose(car, {0, type == VehicleType::SportsCar ? 10 : 5, 'N'});
    }
}

void FourTurnsClosePath() {
    for (const auto type : kTypes) {
        for (const char heading : kHeadings) {
            for (const std::string_view turns : {"LLLL", "RRRR"}) {
                Executor car(type, -4, 9, heading);
                car.Execute(turns);
                ExpectPose(car, {-4, 9, heading});
            }
        }
    }
}

void MixedBatch() {
    Executor sports(VehicleType::SportsCar);
    Executor bus(VehicleType::Bus);
    sports.Execute("MLMR");
    bus.Execute("MLMR");
    ExpectPose(sports, {-3, 3, 'N'});
    ExpectPose(bus, {-2, 2, 'N'});

    sports.Initialize(-5, -7, 'E');
    bus.Initialize(-5, -7, 'E');
    sports.Execute("MLMR");
    bus.Execute("MLMR");
    ExpectPose(sports, {-2, -4, 'E'});
    ExpectPose(bus, {-3, -5, 'E'});
}

void SingleCommandsMatchBatch() {
    constexpr std::string_view commands = "MLMRRMLMMR";
    for (const auto type : kTypes) {
        for (const char heading : kHeadings) {
            Executor single(type, 3, -5, heading);
            Executor batch(type, 3, -5, heading);
            for (const char command : commands) single.Execute(command);
            batch.Execute(commands);
            ExpectSame(single, batch);
        }
    }
}

void SuccessiveBatches() {
    for (const auto type : kTypes) {
        Executor car(type);
        car.Execute("ML");
        ExpectPose(car, type == VehicleType::SportsCar ? Pose{-1, 2, 'W'}
                                                       : Pose{0, 2, 'W'});
        car.Execute("MR");
        ExpectPose(car, type == VehicleType::SportsCar ? Pose{-3, 3, 'N'}
                                                       : Pose{-2, 2, 'N'});
    }
}

void InitializeKeepsType() {
    for (const auto type : kTypes) {
        Executor car(type);
        car.Execute("MLMR");
        car.Initialize(7, -6, 'S');
        ExpectPose(car, {7, -6, 'S'});
        Require(car.Type() == type, "reset changed vehicle type");
        car.Execute('M');
        ExpectPose(car, {7, type == VehicleType::SportsCar ? -8 : -7, 'S'});
        car.Initialize();
        ExpectPose(car, {0, 0, 'N'});
        car.Execute('M');
        ExpectPose(car, {0, type == VehicleType::SportsCar ? 2 : 1, 'N'});
    }
}

void QueryReturnsSnapshot() {
    for (const auto type : kTypes) {
        const Executor car(type, 2, 4, 'E');
        Pose snapshot = car.Query();
        snapshot.x = 99;
        snapshot.heading = 'S';
        Require(snapshot.x == 99 && snapshot.heading == 'S', "snapshot is not writable");
        ExpectPose(car, {2, 4, 'E'});
    }
}

void EmptyBatch() {
    for (const auto type : kTypes) {
        Executor car(type, -2, 7, 'W');
        car.Execute("");
        ExpectPose(car, {-2, 7, 'W'});
    }
}

void Int32Boundaries() {
    constexpr auto low = std::numeric_limits<std::int32_t>::min();
    constexpr auto high = std::numeric_limits<std::int32_t>::max();
    for (const auto type : kTypes) {
        const int distance = type == VehicleType::SportsCar ? 2 : 1;
        const Pose starts[] = {
            {0, high - distance, 'N'}, {high - distance, 0, 'E'},
            {0, low + distance, 'S'}, {low + distance, 0, 'W'}
        };
        constexpr Pose ends[] = {
            {0, high, 'N'}, {high, 0, 'E'}, {0, low, 'S'}, {low, 0, 'W'}
        };
        for (std::size_t i = 0; i < 4; ++i) {
            Executor car(type, starts[i].x, starts[i].y, starts[i].heading);
            car.Execute('M');
            ExpectPose(car, ends[i]);
        }
        for (std::size_t i = 0; i < 4; ++i) {
            for (const char command : std::string_view("LR")) {
                const Pose* offsets = type == VehicleType::SportsCar
                    ? (command == 'L' ? kSportsLeft : kSportsRight)
                    : (command == 'L' ? kBusLeft : kBusRight);
                Executor car(type, low + 2, high - 2, kHeadings[i]);
                car.Execute(command);
                ExpectPose(car, {low + 2 + offsets[i].x, high - 2 + offsets[i].y,
                                 offsets[i].heading});
            }
        }
    }
}

void IndependentInstances() {
    Executor sports(VehicleType::SportsCar);
    Executor bus(VehicleType::Bus);
    sports.Execute("MLMR");
    ExpectPose(bus, {0, 0, 'N'});
    bus.Execute("RM");
    ExpectPose(bus, {1, 1, 'E'});
    ExpectPose(sports, {-3, 3, 'N'});
    bus.Initialize();
    ExpectPose(sports, {-3, 3, 'N'});
}

// Expand the specification into physical moves and rotations independently.
struct Reference {
    std::int32_t x = 7;
    std::int32_t y = -3;
    int direction = 0;
};

void ReferenceExecute(Reference& model, VehicleType type, char command) {
    constexpr std::string_view actions[2][3] = {
        {"MM", "LM", "RM"}, {"M", "ML", "MR"}
    };
    constexpr int dx[] = {0, 1, 0, -1};
    constexpr int dy[] = {1, 0, -1, 0};
    const int vehicle = type == VehicleType::SportsCar ? 0 : 1;
    const int index = command == 'M' ? 0 : (command == 'L' ? 1 : 2);
    for (const char primitive : actions[vehicle][index]) {
        if (primitive == 'M') {
            model.x += dx[model.direction];
            model.y += dy[model.direction];
        } else {
            model.direction = (model.direction + (primitive == 'L' ? 3 : 1)) % 4;
        }
    }
}

void CheckSequences(const Executor& car, const Reference& model, int remaining,
                    char initialHeading, const std::string& sequence, int& checked) {
    try {
        ExpectPose(car, {model.x, model.y,
                        kHeadings[static_cast<std::size_t>(model.direction)]});
        Executor batch(car.Type(), 7, -3, initialHeading);
        batch.Execute(sequence);
        ExpectSame(car, batch);
    } catch (const std::exception& error) {
        const std::string vehicle = car.Type() == VehicleType::SportsCar ? "SportsCar" : "Bus";
        throw std::runtime_error(vehicle + " " + initialHeading + " '" + sequence
                                 + "': " + error.what());
    }
    ++checked;
    if (remaining == 0) return;
    for (const char command : std::string_view("MLR")) {
        Executor next = car;
        Reference nextModel = model;
        next.Execute(command);
        ReferenceExecute(nextModel, car.Type(), command);
        CheckSequences(next, nextModel, remaining - 1, initialHeading,
                       sequence + command, checked);
    }
}

void ExhaustiveSequences() {
    int checked = 0;
    for (const auto type : kTypes) {
        for (std::size_t i = 0; i < kHeadings.size(); ++i) {
            Executor car(type, 7, -3, kHeadings[i]);
            Reference model;
            model.direction = static_cast<int>(i);
            CheckSequences(car, model, 6, kHeadings[i], "", checked);
        }
    }
    Require(checked == 8744, "incomplete reference coverage");
    std::cout << "  checked " << checked << " reference sequences\n";
}

struct Test {
    const char* name;
    void (*run)();
};
}

int main() {
    const Test tests[] = {
        {"sports car default initialization", DefaultSportsCar},
        {"bus default initialization", DefaultBus},
        {"custom initialization", CustomInitialization},
        {"sports car M in all directions", SportsMove},
        {"bus M in all directions", BusMove},
        {"sports car L turns before moving", SportsLeft},
        {"sports car R turns before moving", SportsRight},
        {"bus L moves before turning", BusLeft},
        {"bus R moves before turning", BusRight},
        {"repeated moves", RepeatedMoves},
        {"four turns close a path", FourTurnsClosePath},
        {"mixed batch and negative coordinates", MixedBatch},
        {"single commands match batch", SingleCommandsMatchBatch},
        {"successive batches keep state", SuccessiveBatches},
        {"initialization keeps vehicle type", InitializeKeepsType},
        {"query returns snapshot", QueryReturnsSnapshot},
        {"empty batch", EmptyBatch},
        {"legal int32 boundary movements", Int32Boundaries},
        {"independent vehicle instances", IndependentInstances},
        {"exhaustive sequences match reference", ExhaustiveSequences}
    };
    std::size_t passed = 0;
    for (const Test& test : tests) {
        try {
            test.run();
            ++passed;
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
        }
    }
    constexpr std::size_t total = sizeof(tests) / sizeof(tests[0]);
    std::cout << passed << '/' << total << " tests passed.\n";
    return passed == total ? 0 : 1;
}
