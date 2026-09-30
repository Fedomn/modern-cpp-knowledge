# S2 经纬度单元测试

测试使用 `(经度, 纬度)`，单位为度，例如 `(120.5, 30.5)`。
`PointFromLngLat(lng, lat)` 内部调用 `S2LatLng::FromDegrees(lat, lng).ToPoint()`。
S2、Abseil 和 GoogleTest 均从本仓库 `deps` 中的上游源码编译。
Boost 也来自 `deps`，但只用头文件：`mysql_point_mbr_test.cpp` 复现 MySQL 的 MBR 计算时依赖 Boost.Geometry。

`MakeBox(west_lng, south_lat, east_lng, north_lat)` 以四个经纬度角点构造 Polygon。
基准面 A 的角点为 `(120,30)`、`(121,30)`、`(121,31)`、`(120,31)`；
`(120.5,30.5)` 位于内部，`(121.5,31.5)` 位于外部。
S2 的边沿大圆弧连接角点；用例中的 Polygon 不跨越 ±180° 经线，采样点避开边界。

## 依赖版本

S2 0.14.0 为其 `FETCH_ABSEIL` 固定了 Abseil `20250814.1`，Makefile 跟随这对组合，
并复用仓库已有的 GoogleTest 版本：

| 依赖 | tag | 源码目录 |
| --- | --- | --- |
| S2Geometry | `v0.14.0` | `deps/s2geometry` |
| Abseil | `20250814.1` | `deps/abseil-cpp` |
| GoogleTest | `release-1.11.0` | `deps/googletest` |

OceanBase 的 `deps/init/*.deps` 目前仍固定在 `devdeps-s2geometry-0.10.0` 和
`devdeps-abseil-cpp-20211102.0`，与本仓库的版本不同。

构建需要 Git、CMake 3.18+（S2 自身的最低要求）、支持 C++17 的编译器和系统 OpenSSL 开发包
（RHEL 系为 `openssl-devel`，Debian 系为 `libssl-dev`）。
所有 C++ 库使用当前编译器构建，保留编译器默认 ABI。

## 构建并运行

在仓库根目录执行：

```bash
make deps
bash test/src/gis/build_s2_test.sh --gtest_filter=S2LatLngTest.PointInPolygon
```

S2 测试是单测的一部分，首次配置需要 `make deps` 的全部源码依赖；
只跑 S2 时可以按「只构建 S2」一节的独立配置，只需 `make deps-s2`。
`make deps-s2` 只下载缺失的目录，可以重复执行；现有源码目录不会被重置，
便于直接修改 S2 源码进行实验。

一条命令构建并运行全部 12 个用例（同一个 `gis_test` 目标）：

```bash
bash test/src/gis/build_s2_test.sh
```

`make test-s2` 也会运行全部用例。只查看 CellId 的层级和排序输出：

```bash
bash test/src/gis/build_s2_test.sh --gtest_filter=S2LatLngTest.CellIdAncestorAndDescendantRanges
```

该用例打印选中点的 L28 祖先、L29 父 cell、L30 叶子，以及两个祖先各自的四个子 cell。
每行包含十进制 ID、十六进制 ID、token、层级路径和后代 ID 闭区间，选中叶子所在的分支会标记出来。
输出还会直接展示 `child[0] < child[1] < parent < child[2] < child[3]` 的数值排序；
父子关系通过包含范围判断，ID 的排序会穿插不同层级。

脚本配置仓库顶层 CMake，以 Debug 模式构建到 `build/`，默认并行度为 4，沿用已有构建目录的
编译器和单测开关。首次配置时可通过 `CC` / `CXX` 指定编译器；切换编译器请使用新的构建目录：

```bash
CC=gcc CXX=g++ S2_TEST_BUILD_DIR=/tmp/s2-debug S2_TEST_JOBS=8 \
  bash test/src/gis/build_s2_test.sh
```

