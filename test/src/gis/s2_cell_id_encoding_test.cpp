#include "s2_cell_id_encoding.h"

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "s2/s2latlng.h"
#include "s2/s2loop.h"
#include "s2/s2polygon.h"
#include "s2/s2region_coverer.h"

namespace {
  void CheckEncoding(S2CellId cell) {
    ASSERT_TRUE(cell.is_valid());
    SCOPED_TRACE(::testing::Message() << "CellID=" << cell.id() << " path=" << cell.ToString());
    const auto decoded = s2_test::DecodeCellId(cell);
    EXPECT_EQ(decoded.face, cell.face());
    EXPECT_EQ(decoded.level, cell.level());
    EXPECT_EQ(decoded.marker, cell.lsb());
    const uint64_t reconstructed =
        (static_cast<uint64_t>(decoded.face) << 61) | (decoded.path << (decoded.marker_bit + 1)) | decoded.marker;
    EXPECT_EQ(reconstructed, cell.id());

    std::string path = std::to_string(decoded.face) + '/';
    for (int level = 1; level <= decoded.level; ++level) {
      const int child = static_cast<int>((decoded.path >> (2 * (decoded.level - level))) & 3);
      EXPECT_EQ(child, cell.child_position(level));
      path += static_cast<char>('0' + child);
    }
    EXPECT_EQ(path, cell.ToString());
    EXPECT_EQ(S2CellId::FromDebugString(path), cell);
    EXPECT_EQ(S2CellId::FromToken(cell.ToToken()), cell);
    EXPECT_EQ(cell.id() - (decoded.marker - 1), cell.range_min().id());
    EXPECT_EQ(cell.id() + (decoded.marker - 1), cell.range_max().id());

    // Truncate the path, move the marker and clear the suffix in place.
    // Compare the bit calculation against S2's actual parent() implementation.
    if (decoded.level > 0) {
      const uint64_t parent_marker = decoded.marker << 2;
      EXPECT_EQ((cell.id() & ~(parent_marker - 1)) | parent_marker, cell.parent().id());
    }
  }

