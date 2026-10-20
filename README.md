# C++ 课程实验：智能车 Executor

使用 C++17 实现智能车的位置、朝向和控制指令。各实验可以独立构建，每个目录包含组件、演示程序、单元测试及说明文档。

| 目录 | 功能 | 单元测试数 |
| --- | --- | --- |
| [Lab01](Lab01/README.md) | 初始化、查询、M 前进、L/R 转向及批量执行 | 18 |
| [Lab02](Lab02/README.md) | F 加速状态切换 | 36 |
| [Lab03](Lab03/README.md) | B 倒车及与 F 叠加 | 56 |
| [Lab04](Lab04/README.md) | TR 掉头，忽略 B 并按 F 调整动作 | 72 |

坐标使用 std::int32_t，朝向使用 char（N/S/E/W）。默认位置和朝向为 (0, 0, N)，北向使 y 增大，东向使 x 增大。根据课程约束，输入合法且不会发生整数溢出。

## 构建与运行

需要支持 C++17 的编译器。以下为在仓库根目录使用 MinGW g++ 和 PowerShell 的示例，以 Lab04 为例：

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 Lab04/Executor.cpp Lab04/main.cpp -o Lab04/main.exe
.\Lab04\main.exe

g++ -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 -I Lab04 Lab04/Executor.cpp Lab04/tests/executor_tests.cpp -o Lab04/tests/executor_tests.exe
.\Lab04\tests\executor_tests.exe
```

将命令中的 Lab04 替换为 Lab01、Lab02 或 Lab03，可以构建对应实验。演示与测试各有一个 main 入口，需要分别构建；不同实验也应分别构建。

测试无需第三方库，不依赖 assert。失败时打印用例名与原因并返回非零退出码，成功返回 0。四个实验共 182 个测试用例，另有用例内的参考模型序列比对。

每个实验的 README 提供更详细的接口、指令规则和运行说明。可执行文件、构建产物和临时日志由 .gitignore 排除。
