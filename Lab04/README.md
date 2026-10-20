# Lab04：支持掉头功能

本实验保留基本控制、加速和倒车功能，新增 TR 掉头指令，使用 C++17，无第三方依赖。

## TR 执行规则

| 状态 | 动作顺序 |
| --- | --- |
| 未加速（无论是否倒车） | 左转 90° → 前进 1 格 → 左转 90° |
| 已加速（无论是否倒车） | 前进 1 格 → 左转 90° → 前进 1 格 → 左转 90° |

TR 中的前进始终是沿车当前朝向前进，左转始终是物理左转，不受 B 影响。每一步移动只前进一格；加速时按上表增加最开始的那一步，不对表内的每个动作再次套用加速。

TR 执行完毕后，朝向与原来相反，F/B 状态保持。再次收到 F/B 仍只切换相应状态；重新初始化时同时恢复普通速度和前进状态。

TR 作为一个完整指令识别，必须在同一次字符串调用中连续出现。TRR 表示一次掉头再执行一次 R；RTR 表示一次 R 再执行一次掉头。按需求约束，不处理非法输入和整数溢出。

## 文件与接口

- Executor.h / Executor.cpp：组件接口、完整指令解析和动作实现。
- main.cpp：四种状态下的掉头、掉头后的倒车，以及混合指令演示。
- tests/executor_tests.cpp：独立单元测试入口。

```cpp
Executor car;                  // 默认 (0, 0, N)
car.Execute("TR");             // (-1, 0, S)
car.Initialize();
car.Execute("BFTR");           // (-1, 1, S)，保持加速和倒车状态
car.Execute('M');              // (-1, 3, S)，逐格倒退 2 格
Pose pose = car.Query();
bool fast = car.IsAccelerated();
bool reverse = car.IsReversing();
car.Initialize(1, 2, 'E');     // 重新初始化并取消 F/B 状态
```

单字符 Execute(char) 用于 M/L/R/F/B，Execute(std::string_view) 支持这些单字符指令及完整的 TR。TR 不拆为两次单字符调用。

## VS Code 构建和调试

以 CPP-Labs 为工作区根目录：

- “终端 → 运行任务 → Lab04: Build demo”：构建演示程序。
- “运行和调试”选择 Lab04: Debug demo，按 F5：自动构建并调试演示程序。
- “终端 → 运行任务 → Lab04: Run unit tests”：自动构建并运行测试。
- 选择 Lab04: Debug unit tests，按 F5：自动构建并调试测试程序。

## 命令行构建

在仓库根目录的 PowerShell 中执行：

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 Lab04/Executor.cpp Lab04/main.cpp -o Lab04/main.exe
.\Lab04\main.exe

g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 -I Lab04 Lab04/Executor.cpp Lab04/tests/executor_tests.cpp -o Lab04/tests/executor_tests.exe
.\Lab04\tests\executor_tests.exe
```

每个实验及其测试程序分别包含 main，需要分别编译。

## 测试覆盖

共 72 个测试用例，包含此前 56 个基本控制、加速和倒车测试，以及 16 个掉头相关测试。

新增测试覆盖四个朝向、四种状态、F/B 状态保持、连续 TR、TRR/RTR 的解析、跨批次调用、混合指令、初始化和 int32 边界内的合法路径。

在此前参考模型测试基础上，新增包含 TR 的指令序列检查：覆盖四个朝向、四种初始状态和长度 0 至 4 的所有 M/L/R/F/B/TR 指令序列，共 24,880 种组合。参考模型将 TR 独立展开成 LML 或 MLML，不受倒车影响。

成功输出 72/72 tests passed.，返回 0；失败报告用例与原因并返回非零退出码。
