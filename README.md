# 0-1 背包问题算法实现与性能分析

本项目旨在使用 C 语言实现解决 0-1 背包问题的多种经典算法，并对它们的性能进行比较。此外，项目还包含一个 MATLAB 脚本，用于将 C 程序输出的性能数据可视化。

## 项目功能

*   **多种算法实现**：实现了以下解决 0-1 背包问题的算法：
    *   蛮力法 (Brute Force)
    *   动态规划法 (2D Array)
    *   贪心法 (Greedy Algorithm)
    *   回溯法 (Backtracking with Pruning)
*   **数据处理**：
    *   支持浮点数表示的物品重量和价值（精确到两位小数）。
    *   内部将浮点数重量和价值乘以 100 转换为整数进行计算，以避免浮点数精度问题。
    *   算法结果（包括选中的物品、总价值、总重量和执行时间）会输出到控制台。
*   **性能数据记录**：
    *   将各算法在不同物品数量和背包容量下的执行时间、获得的总价值和总重量记录到 `results.csv` 文件中。
*   **性能可视化**：
    *   提供 MATLAB 脚本 (`0_1_Knapsack Problem/KnapsackPerformancePlot.m`) 读取 `results.csv` 文件，并生成性能对比图表（执行时间 vs. 物品数量），保存在 `0_1_Knapsack Problem/performance_charts` 目录下。

## 已实现算法

1.  **蛮力法 (Brute Force)**:
    *   尝试所有可能的物品组合，找到在不超过背包容量前提下的最大价值组合。
    *   时间复杂度：O(2^n)，其中 n 是物品数量。
    *   适用于物品数量较少的情况 (代码中设置为 n > 15 时跳过)。

2.  **动态规划法 (Dynamic Programming - 2D Array)**:
    *   使用一个二维数组 `dp[i][j]` 表示在前 `i` 个物品中，容量为 `j` 时可以获得的最大价值。
    *   时间复杂度：O(n*W)，其中 n 是物品数量，W 是背包容量 (经过缩放后的整数容量)。
    *   空间复杂度：O(n*W)。
    *   代码中包含内存限制检查，当预估 DP 表所需内存超过阈值 (2GB) 时会跳过测试。

3.  **贪心法 (Greedy Algorithm)**:
    *   将物品按照价值重量比 (v/w)降序排序。
    *   依次选择单位重量价值最高的物品，直到背包装满或没有物品可选。
    *   时间复杂度：O(n log n) (主要为排序时间)。
    *   贪心法不一定能得到最优解，但执行速度快。

4.  **回溯法 (Backtracking with Pruning)**:
    *   通过深度优先搜索遍历解空间树。
    *   使用上界函数 (upper bound) 进行剪枝，提前排除不可能产生更优解的分支。
    *   物品首先按价值重量比降序排序，以提高剪枝效率。
    *   时间复杂度在最坏情况下仍为 O(2^n)，但通过剪枝通常表现优于纯蛮力法。
    *   适用于物品数量中等的情况 (代码中设置为 n > 32 时跳过)。

## 项目结构

```
. (项目根目录)
├── 0_1_Knapsack Problem/
│   ├── src/
│   │   └── 20231060285-彭家城-代码.c  (C 语言源代码)
│   ├── KnapsackPerformancePlot.m     (MATLAB 绘图脚本)
│   ├── results.csv                   (C 程序运行后生成的性能数据，可被 .m 脚本读取)
│   └── performance_charts/           (MATLAB 脚本生成的图表将保存于此目录)
└── README.md                         (本项目说明文件)
```

## 如何编译和运行

### 1. C 程序 (算法实现)

*   **编译环境**：需要一个 C 编译器，例如 GCC。
*   **编译命令 (示例使用 GCC)**：
    进入 `0_1_Knapsack Problem/src` 目录:
    ```bash
    cd "0_1_Knapsack Problem/src"
    gcc "20231060285-彭家城-代码.c" -o knapsack_solver -lm
    ```
    (`-lm` 用于链接数学库，因为代码中使用了 `pow`, `round` 等函数)。
*   **运行**：
    在 `0_1_Knapsack Problem/src` 目录下执行：
    ```bash
    ./knapsack_solver
    ```
    程序运行后，会在 `0_1_Knapsack Problem` 目录下生成 `results.csv` 文件。

### 2. MATLAB 脚本 (性能可视化)

*   **环境**：需要 MATLAB 环境。
*   **运行步骤**：
    1.  确保 C 程序已经运行并成功生成了 `0_1_Knapsack Problem/results.csv` 文件。
    2.  打开 MATLAB。
    3.  将 MATLAB 的当前工作目录更改到 `0_1_Knapsack Problem` 目录。
    4.  在 MATLAB 命令窗口中运行脚本：
        ```matlab
        KnapsackPerformancePlot
        ```
    5.  脚本执行完毕后，性能图表将保存在 `0_1_Knapsack Problem/performance_charts` 目录下。

## 注意事项

*   C 代码中的物品价值 `v` 和动态规划中的 `dp` 表存储的都是乘以 100 后的整数值，以处理两位小数的价值。输出时会转换回原始浮点数值。
*   物品重量 `w_double` 是原始浮点数重量，`w_int_scaled` 是乘以 100 后的整数重量，用于动态规划。
*   MATLAB 脚本 `KnapsackPerformancePlot.m` 需要 `results.csv` 文件存在于其相同目录下才能正确读取数据。
*   文件路径和脚本编码：MATLAB 脚本建议使用 UTF-8 编码保存，并确保文件路径不包含中文字符或空格，以避免潜在的兼容性问题。

## 作者

彭家城 (20231060285)