  TEST(S2CellIdEncodingTest, RealCellIdsAndFiveBitExample) {
    // Read alongside 02_OB在KV上的实现细节.md section 3.1 and its linked
    // 04_S2与Hilbert曲线原理.md sections 1.1/1.2 (the 5-bit teaching model).
    const S2CellId point_cell(S2LatLng::FromDegrees(30.5, 120.5));
    ASSERT_TRUE(point_cell.is_leaf());
    for (int level = 0; level <= S2CellId::kMaxLevel; ++level) {
      CheckEncoding(point_cell.parent(level));
    }
    for (int face = 0; face < S2CellId::kNumFaces; ++face) {
      CheckEncoding(S2CellId::FromFace(face));  // Includes IDs above INT64_MAX.
    }

    const S2CellId point_parent = point_cell.parent();
    std::cout << "\n[1] 经纬度 -> S2 真实 CellID -> 文档的 64-bit 编码形式\n"
              << "  [face:3][Hilbert child path:2*k][level marker:1][zeros:60-2*k]\n"
              << "  point(lng,lat)=(120.5, 30.5) -> CellID=" << point_cell.id() << " token=" << point_cell.ToToken()
              << " path=" << point_cell.ToString() << '\n'
              << s2_test::FormatCellIdEncoding(point_cell) << "  point_cell.parent(): level=" << point_parent.level()
              << " CellID=" << point_parent.id() << " token=" << point_parent.ToToken()
              << " path=" << point_parent.ToString() << '\n'
              << s2_test::FormatCellIdEncoding(point_parent)
              << "  parent() 去掉最后一对子格路径位，将层级标记左移 2 位，低位清零。\n"
              << "  path 中的每个 0/1/2/3 对应一对 00/01/10/11，token 是完整十六进制去掉末尾的 0。\n";

    // Also decode IDs actually selected by GetCovering(), not only point IDs.
    std::vector<S2Point> vertices;
    for (const auto& [lng, lat] :
         std::vector<std::pair<double, double>>{ { 120.0, 30.0 }, { 121.0, 30.0 }, { 121.0, 31.0 }, { 120.0, 31.0 } }) {
      vertices.push_back(S2LatLng::FromDegrees(lat, lng).ToPoint());
    }
    auto loop = std::make_unique<S2Loop>(vertices);
    loop->Normalize();
    const S2Polygon polygon(std::move(loop));
    ASSERT_TRUE(polygon.IsValid());
    S2RegionCoverer::Options options;
    options.set_max_cells(4);
    options.set_max_level(S2CellId::kMaxLevel);
    const auto covering = S2RegionCoverer(options).GetCovering(polygon);
    ASSERT_FALSE(covering.empty());
    const auto mbr = polygon.GetRectBound();
    std::cout << "\n[2] GetCovering(max_cells=4) 自动生成的 CellID -> (CellId, MBR, PK)\n"
              << "  G=Polygon((120,30),(121,30),(121,31),(120,31)), PK=101\n"
              << "  MBR(G): lng=[" << mbr.lng_lo().degrees() << ", " << mbr.lng_hi().degrees() << "], lat=["
              << mbr.lat_lo().degrees() << ", " << mbr.lat_hi().degrees() << "]\n";
    int index = 0;
    for (S2CellId cell : covering) {
      CheckEncoding(cell);
      std::cout << "  C[" << index++ << "]=" << cell.id() << " level=" << cell.level() << " token=" << cell.ToToken()
                << " path=" << cell.ToString() << '\n'
                << s2_test::FormatCellIdEncoding(cell) << "    文档的 (C[i], MBR(G), PK) -> (" << cell.id()
                << ", MBR(G), 101)\n";
    }
    std::cout << "  MBR(G) 由 Geometry 另外计算，PK 来自主表；二者不包含在 S2 的 64 位 CellID 中。\n";

    // Embed the entire 5-bit example in the final two levels of a real subtree.
    // All cells share the high 59 bits (3 face bits + 28*2 path bits).
    // Local levels 0/1/2 therefore correspond to real S2 levels 28/29/30.
    const S2CellId root = point_cell.parent(28);
    const S2CellId q = root.child(1);
    const uint64_t prefix = root.id() & ~uint64_t{ 31 };
    const std::array<std::pair<int, S2CellId>, 6> mappings = {
      { { 16, root }, { 12, q }, { 9, q.child(0) }, { 11, q.child(1) }, { 13, q.child(2) }, { 15, q.child(3) } }
    };
    ASSERT_EQ(root.level(), 28);
    ASSERT_EQ(q.level(), 29);
    std::cout << "\n[3] 把文档的 5-bit 样例嵌入真实 S2 子树\n"
              << "  文档局部 level 0/1/2 -> 真实 S2 level 28/29/30，叶子上限仍为 30。\n"
              << "  共同高 59 位=" << std::bitset<64>(prefix).to_string().substr(0, 59) << '\n'
              << "  prefix=" << prefix << "，在此子树中：真实 CellID = prefix + 教学编码。\n";
    for (const auto& [toy, cell] : mappings) {
      CheckEncoding(cell);
      EXPECT_EQ(cell.id() & ~uint64_t{ 31 }, prefix);
      EXPECT_EQ(cell.id() & 31, static_cast<uint64_t>(toy));
      EXPECT_EQ(cell.id(), prefix + toy);
      const S2LatLng center(cell.ToPoint());
      // Recover the same cell from geographic coordinates through the library.
      EXPECT_EQ(S2CellId(center).parent(cell.level()), cell);
      std::cout << "  toy=" << toy << " bits5=" << std::bitset<5>(toy) << " -> real L" << cell.level()
                << " CellID=" << cell.id() << " token=" << cell.ToToken() << " path=" << cell.ToString() << '\n'
                << s2_test::FormatCellIdEncoding(cell);
    }

    const S2CellId leaf11 = q.child(1);
    EXPECT_TRUE(leaf11.is_leaf());
    EXPECT_EQ(leaf11.parent(), q);
    EXPECT_EQ(q.parent(), root);
    EXPECT_EQ(q.range_min(), q.child(0));
    EXPECT_EQ(q.range_max(), q.child(3));
    std::cout << "\n[4] 教学范围/祖先查找 -> 真实 UInt64 扫描条件\n"
              << "  01011(11) -> parent 01100(12) -> parent 10000(16)\n"
              << "  " << leaf11.id() << " -> parent " << q.id() << " -> parent " << root.id() << '\n'
              << "  Q 的 lsb=" << q.lsb() << ": [Q-(lsb-1), Q+(lsb-1)]\n"
              << "  教学: [12-3, 12+3] = [9,15]\n"
              << "  真实: [" << q.id() << "-3, " << q.id() << "+3] = [" << q.range_min().id() << ", " << q.range_max().id()
              << "]\n"
              << "  教学: CellId=16 OR 9<=CellId<=15\n"
              << "  真实: CellId=" << root.id() << " OR " << q.range_min().id() << "<=CellId<=" << q.range_max().id()
              << "（局部子树；完整查询还需枚举 L27..L0 祖先）\n";
    const std::vector<S2CellId> sorted = { q.child(0), q.child(1), q, q.child(2), q.child(3), root };
    for (std::size_t i = 1; i < sorted.size(); ++i) {
      EXPECT_LT(sorted[i - 1].id(), sorted[i].id());
    }
    // 10/14 are invalid in the teaching model, even after restoring the prefix.
    // They put the marker at bit 1, whereas S2 markers occupy even bit positions.
    EXPECT_FALSE(S2CellId(prefix + 10).is_valid());
    EXPECT_FALSE(S2CellId(prefix + 14).is_valid());
    std::cout << "  同序映射: 9 < 11 < 12 < 13 < 15 < 16；区间中的 10/14 对应无效 CellID。\n"
              << "  prefix+toy 仅用于本例固定前缀，不能对任意 CellID 去掉高位后当作完整索引键。\n";
  }
}  // namespace
