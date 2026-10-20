#include "Executor.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

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


void ExpectReverse(const Executor& car, bool expected) {
    if (car.IsReversing() != expected) {
        throw std::runtime_error(expected ? "expected reversing state"
                                          : "expected forward state");
    }
}

void ReverseToggleKeepsPose() {
    Executor car(2, 7, 'N');
    ExpectReverse(car, false);
    car.Execute('B');
    ExpectPose(car.Query(), {2, 7, 'N'});
    ExpectReverse(car, true);
    car.Execute('M');
    ExpectPose(car.Query(), {2, 6, 'N'});
    car.Execute('B');
    ExpectPose(car.Query(), {2, 6, 'N'});
    ExpectReverse(car, false);
}

void ReverseMoveInAllDirections() {
    const Pose expected[] = {
        {5, -4, 'N'}, {5, -2, 'S'}, {4, -3, 'E'}, {6, -3, 'W'}
    };
    for (const Pose& pose : expected) {
        Executor car(5, -3, pose.heading);
        car.Execute("BM");
        ExpectPose(car.Query(), pose);
    }
}

void ReverseLeftInAllDirections() {
    const char headings[] = {'N', 'E', 'S', 'W'};
    const char expected[] = {'E', 'S', 'W', 'N'};
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute("BL");
        ExpectPose(car.Query(), {5, -3, expected[i]});
    }
}

void ReverseRightInAllDirections() {
    const char headings[] = {'N', 'E', 'S', 'W'};
    const char expected[] = {'W', 'N', 'E', 'S'};
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute("BR");
        ExpectPose(car.Query(), {5, -3, expected[i]});
    }
}

void AcceleratedReverseMoveInAllDirections() {
    const Pose expected[] = {
        {5, -5, 'N'}, {5, -1, 'S'}, {3, -3, 'E'}, {7, -3, 'W'}
    };
    for (const Pose& pose : expected) {
        Executor car(5, -3, pose.heading);
        car.Execute("BFM");
        ExpectPose(car.Query(), pose);
        ExpectReverse(car, true);
        ExpectAcceleration(car, true);
    }
}

