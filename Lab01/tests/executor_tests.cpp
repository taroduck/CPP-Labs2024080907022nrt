#include "Executor.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
std::string Describe(const Pose& pose) {
    return "(" + std::to_string(pose.x) + ", " + std::to_string(pose.y)
           + ", " + std::string(1, pose.heading) + ")";
}

void ExpectPose(const Pose& actual, const Pose& expected) {
    if (actual.x != expected.x || actual.y != expected.y
        || actual.heading != expected.heading) {
        throw std::runtime_error("expected " + Describe(expected)
                                 + ", got " + Describe(actual));
    }
}

void DefaultInitialization() {
    const Executor car;
    ExpectPose(car.Query(), {0, 0, 'N'});
}

void CustomInitialization() {
    for (const char heading : std::string("NSEW")) {
        const Executor car(12, -7, heading);
        ExpectPose(car.Query(), {12, -7, heading});
    }
}

void ResetToCustomPosition() {
    Executor car;
    car.Execute("MRM");
    car.Initialize(-8, 15, 'W');
    ExpectPose(car.Query(), {-8, 15, 'W'});
    car.Execute('M');
    ExpectPose(car.Query(), {-9, 15, 'W'});
}

void ResetToDefaultPosition() {
    Executor car(5, -3, 'S');
    car.Execute("MLM");
    car.Initialize();
    ExpectPose(car.Query(), {0, 0, 'N'});
}

void MoveInAllDirections() {
    const Pose expected[] = {
        {5, -2, 'N'}, {5, -4, 'S'}, {6, -3, 'E'}, {4, -3, 'W'}
    };
    for (const Pose& pose : expected) {
        Executor car(5, -3, pose.heading);
        car.Execute('M');
        ExpectPose(car.Query(), pose);
    }
}

void TurnLeftInAllDirections() {
    const char headings[] = {'N', 'W', 'S', 'E'};
    const char expected[] = {'W', 'S', 'E', 'N'};
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute('L');
        ExpectPose(car.Query(), {5, -3, expected[i]});
    }
}

void TurnRightInAllDirections() {
    const char headings[] = {'N', 'E', 'S', 'W'};
    const char expected[] = {'E', 'S', 'W', 'N'};
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute('R');
        ExpectPose(car.Query(), {5, -3, expected[i]});
    }
}

void FullRotationKeepsPosition() {
    for (const char heading : std::string("NSEW")) {
        Executor car(-4, 9, heading);
        car.Execute("LLLL");
        ExpectPose(car.Query(), {-4, 9, heading});
        car.Execute("RRRR");
        ExpectPose(car.Query(), {-4, 9, heading});
    }
}

void EmptyBatchKeepsPosition() {
    Executor car(-2, 7, 'W');
    car.Execute("");
    ExpectPose(car.Query(), {-2, 7, 'W'});
}

void RepeatedMoves() {
    Executor car;
    car.Execute("MMMMM");
    ExpectPose(car.Query(), {0, 5, 'N'});
}

void MixedBatchFromDefault() {
    Executor car;
    car.Execute("MMRMMLM");
    ExpectPose(car.Query(), {2, 3, 'N'});
}

void MixedBatchFromCustomPosition() {
    Executor car(1, 2, 'E');
    car.Execute("MMLM");
    ExpectPose(car.Query(), {3, 3, 'N'});
}

void MoveAcrossNegativeCoordinates() {
    Executor car(-1, -1, 'S');
    car.Execute("MMRMM");
    ExpectPose(car.Query(), {-3, -3, 'W'});
}

void ClosedPathReturnsToStart() {
    Executor car(3, -5, 'W');
    car.Execute("MMRMMRMMRMMR");
    ExpectPose(car.Query(), {3, -5, 'W'});
}

void SuccessiveBatchesKeepState() {
    Executor car;
    car.Execute("MMR");
    ExpectPose(car.Query(), {0, 2, 'E'});
    car.Execute("MMLM");
    ExpectPose(car.Query(), {2, 3, 'N'});
}

void SingleCommandsMatchBatch() {
    Executor single(-3, 8, 'E');
    Executor batch(-3, 8, 'E');
    const std::string commands = "MLMRMMRMLM";
    for (const char command : commands) {
        single.Execute(command);
    }
    batch.Execute(commands);
    ExpectPose(single.Query(), batch.Query());
}

void QueryReturnsIndependentSnapshot() {
    Executor car(2, 4, 'S');
    Pose snapshot = car.Query();
    snapshot.x = 99;
    snapshot.y = -99;
    snapshot.heading = 'E';
    ExpectPose(snapshot, {99, -99, 'E'});
    ExpectPose(car.Query(), {2, 4, 'S'});
    ExpectPose(car.Query(), {2, 4, 'S'});
}

void Int32BoundariesWithoutOverflow() {
    constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
    Executor car(minimum, maximum, 'W');
    ExpectPose(car.Query(), {minimum, maximum, 'W'});
    car.Execute("LR");
    ExpectPose(car.Query(), {minimum, maximum, 'W'});

    car.Initialize(maximum - 1, 0, 'E');
    car.Execute('M');
    ExpectPose(car.Query(), {maximum, 0, 'E'});
    car.Initialize(minimum + 1, 0, 'W');
    car.Execute('M');
    ExpectPose(car.Query(), {minimum, 0, 'W'});
    car.Initialize(0, maximum - 1, 'N');
    car.Execute('M');
    ExpectPose(car.Query(), {0, maximum, 'N'});
    car.Initialize(0, minimum + 1, 'S');
    car.Execute('M');
    ExpectPose(car.Query(), {0, minimum, 'S'});
}

struct TestCase {
    const char* name;
    void (*run)();
};
}

int main() {
    const TestCase tests[] = {
        {"default initialization", DefaultInitialization},
        {"custom initialization", CustomInitialization},
        {"reset to custom position", ResetToCustomPosition},
        {"reset to default position", ResetToDefaultPosition},
        {"move in all directions", MoveInAllDirections},
        {"turn left in all directions", TurnLeftInAllDirections},
        {"turn right in all directions", TurnRightInAllDirections},
        {"full rotation keeps position", FullRotationKeepsPosition},
        {"empty batch keeps position", EmptyBatchKeepsPosition},
        {"repeated moves", RepeatedMoves},
        {"mixed batch from default", MixedBatchFromDefault},
        {"mixed batch from custom position", MixedBatchFromCustomPosition},
        {"move across negative coordinates", MoveAcrossNegativeCoordinates},
        {"closed path returns to start", ClosedPathReturnsToStart},
        {"successive batches keep state", SuccessiveBatchesKeepState},
        {"single commands match batch", SingleCommandsMatchBatch},
        {"query returns independent snapshot", QueryReturnsIndependentSnapshot},
        {"int32 boundaries without overflow", Int32BoundariesWithoutOverflow}
    };

    int total = 0;
    int failures = 0;
    for (const TestCase& test : tests) {
        ++total;
        try {
            test.run();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
        }
    }
    std::cout << '\n' << total - failures << '/' << total << " tests passed.\n";
    return failures == 0 ? 0 : 1;
}