构建后可直接运行二进制或 CTest：

```bash
./build/test/src/gis/gis_test --gtest_list_tests
./build/test/src/gis/gis_test --gtest_filter=S2LatLngTest.CoveringsContainInteriorSamples
ctest --test-dir build -R '^gis\.' --output-on-failure
```

常规的 `mcpp_ENABLE_UNIT_TESTING=ON` 构建默认启用 S2，`test/CMakeLists.txt` 注册 GIS 子目录。

## 只构建 S2

`test/src/gis` 也可以单独配置，这条路径只编 S2 和测试本身：Boost 只用头文件，不编 Boost 库，也不需要 brpc。
S2 依赖来自 `make deps-s2`，Boost 源码树来自 `make deps`：

```bash
cmake -S test/src/gis -B build/s2-only -DCMAKE_BUILD_TYPE=Debug
cmake --build build/s2-only --target gis_test --parallel 4
ctest --test-dir build/s2-only -R '^gis\.' --output-on-failure
```

`cmake/s2.cmake` 用 `add_subdirectory` 构建 Abseil 和 S2；S2 0.14.0 直接复用已有的
`absl::` 目标，不再调用 `find_package(absl)`。S2、Abseil 和单测统一使用 C++17，
避免 Abseil 的 `string_view` 等类型产生 ABI 差异。Abseil 在 configure 阶段用
`try_compile` 探测 C++ 版本，该探测在部分调用路径上拿不到 `CMAKE_CXX_STANDARD`，
所以 `cmake/s2.cmake` 会额外显式追加 `-std=c++17`。

## 真实 CellID 与文档编码的对应

`s2_cell_id_encoding_test.cpp` 的 `S2CellIdEncodingTest.RealCellIdsAndFiveBitExample`
专门展示“真实库生成的 CellID → 文档中的编码形式”。位布局及 5-bit 样例见
02 文档链接的 04_S2与Hilbert曲线原理.md §1.1/1.2

```bash
bash test/src/gis/build_s2_test.sh --gtest_filter=S2CellIdEncodingTest.RealCellIdsAndFiveBitExample
```

用例先从经纬度构造叶子，再对 `(120,30)..(121,31)` 的多边形真正调用 `GetCovering(max_cells=4)`。
对每个返回的 CellID 打印十进制、固定 16 位十六进制、token、路径和以下位拆分：

```text
[face:3][Hilbert child path:2*k][level marker:1][zeros:60-2*k]
```

例如实际 covering 中的一个 cell：

```text
CellID = 3768105513225551872
hex16  = 0x344b000000000000
token  = 344b
path   = 1/220211
fields = [001][10 10 00 10 01 01][1][48 个 0]
          face       路径      标记    低位补零
level  = (60 - 48) / 2 = 6
文档的 (C[i], MBR(G), PK) -> (3768105513225551872, MBR(G), 101)
```

`path` 的每个四进制数字表示一次子格选择，分别对应两位二进制；`token` 是完整十六进制表示去掉末尾的 0。
MBR 和 PK 是复合索引键的另外两个字段，不在 S2 CellID 的 64 位中。
公共打印函数位于 `s2_cell_id_encoding.h`，原 KV 用例中的 P/A/B/Y/M 也会打印同样的位布局。

为逐项对应文档的 `11 → 12 → 16`、`[9,15]`，测试取一个真实 L28 cell 的两层子树：
高 59 位相同，低 5 位恰好复现教学模型。文档的局部 level 0/1/2 对应真实 L28/L29/L30，
因此教学中的叶子在这个映射里也确实是 S2 叶子，无须改变库的最大层级。

```text
prefix = 3768589017835032128
真实 CellID = prefix + 教学编码
```

