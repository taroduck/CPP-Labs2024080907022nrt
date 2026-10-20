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

void ExpectAcceleration(const Executor& car, bool expected) {
    if (car.IsAccelerated() != expected) {
        throw std::runtime_error(expected ? "expected accelerated state"
                                          : "expected normal state");
    }
}

void ToggleOnKeepsPose() {
    Executor car(-3, 7, 'W');
    ExpectAcceleration(car, false);
    car.Execute('F');
    ExpectPose(car.Query(), {-3, 7, 'W'});
    ExpectAcceleration(car, true);
}

void ToggleOffKeepsPose() {
    Executor car(4, -8, 'S');
    car.Execute("FM");
    ExpectPose(car.Query(), {4, -10, 'S'});
    car.Execute('F');
    ExpectPose(car.Query(), {4, -10, 'S'});
    ExpectAcceleration(car, false);
}

void AcceleratedMoveInAllDirections() {
    const Pose expected[] = {
        {5, -1, 'N'}, {5, -5, 'S'}, {7, -3, 'E'}, {3, -3, 'W'}
    };
    for (const Pose& pose : expected) {
        Executor car(5, -3, pose.heading);
        car.Execute("FM");
        ExpectPose(car.Query(), pose);
        ExpectAcceleration(car, true);
    }
}

