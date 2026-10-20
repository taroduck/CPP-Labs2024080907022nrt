# Lab04：支持掉头 TR 指令

本目录重新实现需求 4，保留 Lab03 的 M/L/R、F 加速、B 倒车及初始化、位置查询功能。使用 C++17，无第三方依赖。所有输入合法，不处理异常输入或坐标溢出。

## 掉头规则

TR 是一个连续的双字符指令，必须通过同一次 `Execute("TR")` 字符串调用传入。

| F 加速 | B 倒车 | TR 动作顺序 |
| --- | --- | --- |
| 关闭 | 关闭或开启 | 左转 90° → 向前 1 格 → 左转 90° |
| 开启 | 关闭或开启 | 向前 1 格 → 左转 90° → 向前 1 格 → 左转 90° |

这里的前进和左转始终相对车身朝向，不受 B 影响；每次前进严格为一格。TR 结束后朝向与原来相反，F/B 状态保留。实现直接调用单格移动和原地转向，不重复套用 M/L 的加速或倒车逻辑。

从默认 `(0, 0, N)` 出发：

| 指令 | 最终位置、朝向 | 最终 F / B |
| --- | --- | --- |
| TR | (-1, 0, S) | false / false |
| BTR | (-1, 0, S) | false / true |
| FTR | (-1, 1, S) | true / false |
| FBTR | (-1, 1, S) | true / true |

`TRR` 按 `TR`、`R` 解析，`RTR` 按 `R`、`TR` 解析；`TRTR` 表示连续两次掉头。单字符接口用于 M/L/R/F/B，不把 TR 拆成两次单字符调用。

## 原有指令

默认朝北，北向使 y 增大，东向使 x 增大。F、B 各自切换状态，不移动或转向。

| 指令 | 普通前进 | 加速前进 | 普通倒车 | 加速倒车 |
| --- | --- | --- | --- | --- |
| M | 前进 1 格 | 前进 2 格 | 后退 1 格 | 后退 2 格 |
| L | 左转 | 前进 1 格后左转 | 右转 | 后退 1 格后右转 |
| R | 右转 | 前进 1 格后右转 | 左转 | 后退 1 格后左转 |

每次转向均为 90°。`Initialize` 恢复指定位置、朝向，并关闭 F/B。

## 文件与接口

- `Executor.h`、`Executor.cpp`：组件声明、指令解析与动作实现。
- `main.cpp`：四种 F/B 状态下的 TR 演示。
- `tests/executor_tests.cpp`：24 个测试组，包括独立参考模型比对。

```cpp
Executor car;
car.Execute("FBTR");          // (-1, 1, S)，仍处于加速倒车状态
car.Execute('M');             // (-1, 3, S)
Pose pose = car.Query();
bool fast = car.IsAccelerated();
bool reverse = car.IsReversing();
car.Initialize(1, 2, 'E');     // 关闭 F/B
```

## 构建与运行

在仓库根目录的 PowerShell 中执行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-active.ps1 -SourceFile Lab04/main.cpp
.\Lab04\main.exe

powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-active.ps1 -SourceFile Lab04/tests/executor_tests.cpp
.\Lab04\tests\executor_tests.exe
```

也可以直接用 g++ 编译，必须链接组件实现：

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic Lab04/Executor.cpp Lab04/main.cpp -o Lab04/main.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic Lab04/Executor.cpp Lab04/tests/executor_tests.cpp -o Lab04/tests/executor_tests.exe
```

测试使用 `../Executor.h` 明确引用本实验头文件，避免其他实验的同名头文件干扰。VS Code 中选择“终端 → 运行任务 → Lab04: Run unit tests”可构建并运行；打开 `.cpp` 后按 Ctrl+Shift+B 可构建当前程序。

## 验证范围

24 个测试组覆盖四个朝向、四种 F/B 状态、掉头后状态保持、后续移动、连续掉头、TRR/RTR 解析、跨批次执行、初始化以及合法的 int32 边界路径。

独立参考模型将 TR 展开为 `LML` 或 `MLML`，把原有指令展开为单格前进/后退与原地左右转。对四个朝向、四种初始状态以及长度 0～4 的全部 M/L/R/F/B/TR 指令序列，检查 24,880 种组合，同时比对整串执行与逐指令执行。测试不依赖 assert，失败返回非零退出码。
