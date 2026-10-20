# Lab03：支持倒车功能

本实验实现默认初始化、基本指令、加速和倒车功能，使用 C++17，无需第三方依赖。

## 状态与指令

默认位置和朝向为 (0, 0, N)，默认不加速、不倒车。北向使 y 增大，东向使 x 增大。

B 切换倒车状态，F 切换加速状态；两者独立，切换时不改变位置和朝向。状态会在单条指令及多次批量执行之间保持。

| 指令 | 普通前进 | 加速前进 | 普通倒车 | 加速倒车 |
| --- | --- | --- | --- | --- |
| M | 前进 1 格 | 逐格前进 2 格 | 后退 1 格 | 逐格后退 2 格 |
| L | 原地左转 | 先前进 1 格，再左转 | 原地右转 | 先后退 1 格，再右转 |
| R | 原地右转 | 先前进 1 格，再右转 | 原地左转 | 先后退 1 格，再左转 |

每次转向均为 90 度。倒车移动保持车的朝向不变；先移动后转向时，移动依据转向前的朝向。

再次收到 B 仅取消倒车，不影响加速；再次收到 F 仅取消加速，不影响倒车。重新调用 Initialize 会同时取消两种状态。

加速 M 调用两次 MoveOneStep，每次只移动一格。根据需求约束，参数合法，移动不会导致 int32 溢出。

## 文件与接口

- Executor.h / Executor.cpp：组件接口和实现。
- main.cpp：倒车、加速叠加、独立取消状态及混合指令的演示。
- tests/executor_tests.cpp：独立测试入口。

```cpp
Executor car;                  // 默认 (0, 0, N)，普通前进
Executor other(1, 2, 'E');      // 指定初始状态
car.Execute('B');              // 开启倒车
car.Execute('F');              // 开启加速，倒车继续生效
car.Execute('M');              // (0, -2, N)，经过 (0, -1, N)
car.Execute('L');              // 先退到 (0, -3, N)，再右转为 E
car.Execute('B');              // 取消倒车，保留加速
car.Execute("MRM");            // 批量执行
Pose pose = car.Query();       // 坐标为 std::int32_t，朝向为 char
bool fast = car.IsAccelerated();
bool reverse = car.IsReversing();
car.Initialize();              // 恢复默认位置、朝向及两种状态
```

## VS Code 构建和调试

以 CPP-Labs 为工作区根目录：

- “终端 → 运行任务 → Lab03: Build demo”：构建演示程序。
- “运行和调试”选择 Lab03: Debug demo，按 F5：自动构建并调试演示程序。
- “终端 → 运行任务 → Lab03: Run unit tests”：自动构建并运行所有测试。
- 选择 Lab03: Debug unit tests，按 F5：自动构建并调试测试程序。

每个实验及测试程序都有自己的 main 入口，需分别构建。

## 命令行构建

在 CPP-Labs 根目录的 PowerShell 中执行：

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 Lab03/Executor.cpp Lab03/main.cpp -o Lab03/main.exe
.\Lab03\main.exe

g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 -I Lab03 Lab03/Executor.cpp Lab03/tests/executor_tests.cpp -o Lab03/tests/executor_tests.exe
.\Lab03\tests\executor_tests.exe
```

## 单元测试

共 56 个测试用例：包含 36 个基本指令和加速测试，以及 20 个倒车及状态组合测试。

新增测试覆盖四个方向下的倒车移动、左右转向、加速倒车、独立取消状态、重复 B、跨批次保持、重新初始化、对象状态独立、混合指令，以及不溢出的 int32 边界场景。

其中一个测试使用独立的指令展开参考模型：把四种状态下的指令转换为基本单格移动与原地转向，再与 Executor 比对。对四个初始朝向、四种初始状态及长度 0 至 5 的全部 M/L/R/F/B 指令序列，共检查 62,496 种组合。

测试全部通过时输出 56/56 tests passed.，返回 0；失败时输出用例名及原因，返回非零退出码。测试检查不依赖 assert。