| 教学编码 | 低 5 位 | 真实 level | 真实 CellID |
| --- | --- | --- | --- |
| 根 16 | `10000` | 28 | 3768589017835032144 |
| 父 12 | `01100` | 29 | 3768589017835032140 |
| 最小叶 9 | `01001` | 30 | 3768589017835032137 |
| 叶 11 | `01011` | 30 | 3768589017835032139 |
| 叶 13 | `01101` | 30 | 3768589017835032141 |
| 最大叶 15 | `01111` | 30 | 3768589017835032143 |

于是文档的范围 `[9,15]` 对应真实范围 `[3768589017835032137,3768589017835032143]`，
父 cell 自身 `3768589017835032140` 也在范围内。`prefix+10`、`prefix+14` 的层级标记落在奇数 bit 位，
不是合法 S2 CellID，说明一个整数区间不等于其中每个整数都是合法 cell。
这种映射仅用于同一前缀下的教学对照；完整索引键仍使用全部 64 位。
局部根是 L28，完整查询还要枚举它之上的 L27..L0 祖先，不能把教学模型的局部根当成整个 face。

测试把从整数解出的 face、level、child path、祖先和区间端点与 S2 API 对比，
再从解码字段重建原始 ID，并检查 token/路径以及 cell 中心经纬度的往返转换。

## MySQL 的 kPoint MBR 对照

`mysql_point_mbr_test.cpp` 用 Boost.Geometry 复现 MySQL `sql/gis/mbr_utils.cc` 里 kPoint 的 MBR 计算：
`cartesian_envelope()` 调 `bg::envelope(Cartesian_point, Cartesian_box)`（mbr_utils.cc:114），
`geographic_envelope()` 调 `bg::envelope(Geographic_point, Geographic_box)` 且不传 andoyer 策略
（mbr_utils.cc:197）。类型注册与 `sql/gis/geometries_traits.h` 一致：笛卡尔用 `cs::cartesian`，
地理用 `cs::geographic<radian>`，所以地理坐标在内存里是弧度。

该用例编在同一个 `gis_test` 目标里，直接用 Boost 头文件（`deps/boost`，由 `make deps` 拉取，不编 Boost 库）；
缺 Boost 源码树时配置会直接报错：

```bash
cmake --build build --target gis_test --parallel 4
./build/test/src/gis/gis_test --gtest_filter=MysqlPointMbrTest.*
```

`(120.5, 30.5)` 这个点在两种 CS 下拿到的都是 collapsed box，差别只在坐标单位与 CS 标记：

| CS | 内存中的 MBR | 写入索引前 `from_radians()`（rtree_support.cc:392） |
| --- | --- | --- |
| `kCartesian` | min=(120.5, 30.5) max=(120.5, 30.5) | 不做转换，直接是 SRS 单位 |
| `kGeographic` | min=(2.10312, 0.532325) max=(同上)，弧度 | 转回度 = min=(120.5, 30.5) max=(120.5, 30.5) |

两种 CS 下 `mbr_is_point()` 都是 true，`mbr_is_line()` 和 `mbr_is_empty()` 都是 false。
单点即使显式传 andoyer 策略，结果也是同一个 collapsed box，所以 kPoint 分支不传策略没有影响。

kPolygon 就不同了：笛卡尔分支不传策略（mbr_utils.cc:119-121），地理分支要传 andoyer 策略
（mbr_utils.cc:203-205）。`(0,45)-(90,45)-(90,55)-(0,55)` 这个经纬度"矩形"：

| CS | MBR | 说明 |
| --- | --- | --- |
| `kCartesian` | min=(0, 45) max=(90, 55) | 环上顶点的包围盒 |
| `kGeographic` | min=(0, 45) max=(90, 63.673209) | 北边大地线向北凸出，抬高上界纬度 |

折线同理，`(0,45)-(90,45)` 的 MBR 上界纬度是 54.763085 度，高于端点纬度 45 度。

加上这 5 个用例后，`gis_test` 里共 13 个用例（8 个 S2 + 5 个 MBR），`ctest -R '^gis\.'` 全部注册。
