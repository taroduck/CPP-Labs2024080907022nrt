# Lab01：基本控制指令

实现智能车 Executor 组件，使用 C++17，无需第三方依赖。

## 文件结构

- `Executor.h`：位置结构和组件的公开接口。
- `Executor.cpp`：指令执行实现。
- `main.cpp`：演示初始化、批量执行和查询。
- `tests/executor_tests.cpp`：独立的单元测试入口。

演示程序和测试程序各自包含一个 `main`，需要分别编译。

## 对外接口

```cpp
Executor car;                 // 默认 (0, 0, N)
Executor other(1, 2, 'E');     // 指定初始位置和朝向
car.Initialize(-2, 3, 'W');   // 重新初始化
car.Initialize();             // 恢复默认状态
car.Execute('M');             // 执行单条指令
car.Execute("MRMLM");         // 按顺序执行批量指令
Pose pose = car.Query();      // 获取当前位置和朝向的副本
```

`Pose` 的 `x`、`y` 均为 `std::int32_t`，`heading` 为 `char`。

| 朝向 | 前进 M |
| --- | --- |
| N（北） | y 加 1 |
| S（南） | y 减 1 |
| E（东） | x 加 1 |
| W（西） | x 减 1 |

`L` 左转 90 度，`R` 右转 90 度，转向不改变坐标。批量指令保留执行顺序，多次调用会在当前状态上继续执行。根据需求约束，调用方保证输入合法且移动不会导致整数溢出。

## 在 VS Code 中运行

以 `CPP-Labs` 为工作区根目录：

- `Ctrl+Shift+B`：构建 Lab01 演示程序。
- 在“运行和调试”中选择 `Lab01: Debug demo`，按 `F5`：先构建，再调试演示程序。
- 在“终端 → 运行任务”中选择 `Lab01: Run unit tests`：先构建，再执行全部测试。
- 选择 `Lab01: Debug unit tests`，按 `F5`：构建并调试测试程序。

这些任务使用项目中配置的 `C:/mingw64/bin/g++.exe`。

## 命令行构建

在 `CPP-Labs` 根目录的 PowerShell 中执行：

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 Lab01/Executor.cpp Lab01/main.cpp -o Lab01/main.exe
.\Lab01\main.exe

g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 -I Lab01 Lab01/Executor.cpp Lab01/tests/executor_tests.cpp -o Lab01/tests/executor_tests.exe
.\Lab01\tests\executor_tests.exe
```

演示程序预期输出：

```text
Default position: (0, 0, N)
After MMRMMLM:    (2, 3, N)
Reset position:   (1, 2, E)
After MMLM:       (3, 3, N)
```

## 单元测试

共 18 个测试用例，覆盖默认及指定初始化、重新初始化、四个方向的前进及左右转、完整旋转、空指令、连续前进、混合指令、负坐标、闭合路径、多批次执行、单条和批量执行的一致性、查询副本独立性及 int32 边界内移动。

测试失败时打印用例名及预期/实际状态，并返回非零退出码；成功返回 0。检查不依赖 `assert`，在发布构建中也会执行。
