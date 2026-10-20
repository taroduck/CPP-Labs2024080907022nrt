# Lab02：支持加速功能

本实验在基本控制指令的基础上加入加速状态，使用 C++17，可以独立编译运行。

## 指令规则

初始状态为普通速度，默认位置和朝向为 `(0, 0, N)`。北向使 y 增大，东向使 x 增大。

| 指令 | 普通状态 | 加速状态 |
| --- | --- | --- |
| F | 开启加速，位置与朝向不变 | 取消加速，位置与朝向不变 |
| M | 前进 1 格 | 连续执行两次前进 1 格 |
| L | 原地左转 90 度 | 沿当前朝向先前进 1 格，再左转 90 度 |
| R | 原地右转 90 度 | 沿当前朝向先前进 1 格，再右转 90 度 |

加速状态属于每个 Executor 对象，在多条指令和多次批量执行之间保持，直到再次收到 F。重新调用 `Initialize` 会恢复普通速度。

`MoveForwardOneStep()` 每次仅移动一格，加速 M 调用两次该函数，满足逐格前进要求。

按照需求约束，传入参数合法，移动不会造成 int32 溢出。

## 文件和接口

- `Executor.h` / `Executor.cpp`：组件接口与实现。
- `main.cpp`：加速、转向、取消加速及混合批量指令的演示。
- `tests/executor_tests.cpp`：36 个独立测试用例，无需第三方测试库。

```cpp
Executor car;                  // 默认 (0, 0, N)，普通速度
Executor other(1, 2, 'E');      // 指定初始位置和朝向
car.Execute('F');              // 开启加速
car.Execute('M');              // 逐格前进到 (0, 2, N)
car.Execute("LRM");            // 批量执行，保留加速状态
bool fast = car.IsAccelerated();
Pose pose = car.Query();       // 坐标为 std::int32_t，朝向为 char
car.Execute('F');              // 取消加速
car.Initialize();             // 恢复 (0, 0, N)，普通速度
```

## VS Code 构建和调试

以 `CPP-Labs` 为工作区根目录：

- “终端 → 运行任务 → Lab02: Build demo”：构建演示程序。
- “运行和调试”选择 `Lab02: Debug demo`，按 F5：自动构建并调试演示程序。
- “终端 → 运行任务 → Lab02: Run unit tests”：自动构建并运行全部测试。
- 选择 `Lab02: Debug unit tests`，按 F5：自动构建并调试测试程序。

Lab01 和 Lab02 分别有自己的演示与测试入口，按实验分别构建。

## 命令行构建

在 `CPP-Labs` 根目录的 PowerShell 中执行：

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 Lab02/Executor.cpp Lab02/main.cpp -o Lab02/main.exe
.\Lab02\main.exe

g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 -I Lab02 Lab02/Executor.cpp Lab02/tests/executor_tests.cpp -o Lab02/tests/executor_tests.exe
.\Lab02\tests\executor_tests.exe
```

演示程序预期输出：

```text
Initial:    (0, 0, N), accelerated=false
After F:    (0, 0, N), accelerated=true
After M:    (0, 2, N), accelerated=true
After L:    (0, 3, W), accelerated=true
After R:    (-1, 3, N), accelerated=true
After F:    (-1, 3, N), accelerated=false
After M:    (-1, 4, N), accelerated=false
Mixed batch:(2, 5, N), accelerated=false
```

## 测试覆盖

保留 18 个基本指令测试，新增 18 个加速测试，覆盖开启与取消加速、F 不改变位置朝向、四个方向的加速移动及先移动后转向、跨指令和跨批次的状态保持、连续 F、取消后的普通速度、重新初始化、多个对象的独立性、混合批量指令及 int32 边界内移动。

全部成功时输出 `36/36 tests passed.`，返回 0；失败时报告用例和原因，并返回非零退出码。
