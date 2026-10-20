# Lab05：支持不同的车辆类型

独立实现课后综合作业中的跑车和 Bus，使用 C++17，无第三方依赖。每个对象在构造时指定车辆类型，支持 M/L/R 单条及批量指令。

## 指令要求

| 车辆 | M | L | R |
| --- | --- | --- | --- |
| 跑车 SportsCar | 前进 1 格，再前进 1 格 | 左转 90°，再前进 1 格 | 右转 90°，再前进 1 格 |
| Bus | 前进 1 格 | 前进 1 格，再左转 90° | 前进 1 格，再右转 90° |

每次移动均由单格移动实现。跑车转向后的移动沿新朝向进行，Bus 转向前的移动沿原朝向进行。

Bus 长度占 2 格，`Query()` 返回车头的坐标。所有车辆默认在 `(0, 0, N)`；北向使 y 增大，东向使 x 增大，坐标使用 `std::int32_t`。输入合法且移动不导致整数溢出。

从默认位置分别执行一条指令：

| 车辆 | M 的结果 | L 的结果 | R 的结果 |
| --- | --- | --- | --- |
| 跑车 | (0, 2, N) | (-1, 0, W) | (1, 0, E) |
| Bus | (0, 1, N) | (0, 1, W) | (0, 1, E) |

本题给出的车辆行为对应 M/L/R。加速 F、倒车 B、掉头 TR 与新车型组合的规则需另行定义。

## 文件

- `Executor.h` / `Executor.cpp`：车辆类型、位置状态及指令实现。
- `main.cpp`：两种车辆的指令结果演示。
- `tests/executor_tests.cpp`：20 个测试组，含独立参考模型验证。

## 接口示例

```cpp
#include "Executor.h"

Executor sports(VehicleType::SportsCar);
sports.Execute('L');                         // (-1, 0, W)
sports.Execute('M');                         // (-3, 0, W)

Executor bus(VehicleType::Bus, 1, 2, 'E');
bus.Execute("LM");                          // (2, 3, N)，坐标表示车头
Pose pose = bus.Query();
VehicleType type = bus.Type();
bus.Initialize();                           // (0, 0, N)，仍为 Bus
```

`Initialize` 重置位置和朝向，保留车辆类型。`Query` 返回独立的位置快照，`Type` 返回构造时指定的车型。各对象的状态独立。

## 构建与运行

在仓库根目录的 PowerShell 中执行：

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic Lab05/Executor.cpp Lab05/main.cpp -o Lab05/main.exe
.\Lab05\main.exe

g++ -std=c++17 -Wall -Wextra -Wpedantic Lab05/Executor.cpp Lab05/tests/executor_tests.cpp -o Lab05/tests/executor_tests.exe
.\Lab05\tests\executor_tests.exe
```

也可使用仓库已有的构建脚本：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-active.ps1 -SourceFile Lab05/tests/executor_tests.cpp
.\Lab05\tests\executor_tests.exe
```

VS Code 中打开本目录的 `.cpp` 文件后，按 Ctrl+Shift+B 使用已有默认构建任务。测试通过 `../Executor.h` 明确引用本实验的头文件。

## 测试范围

20 个测试组覆盖两种车型的初始化、四个朝向下的全部指令、动作先后顺序、连续移动与转向、混合指令、批次间状态保持、重置后车型保持、快照及对象独立性和合法的 int32 边界移动。

独立参考模型通过方向向量计算预期状态，对两种车型、四个初始朝向及长度 0～6 的全部 M/L/R 指令序列进行验证，共 8,744 种组合，同时比对整串执行和逐字符执行。全部通过时输出 `20/20 tests passed.`，失败返回非零退出码；测试不依赖 assert。
