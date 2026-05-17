# ESet Lab Report

## 运行配置

```text
CPU Model:
Intel(R) Core(TM) i9-14900HX

CPU Base Frequency:
2419.198 MHZ

Cores and Threads:
16 cores, 32 threads

RAM:
7.6Gi

Cache:
L1d: 768
L1i: 512
L2: 32
L3: 36

Operating System:
Ubuntu 24.04.4 LTS
```

## 实验设计

本次实验使用 [test/bench.cpp](test/bench.cpp) 对 `ESet` 和 `std::set` 做同条件对比，分成两类：

1. Trace benchmark：按 README 中的操作比例生成 $N=100000$ 次混合操作，并在两种容器上执行同一条操作序列。
2. Micro benchmark：分别对 `emplace`、`find`、`erase`、`range`、`copy`、`iterator_walk` 测平均耗时，并统计标准差。

benchmark 的操作分布遵循 README：

| 总操作数 N ~ 100000 |         |       |      |              |       |          |
| ------------------- | ------- | ----- | ---- | ------------ | ----- | -------- |
| 操作                | emplace | erase | find | `=` operator | range | it相关   |
| 个数约              | N/2     | N/6   | N/9  | 25           | N/18  | 剩余所有 |

- `emplace` 50000 次
- `erase` 16666 次
- `find` 11111 次
- `copy` 25 次
- `range` 5555 次
- `iterator` 8787 次
- `bound` 7856 次


## 基线实验：秩统计 `range()`

### Trace 结果

| 容器 | emplace | erase | find | copy | range | iterator | bound | 总计 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| ESet | 9.7664 ms | 1.5692 ms | 1.0726 ms | 1.0413 ms | 0.8119 ms | 0.4174 ms | 0.1040 ms | 14.7827 ms |
| std::set | 5.6408 ms | 1.5555 ms | 0.9930 ms | 0.5145 ms | 26.7196 ms | 0.2508 ms | 0.0987 ms | 35.7728 ms |

### Micro 结果

| 操作 | ESet 平均耗时 | ESet 标准差 | std::set 平均耗时 | std::set 标准差 | ESet / std::set |
| --- | ---: | ---: | ---: | ---: | ---: |
| emplace | 0.1599 μs | 0.0179 μs | 0.0909 μs | 0.0040 μs | 1.760 |
| find | 0.1124 μs | 0.0111 μs | 0.1102 μs | 0.0088 μs | 1.020 |
| erase | 0.1678 μs | 0.0454 μs | 0.1141 μs | 0.0088 μs | 1.470 |
| range | 0.2129 μs | 0.0176 μs | 182.4560 μs | 58.7766 μs | 0.001 |
| copy | 1006.3509 μs | 92.0931 μs | 137.0694 μs | 12.4464 μs | 7.342 |
| iterator_walk | 0.0185 μs | 0.0033 μs | 0.0177 μs | 0.0019 μs | 1.050 |

### 基线结论

当前实现的 `range()` 使用秩统计：`count_less_equal(r) - count_less(l)`，复杂度是 $O(\log n)$。

- trace 中 `ESet` 总时间只有 14.7827 ms，而 `std::set` 是 35.7728 ms。
- micro 中 `range` 从 182.4560 μs 降到 0.2129 μs，差距非常明显。

这证明了维持子树规模、用秩差来统计区间个数的做法是有效的。

## 消融实验：把 `range()` 改成线性遍历

### 具体操作

我把 `range()` 临时改成：从 `lower_bound(l)` 开始，逐个 `++it` 统计，直到超过 `r` 为止。除此之外，代码和输入数据都保持不变。

### 消融 Trace 结果

| 容器 | emplace | erase | find | copy | range | iterator | bound | 总计 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| ESet | 9.4508 ms | 1.6032 ms | 1.0393 ms | 0.6198 ms | 24.2218 ms | 0.2623 ms | 0.1039 ms | 37.3012 ms |
| std::set | 5.3941 ms | 1.5405 ms | 1.0234 ms | 0.4725 ms | 26.6101 ms | 0.2432 ms | 0.1005 ms | 35.3842 ms |

### 消融 Micro 结果

| 操作 | ESet 平均耗时 | ESet 标准差 | std::set 平均耗时 | std::set 标准差 | ESet / std::set |
| --- | ---: | ---: | ---: | ---: | ---: |
| emplace | 0.1505 μs | 0.0100 μs | 0.0892 μs | 0.0060 μs | 1.686 |
| find | 0.1168 μs | 0.0141 μs | 0.1138 μs | 0.0145 μs | 1.026 |
| erase | 0.1669 μs | 0.0167 μs | 0.1264 μs | 0.0208 μs | 1.321 |
| range | 190.1133 μs | 81.0026 μs | 168.7240 μs | 4.0322 μs | 1.127 |
| copy | 979.1175 μs | 76.3509 μs | 121.6019 μs | 9.3395 μs | 8.052 |
| iterator_walk | 0.0176 μs | 0.0034 μs | 0.0182 μs | 0.0040 μs | 0.964 |

### 消融结论

这组消融证明了 `range()` 的秩统计实现是必要的：

- 线性 `range()` 会把 trace 中 `range` 从 0.8119 ms 拉高到 24.2218 ms。
- micro 中 `range` 从 0.2129 μs 拉高到 190.1133 μs。

也就是说，虽然线性遍历实现仍然“能工作”，但它显著违反了 README 对 `range()` 的复杂度目标，也会让总性能接近甚至劣于 `std::set`。

## 分析

1. `find` 和 `iterator_walk` 在基线下与 `std::set` 接近，说明红黑树查找和迭代器移动的核心逻辑没有明显偏离 STL。
2. `range()` 的收益必须通过实验来证明。基线与消融对比显示，秩统计版本在 `range` 上比线性遍历快了两个数量级以上，这就是维护子树规模的直接价值。
3. `emplace`、`erase` 和 `copy` 的常数仍然偏大，尤其是 `copy`。当前是逐元素重建，因此仍是 $O(n\log n)$，而 STL 的树结构复制更快。
4. `std::set` 在这个 benchmark 里对 `range()` 的实现路径并不占优，因为 benchmark 使用了 `std::distance` 统计区间长度，迭代器距离计算是线性的，这也解释了为何 `ESet` 的秩统计版本能在 micro 中明显胜出。
