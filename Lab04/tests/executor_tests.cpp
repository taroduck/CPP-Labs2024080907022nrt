#include "../Executor.h"

#include <cstddef>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr std::string_view kHeadings = "NESW";
constexpr std::string_view kModes[] = {"", "B", "F", "FB"};
constexpr Pose kNormalTR[] = {
    {-1, 0, 'S'}, {0, 1, 'W'}, {1, 0, 'N'}, {0, -1, 'E'}
};
constexpr Pose kFastTR[] = {
    {-1, 1, 'S'}, {1, 1, 'W'}, {1, -1, 'N'}, {-1, -1, 'E'}
};

void Require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

std::string Describe(const Pose& pose) {
    return "(" + std::to_string(pose.x) + ", " + std::to_string(pose.y)
           + ", " + std::string(1, pose.heading) + ")";
}

void ExpectPose(const Executor& car, Pose expected) {
    const Pose actual = car.Query();
    Require(actual.x == expected.x && actual.y == expected.y
                && actual.heading == expected.heading,
            "expected " + Describe(expected) + ", got " + Describe(actual));
}

void ExpectModes(const Executor& car, bool fast, bool reverse) {
    Require(car.IsAccelerated() == fast, "incorrect F state");
    Require(car.IsReversing() == reverse, "incorrect B state");
}

void ExpectSame(const Executor& actual, const Executor& expected) {
    ExpectPose(actual, expected.Query());
    ExpectModes(actual, expected.IsAccelerated(), expected.IsReversing());
}

void Initialization() {
    ExpectPose(Executor{}, {0, 0, 'N'});
    for (const char heading : kHeadings) {
        const Executor car(12, -7, heading);
        ExpectPose(car, {12, -7, heading});
        ExpectModes(car, false, false);
    }
}

void InitializationResetsModes() {
    Executor car;
    car.Execute("FBTR");
    car.Initialize(-8, 15, 'W');
    ExpectPose(car, {-8, 15, 'W'});
    ExpectModes(car, false, false);
    car.Execute('M');
    ExpectPose(car, {-9, 15, 'W'});
    car.Execute("FB");
    car.Initialize();
    ExpectPose(car, {0, 0, 'N'});
    ExpectModes(car, false, false);
}

void BasicMoves() {
    constexpr Pose expected[] = {
        {5, -2, 'N'}, {6, -3, 'E'}, {5, -4, 'S'}, {4, -3, 'W'}
    };
    for (const Pose pose : expected) {
        Executor car(5, -3, pose.heading);
        car.Execute('M');
        ExpectPose(car, pose);
    }
}

void BasicTurns() {
    constexpr std::string_view left = "WNES";
    constexpr std::string_view right = "ESWN";
    for (std::size_t i = 0; i < kHeadings.size(); ++i) {
        Executor car(2, -3, kHeadings[i]);
        car.Execute('L');
        ExpectPose(car, {2, -3, left[i]});
        car.Initialize(2, -3, kHeadings[i]);
        car.Execute('R');
        ExpectPose(car, {2, -3, right[i]});
        car.Execute("RRRLLLL");
        ExpectPose(car, {2, -3, kHeadings[i]});
    }
}

void TogglesAreIndependent() {
    Executor car;
    car.Execute('F');
    ExpectModes(car, true, false);
    car.Execute('B');
    ExpectModes(car, true, true);
    car.Execute('F');
    ExpectModes(car, false, true);
    car.Execute('B');
    ExpectModes(car, false, false);
    ExpectPose(car, {0, 0, 'N'});
}

void ModesPersist() {
    Executor car;
    car.Execute("FM");
    ExpectPose(car, {0, 2, 'N'});
    car.Execute("BM");
    ExpectPose(car, {0, 0, 'N'});
    car.Execute('L');
    ExpectPose(car, {0, -1, 'E'});
    ExpectModes(car, true, true);
}

void QueryReturnsSnapshot() {
    const Executor car(2, 3, 'E');
    Pose snapshot = car.Query();
    snapshot.x = 99;
    Require(snapshot.x == 99, "snapshot was not writable");
    ExpectPose(car, {2, 3, 'E'});
}

void EmptyBatch() {
    Executor car(2, 3, 'W');
    car.Execute("FB");
    car.Execute("");
    ExpectPose(car, {2, 3, 'W'});
    ExpectModes(car, true, true);
}

void IndependentInstances() {
    Executor first;
    const Executor second;
    first.Execute("FBTR");
    ExpectPose(second, {0, 0, 'N'});
    ExpectModes(second, false, false);
}