void AcceleratedLeftInAllDirections() {
    const char headings[] = {'N', 'W', 'S', 'E'};
    const Pose expected[] = {
        {5, -2, 'W'}, {4, -3, 'S'}, {5, -4, 'E'}, {6, -3, 'N'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute("FL");
        ExpectPose(car.Query(), expected[i]);
        ExpectAcceleration(car, true);
    }
}

void AcceleratedRightInAllDirections() {
    const char headings[] = {'N', 'E', 'S', 'W'};
    const Pose expected[] = {
        {5, -2, 'E'}, {6, -3, 'S'}, {5, -4, 'W'}, {4, -3, 'N'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute("FR");
        ExpectPose(car.Query(), expected[i]);
        ExpectAcceleration(car, true);
    }
}

void AccelerationPersistsAcrossCommands() {
    Executor car;
    car.Execute("FMMLMRM");
    ExpectPose(car.Query(), {-3, 7, 'N'});
    ExpectAcceleration(car, true);
}

void CancelRestoresNormalMoves() {
    Executor car;
    car.Execute("FMFM");
    ExpectPose(car.Query(), {0, 3, 'N'});
    ExpectAcceleration(car, false);
}

void CancelRestoresNormalTurns() {
    Executor car;
    car.Execute("FLFL");
    ExpectPose(car.Query(), {0, 1, 'S'});
    ExpectAcceleration(car, false);
    car.Execute('R');
    ExpectPose(car.Query(), {0, 1, 'W'});
}

void RepeatedToggles() {
    Executor car;
    for (int i = 1; i <= 6; ++i) {
        car.Execute('F');
        ExpectAcceleration(car, i % 2 != 0);
        ExpectPose(car.Query(), {0, 0, 'N'});
    }
    car.Execute('M');
    ExpectPose(car.Query(), {0, 1, 'N'});
    car.Execute("FFF");
    ExpectAcceleration(car, true);
    car.Execute('M');
    ExpectPose(car.Query(), {0, 3, 'N'});
}

void AccelerationPersistsAcrossBatches() {
    Executor car;
    car.Execute("F");
    car.Execute("MM");
    ExpectPose(car.Query(), {0, 4, 'N'});
    car.Execute("RM");
    ExpectPose(car.Query(), {2, 5, 'E'});
    ExpectAcceleration(car, true);
}

void MixedNormalAndAcceleratedBatch() {
    Executor car;
    car.Execute("MFMRMFLM");
    ExpectPose(car.Query(), {2, 5, 'N'});
    ExpectAcceleration(car, false);
}

void EmptyBatchKeepsAcceleration() {
    Executor car;
    car.Execute('F');
    car.Execute("");
    ExpectPose(car.Query(), {0, 0, 'N'});
    ExpectAcceleration(car, true);
    car.Execute('M');
    ExpectPose(car.Query(), {0, 2, 'N'});
}

void InitializationClearsAcceleration() {
    Executor car;
    car.Execute("FM");
    car.Initialize(-2, 4, 'E');
    ExpectPose(car.Query(), {-2, 4, 'E'});
    ExpectAcceleration(car, false);
    car.Execute('M');
    ExpectPose(car.Query(), {-1, 4, 'E'});
    car.Execute('F');
    car.Initialize();
    ExpectPose(car.Query(), {0, 0, 'N'});
    ExpectAcceleration(car, false);
    car.Execute('M');
    ExpectPose(car.Query(), {0, 1, 'N'});
}

void InstancesHaveIndependentAcceleration() {
    Executor first;
    Executor second;
    first.Execute("FM");
    second.Execute('M');
    ExpectPose(first.Query(), {0, 2, 'N'});
    ExpectPose(second.Query(), {0, 1, 'N'});
    ExpectAcceleration(first, true);
    ExpectAcceleration(second, false);
    second.Execute('F');
    first.Execute('F');
    ExpectAcceleration(first, false);
    ExpectAcceleration(second, true);
}

void SingleCommandsMatchAcceleratedBatch() {
    Executor single(-3, 8, 'E');
    Executor batch(-3, 8, 'E');
    const std::string commands = "FMLMRFFMMRFLM";
    for (const char command : commands) {
        single.Execute(command);
    }
    batch.Execute(commands);
    ExpectPose(single.Query(), batch.Query());
    ExpectAcceleration(single, batch.IsAccelerated());
}

void AcceleratedMovesNearInt32Boundaries() {
    constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
    const Pose starts[] = {
        {maximum - 2, 0, 'E'}, {minimum + 2, 0, 'W'},
        {0, maximum - 2, 'N'}, {0, minimum + 2, 'S'}
    };
    const Pose expected[] = {
        {maximum, 0, 'E'}, {minimum, 0, 'W'},
        {0, maximum, 'N'}, {0, minimum, 'S'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car(starts[i].x, starts[i].y, starts[i].heading);
        car.Execute("FM");
        ExpectPose(car.Query(), expected[i]);
    }
}

void AcceleratedTurnsNearInt32Boundaries() {
    constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
    Executor car(maximum - 1, 0, 'E');
    car.Execute("FL");
    ExpectPose(car.Query(), {maximum, 0, 'N'});
    car.Initialize(minimum + 1, 0, 'W');
    car.Execute("FR");
    ExpectPose(car.Query(), {minimum, 0, 'N'});
    car.Initialize(0, maximum - 1, 'N');
    car.Execute("FR");
    ExpectPose(car.Query(), {0, maximum, 'E'});
    car.Initialize(0, minimum + 1, 'S');
    car.Execute("FL");
    ExpectPose(car.Query(), {0, minimum, 'E'});
}

void FourAcceleratedTurnsClosePath() {
    for (const char heading : std::string("NSEW")) {
        Executor car(3, -5, heading);
        car.Execute("FLLLL");
        ExpectPose(car.Query(), {3, -5, heading});
        ExpectAcceleration(car, true);
        car.Execute("RRRR");
        ExpectPose(car.Query(), {3, -5, heading});
        ExpectAcceleration(car, true);
    }
}

struct TestCase {
    const char* name;
    void (*run)();
};
}

int main() {
    const TestCase tests[] = {
        {"toggle on keeps pose", ToggleOnKeepsPose},
        {"toggle off keeps pose", ToggleOffKeepsPose},
        {"accelerated move in all directions", AcceleratedMoveInAllDirections},
        {"accelerated left in all directions", AcceleratedLeftInAllDirections},
        {"accelerated right in all directions", AcceleratedRightInAllDirections},
        {"acceleration persists across commands", AccelerationPersistsAcrossCommands},
        {"cancel restores normal moves", CancelRestoresNormalMoves},
        {"cancel restores normal turns", CancelRestoresNormalTurns},
        {"repeated toggles", RepeatedToggles},
        {"acceleration persists across batches", AccelerationPersistsAcrossBatches},
        {"mixed normal and accelerated batch", MixedNormalAndAcceleratedBatch},
        {"empty batch keeps acceleration", EmptyBatchKeepsAcceleration},
        {"initialization clears acceleration", InitializationClearsAcceleration},
        {"instances have independent acceleration", InstancesHaveIndependentAcceleration},
        {"single commands match accelerated batch", SingleCommandsMatchAcceleratedBatch},
        {"accelerated moves near int32 boundaries", AcceleratedMovesNearInt32Boundaries},
        {"accelerated turns near int32 boundaries", AcceleratedTurnsNearInt32Boundaries},
        {"four accelerated turns close path", FourAcceleratedTurnsClosePath},
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