void AcceleratedReverseLeftInAllDirections() {
    const char headings[] = {'N', 'E', 'S', 'W'};
    const Pose expected[] = {
        {5, -4, 'E'}, {4, -3, 'S'}, {5, -2, 'W'}, {6, -3, 'N'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute("BFL");
        ExpectPose(car.Query(), expected[i]);
    }
}

void AcceleratedReverseRightInAllDirections() {
    const char headings[] = {'N', 'E', 'S', 'W'};
    const Pose expected[] = {
        {5, -4, 'W'}, {4, -3, 'N'}, {5, -2, 'E'}, {6, -3, 'S'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute("BFR");
        ExpectPose(car.Query(), expected[i]);
    }
}

void CancellingModesIndependently() {
    Executor car;
    car.Execute("BFM");
    ExpectPose(car.Query(), {0, -2, 'N'});
    car.Execute('B');
    ExpectReverse(car, false);
    ExpectAcceleration(car, true);
    car.Execute('M');
    ExpectPose(car.Query(), {0, 0, 'N'});
    car.Execute("BF");
    ExpectReverse(car, true);
    ExpectAcceleration(car, false);
    car.Execute('M');
    ExpectPose(car.Query(), {0, -1, 'N'});
    car.Execute("BLM");
    ExpectPose(car.Query(), {-1, -1, 'W'});
    ExpectReverse(car, false);
    ExpectAcceleration(car, false);
}

void RepeatedReverseToggles() {
    Executor car;
    for (int i = 1; i <= 6; ++i) {
        car.Execute('B');
        ExpectReverse(car, i % 2 != 0);
        ExpectPose(car.Query(), {0, 0, 'N'});
    }
    car.Execute("BBM");
    ExpectPose(car.Query(), {0, 1, 'N'});
    car.Execute("BBBM");
    ExpectPose(car.Query(), {0, 0, 'N'});
    ExpectReverse(car, true);
}

void ReversePersistsAcrossBatches() {
    Executor car;
    car.Execute("B");
    car.Execute("MM");
    ExpectPose(car.Query(), {0, -2, 'N'});
    car.Execute("FL");
    ExpectPose(car.Query(), {0, -3, 'E'});
    car.Execute("RM");
    ExpectPose(car.Query(), {-1, -5, 'N'});
    ExpectReverse(car, true);
    ExpectAcceleration(car, true);
}

void MixedForwardReverseBatch() {
    Executor car;
    car.Execute("MBMFLMRBMFML");
    ExpectPose(car.Query(), {-3, 2, 'W'});
    ExpectReverse(car, false);
    ExpectAcceleration(car, false);
}

void InitializationClearsBothStates() {
    for (const std::string_view mode : {"B", "FB"}) {
        Executor car;
        car.Execute(mode);
        car.Initialize(-2, 4, 'E');
        ExpectPose(car.Query(), {-2, 4, 'E'});
        ExpectReverse(car, false);
        ExpectAcceleration(car, false);
        car.Execute('M');
        ExpectPose(car.Query(), {-1, 4, 'E'});
        car.Execute(mode);
        car.Initialize();
        ExpectPose(car.Query(), {0, 0, 'N'});
        ExpectReverse(car, false);
        ExpectAcceleration(car, false);
        car.Execute('M');
        ExpectPose(car.Query(), {0, 1, 'N'});
    }
}

void InstancesHaveIndependentReverseState() {
    Executor first;
    Executor second;
    first.Execute("BM");
    second.Execute("FM");
    ExpectPose(first.Query(), {0, -1, 'N'});
    ExpectPose(second.Query(), {0, 2, 'N'});
    ExpectReverse(first, true);
    ExpectReverse(second, false);
    ExpectAcceleration(first, false);
    ExpectAcceleration(second, true);
    first.Execute('F');
    second.Execute('B');
    first.Execute('B');
    ExpectReverse(first, false);
    ExpectReverse(second, true);
    ExpectAcceleration(first, true);
    ExpectAcceleration(second, true);
}

void EmptyBatchKeepsBothStates() {
    Executor car;
    car.Execute("BF");
    car.Execute("");
    ExpectPose(car.Query(), {0, 0, 'N'});
    ExpectReverse(car, true);
    ExpectAcceleration(car, true);
    car.Execute('M');
    ExpectPose(car.Query(), {0, -2, 'N'});
}

void SingleCommandsMatchReverseBatch() {
    Executor single(-3, 8, 'E');
    Executor batch(-3, 8, 'E');
    const std::string commands = "MBMFLMRBMFML";
    for (const char command : commands) {
        single.Execute(command);
    }
    batch.Execute(commands);
    ExpectPose(single.Query(), batch.Query());
    ExpectReverse(single, batch.IsReversing());
    ExpectAcceleration(single, batch.IsAccelerated());
}

void ReverseMovesAtInt32Boundaries() {
    constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
    for (const bool fast : {false, true}) {
        const std::int32_t distance = fast ? 2 : 1;
        const Pose starts[] = {
            {minimum + distance, 0, 'E'}, {maximum - distance, 0, 'W'},
            {0, minimum + distance, 'N'}, {0, maximum - distance, 'S'}
        };
        const Pose expected[] = {
            {minimum, 0, 'E'}, {maximum, 0, 'W'},
            {0, minimum, 'N'}, {0, maximum, 'S'}
        };
        for (int i = 0; i < 4; ++i) {
            Executor car(starts[i].x, starts[i].y, starts[i].heading);
            car.Execute(fast ? "BFM" : "BM");
            ExpectPose(car.Query(), expected[i]);
        }
    }
}

void AcceleratedReverseTurnsAtInt32Boundaries() {
    constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
    const Pose starts[] = {
        {minimum + 1, 0, 'E'}, {maximum - 1, 0, 'W'},
        {0, minimum + 1, 'N'}, {0, maximum - 1, 'S'}
    };
    const Pose expectedLeft[] = {
        {minimum, 0, 'S'}, {maximum, 0, 'N'},
        {0, minimum, 'E'}, {0, maximum, 'W'}
    };
    const Pose expectedRight[] = {
        {minimum, 0, 'N'}, {maximum, 0, 'S'},
        {0, minimum, 'W'}, {0, maximum, 'E'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car(starts[i].x, starts[i].y, starts[i].heading);
        car.Execute("BFL");
        ExpectPose(car.Query(), expectedLeft[i]);
        car.Initialize(starts[i].x, starts[i].y, starts[i].heading);
        car.Execute("BFR");
        ExpectPose(car.Query(), expectedRight[i]);
    }
}

void FourAcceleratedReverseTurnsClosePath() {
    for (const char heading : std::string("NSEW")) {
        Executor car(3, -5, heading);
        car.Execute("BFLLLL");
        ExpectPose(car.Query(), {3, -5, heading});
        car.Execute("RRRR");
        ExpectPose(car.Query(), {3, -5, heading});
        ExpectReverse(car, true);
        ExpectAcceleration(car, true);
    }
}

void ToggleOrderDoesNotMatter() {
    Executor first;
    Executor second;
    first.Execute("BF");
    second.Execute("FB");
    for (const char command : std::string("MLMRRML")) {
        first.Execute(command);
        second.Execute(command);
        ExpectPose(first.Query(), second.Query());
        ExpectReverse(first, second.IsReversing());
        ExpectAcceleration(first, second.IsAccelerated());
    }
}

// Independent reference: expand each command into basic movement/turns.
// Lowercase m means one step backward with the heading unchanged.
struct ReferenceState {
    Pose pose;
    bool fast = false;
    bool reverse = false;
};

void ReferencePrimitive(ReferenceState& state, char primitive) {
    if (primitive == 'M' || primitive == 'm') {
        const int direction = primitive == 'M' ? 1 : -1;
        if (state.pose.heading == 'N') state.pose.y += direction;
        if (state.pose.heading == 'S') state.pose.y -= direction;
        if (state.pose.heading == 'E') state.pose.x += direction;
        if (state.pose.heading == 'W') state.pose.x -= direction;
        return;
    }
    const bool left = primitive == 'L';
    switch (state.pose.heading) {
    case 'N': state.pose.heading = left ? 'W' : 'E'; break;
    case 'E': state.pose.heading = left ? 'N' : 'S'; break;
    case 'S': state.pose.heading = left ? 'E' : 'W'; break;
    case 'W': state.pose.heading = left ? 'S' : 'N'; break;
    }
}

void ReferenceExecute(ReferenceState& state, char command) {
    if (command == 'F') {
        state.fast = !state.fast;
        return;
    }
    if (command == 'B') {
        state.reverse = !state.reverse;
        return;
    }
    constexpr std::string_view expansions[4][3] = {
        {"M", "L", "R"}, {"MM", "ML", "MR"},
        {"m", "R", "L"}, {"mm", "mR", "mL"}
    };
    const int mode = (state.fast ? 1 : 0) + (state.reverse ? 2 : 0);
    const int index = command == 'M' ? 0 : (command == 'L' ? 1 : 2);
    for (const char primitive : expansions[mode][index]) {
        ReferencePrimitive(state, primitive);
    }
}

void CheckReferenceTree(const Executor& car, const ReferenceState& model,
                        int remaining, const std::string& sequence, int& cases) {
    ++cases;
    try {
        ExpectPose(car.Query(), model.pose);
        ExpectAcceleration(car, model.fast);
        ExpectReverse(car, model.reverse);
    } catch (const std::exception& error) {
        throw std::runtime_error("sequence '" + sequence + "': " + error.what());
    }
    if (remaining == 0) return;
    for (const char command : std::string_view("MLRFB")) {
        Executor nextCar = car;
        ReferenceState nextModel = model;
        nextCar.Execute(command);
        ReferenceExecute(nextModel, command);
        CheckReferenceTree(nextCar, nextModel, remaining - 1,
                           sequence + command, cases);
    }
}

void ExhaustiveShortSequencesMatchReference() {
    int cases = 0;
    for (const char heading : std::string("NSEW")) {
        for (const std::string_view mode : {"", "F", "B", "FB"}) {
            Executor car(3, -4, heading);
            ReferenceState model{{3, -4, heading}};
            for (const char command : mode) {
                car.Execute(command);
                ReferenceExecute(model, command);
            }
            CheckReferenceTree(car, model, 5, std::string(mode), cases);
        }
    }
    if (cases != 62496) throw std::runtime_error("incomplete sequence coverage");
}


const Pose kNormalTurnRound[] = {
    {4, -3, 'S'}, {5, -2, 'W'}, {6, -3, 'N'}, {5, -4, 'E'}
};
const Pose kFastTurnRound[] = {
    {4, -2, 'S'}, {6, -2, 'W'}, {6, -4, 'N'}, {4, -4, 'E'}
};

void CheckTurnRoundDirections(std::string_view mode, const Pose* expected) {
    const char headings[] = {'N', 'E', 'S', 'W'};
    for (int i = 0; i < 4; ++i) {
        Executor car(5, -3, headings[i]);
        car.Execute(mode);
        car.Execute("TR");
        ExpectPose(car.Query(), expected[i]);
    }
}

void NormalTurnRoundInAllDirections() {
    CheckTurnRoundDirections("", kNormalTurnRound);
}

void FastTurnRoundInAllDirections() {
    CheckTurnRoundDirections("F", kFastTurnRound);
}

void ReverseDoesNotAffectTurnRound() {
    CheckTurnRoundDirections("B", kNormalTurnRound);
}

void FastReverseDoesNotAffectTurnRound() {
    CheckTurnRoundDirections("BF", kFastTurnRound);
}

void TurnRoundPreservesModes() {
    for (const std::string_view mode : {"", "F", "B", "BF"}) {
        Executor car;
        car.Execute(mode);
        car.Execute("TR");
        ExpectAcceleration(car, mode.find('F') != std::string_view::npos);
        ExpectReverse(car, mode.find('B') != std::string_view::npos);
        car.Execute('M');
        const bool fast = car.IsAccelerated();
        const bool reverse = car.IsReversing();
        const int distance = fast ? 2 : 1;
        const int startY = fast ? 1 : 0;
        ExpectPose(car.Query(), {-1, startY + (reverse ? distance : -distance), 'S'});
    }
}

void RepeatedTurnRoundsReturnToStart() {
    for (const char heading : std::string("NESW")) {
        for (const std::string_view mode : {"", "F", "B", "BF"}) {
            Executor car(5, -3, heading);
            car.Execute(mode);
            car.Execute("TRTR");
            ExpectPose(car.Query(), {5, -3, heading});
            car.Execute("TRTRTRTR");
            ExpectPose(car.Query(), {5, -3, heading});
        }
    }
}

void TrailingRIsSeparateCommand() {
    const std::string_view modes[] = {"", "F", "B", "BF"};
    const Pose expected[] = {
        {-1, 0, 'W'}, {-1, 0, 'W'}, {-1, 0, 'E'}, {-1, 2, 'E'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car;
        car.Execute(modes[i]);
        car.Execute("TRR");
        ExpectPose(car.Query(), expected[i]);
    }
}

void PrecedingRIsSeparateCommand() {
    const std::string_view modes[] = {"", "F", "B", "BF"};
    const Pose expected[] = {
        {0, 1, 'W'}, {1, 2, 'W'}, {0, -1, 'E'}, {-1, -2, 'E'}
    };
    for (int i = 0; i < 4; ++i) {
        Executor car;
        car.Execute(modes[i]);
        car.Execute("RTR");
        ExpectPose(car.Query(), expected[i]);
    }
}

void TurnRoundAcrossBatches() {
    Executor car;
    car.Execute("B");
    car.Execute("F");
    car.Execute("TR");
    car.Execute("M");
    ExpectPose(car.Query(), {-1, 3, 'S'});
    car.Execute("BTR");
    ExpectPose(car.Query(), {0, 2, 'N'});
    car.Execute("FTR");
    ExpectPose(car.Query(), {-1, 2, 'S'});
    ExpectAcceleration(car, false);
    ExpectReverse(car, false);
}

void MixedStateCommandsWithTurnRound() {
    Executor car;
    car.Execute("BMFTRBMFTR");
    ExpectPose(car.Query(), {0, -2, 'N'});
    ExpectAcceleration(car, false);
    ExpectReverse(car, false);
}

void IndividualTokensMatchTurnRoundBatch() {
    Executor single(-3, 8, 'E');
    Executor batch(-3, 8, 'E');
    for (const std::string_view token : {"M", "B", "F", "TR", "L", "M", "R", "B", "TR", "F", "M", "TR"}) {
        single.Execute(token);
    }
    batch.Execute("MBFTRLMRBTRFMTR");
    ExpectPose(single.Query(), batch.Query());
    ExpectAcceleration(single, batch.IsAccelerated());
    ExpectReverse(single, batch.IsReversing());
}

void NormalTurnRoundAtInt32Boundaries() {
    constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
    const Pose starts[] = {
        {minimum + 1, maximum, 'N'}, {maximum, maximum - 1, 'E'},
        {maximum - 1, minimum, 'S'}, {minimum, minimum + 1, 'W'}
    };
    const Pose expected[] = {
        {minimum, maximum, 'S'}, {maximum, maximum, 'W'},
        {maximum, minimum, 'N'}, {minimum, minimum, 'E'}
    };
    for (const std::string_view mode : {"", "B"}) {
        for (int i = 0; i < 4; ++i) {
            Executor car(starts[i].x, starts[i].y, starts[i].heading);
            car.Execute(mode);
            car.Execute("TR");
            ExpectPose(car.Query(), expected[i]);
        }
    }
}

void FastTurnRoundAtInt32Boundaries() {
    constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
    const Pose starts[] = {
        {minimum + 1, maximum - 1, 'N'}, {maximum - 1, maximum - 1, 'E'},
        {maximum - 1, minimum + 1, 'S'}, {minimum + 1, minimum + 1, 'W'}
    };
    const Pose expected[] = {
        {minimum, maximum, 'S'}, {maximum, maximum, 'W'},
        {maximum, minimum, 'N'}, {minimum, minimum, 'E'}
    };
    for (const std::string_view mode : {"F", "BF"}) {
        for (int i = 0; i < 4; ++i) {
            Executor car(starts[i].x, starts[i].y, starts[i].heading);
            car.Execute(mode);
            car.Execute("TR");
            ExpectPose(car.Query(), expected[i]);
        }
    }
}

void InitializationAfterTurnRound() {
    Executor car;
    car.Execute("BFTR");
    car.Initialize();
    car.Execute("TR");
    ExpectPose(car.Query(), {-1, 0, 'S'});
    ExpectAcceleration(car, false);
    ExpectReverse(car, false);
    car.Initialize(5, -3, 'E');
    car.Execute("TR");
    ExpectPose(car.Query(), {5, -2, 'W'});
}

void TogglesAfterTurnRound() {
    Executor car;
    car.Execute("FTRFTR");
    ExpectPose(car.Query(), {0, 1, 'N'});
    ExpectAcceleration(car, false);
    car.Initialize();
    car.Execute("BTRBTR");
    ExpectPose(car.Query(), {0, 0, 'N'});
    ExpectReverse(car, false);
}

void ReferenceToken(ReferenceState& model, std::string_view token) {
    if (token == "TR") {
        // TR always uses physical forward movement and left turns.
        const std::string_view primitives = model.fast ? "MLML" : "LML";
        for (const char primitive : primitives) {
            ReferencePrimitive(model, primitive);
        }
    } else {
        ReferenceExecute(model, token.front());
    }
}

void CheckTokenTree(const Executor& car, const ReferenceState& model,
                    int remaining, const std::string& sequence, int& cases) {
    ++cases;
    try {
        ExpectPose(car.Query(), model.pose);
        ExpectAcceleration(car, model.fast);
        ExpectReverse(car, model.reverse);
    } catch (const std::exception& error) {
        throw std::runtime_error("tokens '" + sequence + "': " + error.what());
    }
    if (remaining == 0) return;
    for (const std::string_view token : {"M", "L", "R", "F", "B", "TR"}) {
        Executor nextCar = car;
        ReferenceState nextModel = model;
        nextCar.Execute(token);
        ReferenceToken(nextModel, token);
        CheckTokenTree(nextCar, nextModel, remaining - 1,
                       sequence + std::string(token), cases);
    }
}

void ExhaustiveTurnRoundSequencesMatchReference() {
    int cases = 0;
    for (const char heading : std::string("NESW")) {
        for (const std::string_view mode : {"", "F", "B", "BF"}) {
            Executor car(3, -4, heading);
            ReferenceState model{{3, -4, heading}};
            for (const char command : mode) {
                car.Execute(command);
                ReferenceExecute(model, command);
            }
            CheckTokenTree(car, model, 4, std::string(mode), cases);
        }
    }
    if (cases != 24880) throw std::runtime_error("incomplete TR sequence coverage");
}

struct TestCase {
    const char* name;
    void (*run)();
};
}

int main() {
    const TestCase tests[] = {
        {"normal TR in all directions", NormalTurnRoundInAllDirections},
        {"fast TR in all directions", FastTurnRoundInAllDirections},
        {"reverse does not affect TR", ReverseDoesNotAffectTurnRound},
        {"fast reverse does not affect TR", FastReverseDoesNotAffectTurnRound},
        {"TR preserves modes", TurnRoundPreservesModes},
        {"repeated TR returns to start", RepeatedTurnRoundsReturnToStart},
        {"trailing R is separate", TrailingRIsSeparateCommand},
        {"preceding R is separate", PrecedingRIsSeparateCommand},
        {"TR across batches", TurnRoundAcrossBatches},
        {"mixed state commands with TR", MixedStateCommandsWithTurnRound},
        {"individual tokens match TR batch", IndividualTokensMatchTurnRoundBatch},
        {"normal TR at int32 boundaries", NormalTurnRoundAtInt32Boundaries},
        {"fast TR at int32 boundaries", FastTurnRoundAtInt32Boundaries},
        {"initialization after TR", InitializationAfterTurnRound},
        {"toggles after TR", TogglesAfterTurnRound},
        {"exhaustive TR sequences match reference", ExhaustiveTurnRoundSequencesMatchReference},
        {"reverse toggle keeps pose", ReverseToggleKeepsPose},
        {"reverse move in all directions", ReverseMoveInAllDirections},
        {"reverse left in all directions", ReverseLeftInAllDirections},
        {"reverse right in all directions", ReverseRightInAllDirections},
        {"accelerated reverse move in all directions", AcceleratedReverseMoveInAllDirections},
        {"accelerated reverse left in all directions", AcceleratedReverseLeftInAllDirections},
        {"accelerated reverse right in all directions", AcceleratedReverseRightInAllDirections},
        {"cancelling modes independently", CancellingModesIndependently},
        {"repeated reverse toggles", RepeatedReverseToggles},
        {"reverse persists across batches", ReversePersistsAcrossBatches},
        {"mixed forward reverse batch", MixedForwardReverseBatch},
        {"initialization clears both states", InitializationClearsBothStates},
        {"instances have independent reverse state", InstancesHaveIndependentReverseState},
        {"empty batch keeps both states", EmptyBatchKeepsBothStates},
        {"single commands match reverse batch", SingleCommandsMatchReverseBatch},
        {"reverse moves at int32 boundaries", ReverseMovesAtInt32Boundaries},
        {"accelerated reverse turns at int32 boundaries", AcceleratedReverseTurnsAtInt32Boundaries},
        {"four accelerated reverse turns close path", FourAcceleratedReverseTurnsClosePath},
        {"toggle order does not matter", ToggleOrderDoesNotMatter},
        {"exhaustive short sequences match reference", ExhaustiveShortSequencesMatchReference},
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