void CheckTurnRound(std::string_view mode) {
    const bool fast = mode.find('F') != std::string_view::npos;
    const bool reverse = mode.find('B') != std::string_view::npos;
    for (std::size_t i = 0; i < kHeadings.size(); ++i) {
        Executor car(0, 0, kHeadings[i]);
        car.Execute(mode);
        car.Execute("TR");
        ExpectPose(car, fast ? kFastTR[i] : kNormalTR[i]);
        ExpectModes(car, fast, reverse);
    }
}

void NormalTurnRound() { CheckTurnRound(""); }
void ReverseTurnRound() { CheckTurnRound("B"); }
void FastTurnRound() { CheckTurnRound("F"); }
void FastReverseTurnRound() { CheckTurnRound("FB"); }

void TurnRoundPreservesModes() {
    for (const auto mode : kModes) {
        Executor car;
        car.Execute(mode);
        const bool fast = car.IsAccelerated();
        const bool reverse = car.IsReversing();
        car.Execute("TRTRTR");
        ExpectModes(car, fast, reverse);
    }
}

void MoveAfterTurnRound() {
    constexpr Pose expected[] = {
        {-1, -1, 'S'}, {-1, 1, 'S'}, {-1, -1, 'S'}, {-1, 3, 'S'}
    };
    for (std::size_t i = 0; i < 4; ++i) {
        Executor car;
        car.Execute(kModes[i]);
        car.Execute("TRM");
        ExpectPose(car, expected[i]);
    }
}

void RepeatedTurnRound() {
    for (const char heading : kHeadings) {
        for (const auto mode : kModes) {
            Executor car(3, -5, heading);
            car.Execute(mode);
            car.Execute("TRTR");
            ExpectPose(car, {3, -5, heading});
        }
    }
}

void TrailingRightIsSeparate() {
    constexpr Pose expected[] = {
        {-1, 0, 'W'}, {-1, 0, 'E'}, {-1, 0, 'W'}, {-1, 2, 'E'}
    };
    for (std::size_t i = 0; i < 4; ++i) {
        Executor car;
        car.Execute(kModes[i]);
        car.Execute("TRR");
        ExpectPose(car, expected[i]);
    }
}

void PrecedingRightIsSeparate() {
    constexpr Pose expected[] = {
        {0, 1, 'W'}, {0, -1, 'E'}, {1, 2, 'W'}, {-1, -2, 'E'}
    };
    for (std::size_t i = 0; i < 4; ++i) {
        Executor car;
        car.Execute(kModes[i]);
        car.Execute("RTR");
        ExpectPose(car, expected[i]);
    }
}

void MixedTokensMatchBatch() {
    Executor tokens(1, -2, 'E');
    Executor batch(1, -2, 'E');
    for (const std::string_view token : {"F", "TR", "B", "M", "TR", "R", "F", "L", "B", "TR"}) {
        tokens.Execute(token);
    }
    batch.Execute("FTRBMTRRFLBTR");
    ExpectSame(tokens, batch);
}

void TurnRoundNearBoundaries() {
    constexpr auto low = std::numeric_limits<std::int32_t>::min();
    constexpr auto high = std::numeric_limits<std::int32_t>::max();
    constexpr std::int32_t coordinates[] = {low + 2, high - 2};
    for (const auto x : coordinates) {
        for (const auto y : coordinates) {
            for (std::size_t i = 0; i < kHeadings.size(); ++i) {
                for (const auto mode : kModes) {
                    Executor car(x, y, kHeadings[i]);
                    car.Execute(mode);
                    car.Execute("TR");
                    const Pose offset = car.IsAccelerated() ? kFastTR[i] : kNormalTR[i];
                    ExpectPose(car, {x + offset.x, y + offset.y, offset.heading});
                }
            }
        }
    }
}

void SingleCommandsMatchBatch() {
    constexpr std::string_view commands = "FBMLRFMBBMLR";
    Executor single(-3, 8, 'E');
    Executor batch(-3, 8, 'E');
    for (const char command : commands) single.Execute(command);
    batch.Execute(commands);
    ExpectSame(single, batch);
}

void CancelModesAfterTurnRound() {
    for (const auto mode : kModes) {
        Executor car;
        car.Execute(mode);
        car.Execute("TR");
        const Pose pose = car.Query();
        car.Execute(mode);
        ExpectPose(car, pose);
        ExpectModes(car, false, false);
        car.Execute('M');
        ExpectPose(car, {pose.x, pose.y - 1, 'S'});
    }
}

void TurnRoundAcrossBatches() {
    Executor separate;
    separate.Execute("FB");
    separate.Execute("TR");
    separate.Execute("BTR");
    separate.Execute("FTR");
    Executor batch;
    batch.Execute("FBTRBTRFTR");
    ExpectSame(separate, batch);
}

// Independent model: expand tokens into physical grid moves and turns.
// Direction is an integer here, independent of Executor's char-based switch.
struct Reference {
    std::int32_t x = 3;
    std::int32_t y = -4;
    int direction = 0;
    bool fast = false;
    bool reverse = false;
};

void ApplyPrimitive(Reference& model, char primitive) {
    constexpr int dx[] = {0, 1, 0, -1};
    constexpr int dy[] = {1, 0, -1, 0};
    if (primitive == 'M' || primitive == 'm') {
        const int sign = primitive == 'M' ? 1 : -1;
        model.x += sign * dx[model.direction];
        model.y += sign * dy[model.direction];
    } else {
        model.direction = (model.direction + (primitive == 'L' ? 3 : 1)) % 4;
    }
}

void ApplyToken(Reference& model, std::string_view token) {
    if (token == "F") { model.fast = !model.fast; return; }
    if (token == "B") { model.reverse = !model.reverse; return; }
    std::string_view primitives;
    if (token == "TR") {
        primitives = model.fast ? "MLML" : "LML";
    } else {
        constexpr std::string_view expansions[4][3] = {
            {"M", "L", "R"}, {"MM", "ML", "MR"},
            {"m", "R", "L"}, {"mm", "mR", "mL"}
        };
        const int mode = (model.fast ? 1 : 0) + (model.reverse ? 2 : 0);
        const int command = token == "M" ? 0 : (token == "L" ? 1 : 2);
        primitives = expansions[mode][command];
    }
    for (const char primitive : primitives) ApplyPrimitive(model, primitive);
}

void CheckSequences(const Executor& car, const Reference& model, int remaining,
                    char initialHeading, const std::string& commands, int& checked) {
    const Pose expected{model.x, model.y,
                        kHeadings[static_cast<std::size_t>(model.direction)]};
    try {
        ExpectPose(car, expected);
        ExpectModes(car, model.fast, model.reverse);
        Executor batch(3, -4, initialHeading);
        batch.Execute(commands);
        ExpectSame(batch, car);
    } catch (const std::exception& error) {
        throw std::runtime_error(std::string(1, initialHeading) + " '"
                                 + commands + "': " + error.what());
    }
    ++checked;
    if (remaining == 0) return;
    for (const std::string_view token : {"M", "L", "R", "F", "B", "TR"}) {
        Executor next = car;
        Reference nextModel = model;
        next.Execute(token);
        ApplyToken(nextModel, token);
        CheckSequences(next, nextModel, remaining - 1, initialHeading,
                       commands + std::string(token), checked);
    }
}

void ExhaustiveSequences() {
    int checked = 0;
    for (std::size_t i = 0; i < kHeadings.size(); ++i) {
        for (const auto mode : kModes) {
            Executor car(3, -4, kHeadings[i]);
            car.Execute(mode);
            Reference model;
            model.direction = static_cast<int>(i);
            for (const char command : mode) ApplyToken(model, std::string_view(&command, 1));
            CheckSequences(car, model, 4, kHeadings[i], std::string(mode), checked);
        }
    }
    Require(checked == 24880, "incomplete exhaustive coverage");
    std::cout << "  checked " << checked << " reference sequences\n";
}

struct Test {
    const char* name;
    void (*run)();
};
}

int main() {
    const Test tests[] = {
        {"initialization", Initialization},
        {"initialization resets modes", InitializationResetsModes},
        {"basic moves in all directions", BasicMoves},
        {"basic turns in all directions", BasicTurns},
        {"independent F/B toggles", TogglesAreIndependent},
        {"modes persist across calls", ModesPersist},
        {"query returns snapshot", QueryReturnsSnapshot},
        {"empty batch", EmptyBatch},
        {"independent instances", IndependentInstances},
        {"normal TR in all directions", NormalTurnRound},
        {"reverse TR in all directions", ReverseTurnRound},
        {"fast TR in all directions", FastTurnRound},
        {"fast reverse TR in all directions", FastReverseTurnRound},
        {"TR preserves F/B", TurnRoundPreservesModes},
        {"move after TR", MoveAfterTurnRound},
        {"repeated TR", RepeatedTurnRound},
        {"TRR parsing", TrailingRightIsSeparate},
        {"RTR parsing", PrecedingRightIsSeparate},
        {"mixed tokens match batch", MixedTokensMatchBatch},
        {"TR near int32 boundaries", TurnRoundNearBoundaries},
        {"single commands match batch", SingleCommandsMatchBatch},
        {"cancel modes after TR", CancelModesAfterTurnRound},
        {"TR across batches", TurnRoundAcrossBatches},
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
