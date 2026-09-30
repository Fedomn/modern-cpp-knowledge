#include "s2/s2region_term_indexer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "s2/s2cell_id.h"
#include "s2/s2cell_union.h"
#include "s2/s2latlng.h"
#include "s2/s2loop.h"
#include "s2/s2polygon.h"
#include "s2/s2region_coverer.h"

// 同一个区域有两种等价的 S2 表示：
//   1. S2RegionCoverer::GetCovering() 直接返回 S2CellUnion（64-bit cell id 集合）；
//   2. S2RegionTermIndexer 把祖先/后代关系编码成字符串 term，供倒排索引使用。
//
// 分工：S2RegionCoverer 是通用底座（几何 -> cell），不假设目标存储提供哪些查询
// 原语；S2RegionTermIndexer 是面向倒排索引的适配层（几何 -> 字符串），它复用同一个
// S2RegionCoverer 求 covering，额外只决定「哪些 cell 生成哪类 term」。它真正适用的
// 场景是：目标存储只有「term -> posting list」的等值查找（文本/IR 索引），拿不到
// 有序 CellID 上的范围扫描；或者空间 term 需要和文本 term 共用一个键空间。
//
// 它把范围查询换成了 term 点查，等价关系、生成规则与代价：
//   写入一个文档时，对它的每个 covering cell C 生成：
//     1 个 covering term：prefix + '$' + token(C)，表示「C 是我的覆盖 cell」；
//     若干 ancestor term：prefix + token(X)，X 取 C 自身以及逐层向上的祖先，
//       直到 min_level（默认 4）为止；相邻 covering cell 共享的祖先会去重。
//   查询时对查询区域的每个 covering cell Q 生成：
//     1 个 ancestor term：token(Q)；
//     若干 covering term：token(Q.parent(level))，level 从 Q 的上一层到 min_level。
//   于是全部命中只有两种相等关系，且永远发生在「查询侧生成的 term」与「文档侧
//   已存的 term」之间：
//     ancestor term 相等（无 '$'） ⟺ 文档 cell ⊆ 查询 cell（原 range scan）；
//     covering term 相等（有 '$'） ⟺ 文档 cell ⊃ 查询 cell（原本就是祖先点查）。
//
// 用下面 [2] 段的 Q4「查 C 内部有哪些 doc」来读上面两条规则：
//   C 的 covering 是 341/343/345/346c/35b4/35cb/369/36a4（L4..L6），查询 C 生成
//   8 个 ancestor term + 5 个 covering term，实际命中全部来自 ancestor term：
//     k345  -> doc {A, B, C}   A/B/C 的 ancestor term 里都有 L4 的 345；
//     k35b4 -> doc {A, C, D}   D 是假阳性；
//     其余 k341/k343/k346c/k35cb/k369/k36a4 -> doc {C}（C 自己也是这些 cell 的子孙）。
//   A/B 不在 C 的覆盖 cell 341 之下（341 只覆盖 C 的西半部分），它们落在 345 与
//   35b4 的 id 区间里——所以 A/B 的 term 里没有 k$341，只有 k345 / k35b4 这些
//   ancestor term；k$341 是「C 对查询声明自己更大」的键，只有当查询区域比 C 小、
//   查询侧的覆盖 cell 把自己的祖先数到 341 时才会命中 C。
//   D 的假阳性来自覆盖近似：D 的 cell 35b21 落在 C 的覆盖 cell 35b4 的 id 区间内
//   （覆盖超出 C 的真实边界），所以候选 {A,B,C,D} ⊋ 精确相交 {A,B,C}。候选超集是
//   覆盖近似的性质，与 term/范围扫的编码无关；要严格「C 内部」还需回表换精确谓词。
//   + 收益：不要求存储层有序；直接复用倒排索引的增量写、并集、多字段前缀隔离等
//     基础设施；points-only 时点查询退化为单个 term 的等值查找。
//   - 索引体积按层数放大：默认参数下点文档 13 个 term，区域是 O(|covering| × 层数)，
//     而 covering 方案只存 |covering| 行。
//   - 丢失顺序语义：term 是 prefix + token 的字符串（token 去掉末尾 0，变长），不能
//     当有序键做范围/前缀裁剪，也无法与 MBR、PK 组成复合键。
//   - 查询结果是 posting list 的并集，需要按 doc 去重，且是覆盖近似的候选集，必须
//     回表做精确几何判定（本用例用 EXPECT 验证「候选 ⊇ 精确相交」）。
//   - 隐含契约：min_level / max_level / level_mod 必须建索引与查询一致（否则静默
//     丢结果），max_cells 才可以随便改。
//
// 本用例让两者面对同一批文档、同一批查询，打印 cell 数 / term 数 / term 形态，
// 并验证候选文档集合完全一致：差异只在编码方式，不在候选集语义。
namespace {
  auto PointFromLngLat(double lng_degrees, double lat_degrees) -> S2Point {
    return S2LatLng::FromDegrees(lat_degrees, lng_degrees).ToPoint();
  }

  auto MakeBox(double west_lng, double south_lat, double east_lng, double north_lat) -> std::unique_ptr<S2Polygon> {
    // These examples do not cross the antimeridian. Edges are great-circle arcs
    // between the four corners. S2Loop closes the ring implicitly; do not
    // duplicate the first vertex.
    const std::vector<S2Point> vertices = { PointFromLngLat(west_lng, south_lat),
                                            PointFromLngLat(east_lng, south_lat),
                                            PointFromLngLat(east_lng, north_lat),
                                            PointFromLngLat(west_lng, north_lat) };
    auto loop = std::make_unique<S2Loop>(vertices);
    loop->Normalize();  // Choose the side with area <= half the sphere.
    return std::make_unique<S2Polygon>(std::move(loop));
  }

  struct Doc {
    int id = 0;
    std::string label;
    std::unique_ptr<S2Polygon> polygon;
  };

  struct QueryCase {
    std::string label;
    std::vector<std::string> query_terms;
    S2CellUnion query_covering;
    S2Point point;                   // 点查询时用于精确判定
    std::unique_ptr<S2Polygon> box;  // 区域查询时用于精确判定，点查询为 nullptr
  };

  // term 形态是 prefix + [marker] + token；marker 只出现在 covering term 上。
  auto IsCoveringTerm(const std::string& term, const std::string& prefix, char marker) -> bool {
    return term.size() > prefix.size() && term[prefix.size()] == marker;
  }

  auto DescribeTerm(const std::string& term, const std::string& prefix, char marker) -> std::string {
    return std::string(IsCoveringTerm(term, prefix, marker) ? "[covering] " : "[ancestor] ") + term;
  }

  template<typename Container>
  auto JoinIds(const Container& ids) -> std::string {
    std::string text = "{";
    bool first = true;
    for (int id : ids) {
      if (!first)
        text += ',';
      first = false;
      text += std::to_string(id);
    }
    return text + "}";
  }

  TEST(S2RegionTermIndexerTest, TermsVersusCoverings) {
    // prefix 是调用方约定的命名空间，S2 只做字符串拼接，不解释也不校验。
    const std::string prefix = "k";
    const S2RegionTermIndexer::Options options;      // 默认 8/4/16/1，见 s2region_term_indexer.cc 的 Options 构造。
    const char marker = options.marker_character();  // 默认 '$'，用于区分 ancestor term 与 covering term。
    S2RegionTermIndexer indexer(options);
    S2RegionCoverer coverer(options);

    std::cout << "\n=== S2RegionTermIndexer vs covering ===\n"
              << "Options: max_cells=" << options.max_cells() << " min_level=" << options.min_level()
              << " max_level=" << options.max_level() << " true_max_level=" << options.true_max_level()
              << " level_mod=" << options.level_mod() << " marker='" << marker << "'\n";

    // 四个文档的几何关系（后面所有查询都对照它们理解）：
    //   A(120,30)-(121,31)          基准盒，查询都落在它附近；
    //   B(120.2,30.2)-(120.6,30.6)  完全在 A 内，比 A 小；
    //   C(110,25)-(121.2,31.2)      完全包住 A，比 A 大；
    //   D(121.5,30.5)-(122,31.5)    在 A 东侧，与 A/B/C 都不相交。
    // 即 B ⊂ A ⊂ C，D 独立。四者的 covering cell 分布在从 L4 到 L9 的不同层级上，
    // 便于观察「文档 cell 与查询 cell 谁包含谁」两个方向各自命中哪些文档。
    std::vector<Doc> docs;
    docs.push_back(Doc{ 0, "A(120,30)-(121,31)", MakeBox(120.0, 30.0, 121.0, 31.0) });
    docs.push_back(Doc{ 1, "B(120.2,30.2)-(120.6,30.6)", MakeBox(120.2, 30.2, 120.6, 30.6) });
    docs.push_back(Doc{ 2, "C(110,25)-(121.2,31.2)", MakeBox(110.0, 25.0, 121.2, 31.2) });
    docs.push_back(Doc{ 3, "D(121.5,30.5)-(122,31.5)", MakeBox(121.5, 30.5, 122.0, 31.5) });

    // 两种索引结构，同一批文档：covering 方案留 cell，term 方案留字符串倒排。
    std::vector<S2CellUnion> stored_coverings;
    std::vector<std::vector<std::string>> doc_terms;
    std::unordered_map<std::string, std::vector<int>> inverted_index;
    for (const Doc& doc : docs) {
      ASSERT_TRUE(doc.polygon->IsValid()) << doc.label;
      stored_coverings.push_back(coverer.GetCovering(*doc.polygon));
      doc_terms.push_back(indexer.GetIndexTerms(*doc.polygon, prefix));
      for (const std::string& term : doc_terms.back()) {
        inverted_index[term].push_back(doc.id);
      }
    }

    std::cout << "\n[1] 每个文档的两种表示（prefix=\"" << prefix << "\"）\n";
    for (std::size_t i = 0; i < docs.size(); ++i) {
      std::size_t covering_terms = 0;
      for (const std::string& term : doc_terms[i]) {
        if (IsCoveringTerm(term, prefix, marker))
          ++covering_terms;
      }
      std::cout << "  doc " << docs[i].id << ' ' << docs[i].label << ": covering cell=" << stored_coverings[i].size()
                << " 个, index term=" << doc_terms[i].size() << " 个（其中 covering term " << covering_terms << " 个）\n";
      for (const S2CellId id : stored_coverings[i]) {
        std::cout << "    cell id=" << id.id() << " token=" << id.ToToken() << " level=" << id.level() << " range=["
                  << id.range_min().id() << ", " << id.range_max().id() << "]\n";
      }
      for (const std::string& term : doc_terms[i]) {
        std::cout << "    " << DescribeTerm(term, prefix, marker) << '\n';
      }
    }

    // covering 方案的查询，模拟有序索引上的两种访问方式：
    //   方向 1：range scan 扫 [range_min, range_max]，命中「文档 cell 在查询 cell 之内」；
    //   方向 2：对查询 cell 的各级祖先做等值点查，命中「文档 cell 是查询 cell 的祖先」。
    // term 方案把同一对关系物化成了 ancestor term 与 covering term 的等值查找。
    struct CoveringQueryDetail {
      std::set<int> candidates;
      int range_scans = 0;
      int ancestor_lookups = 0;
    };
    const auto query_by_covering = [&](const S2CellUnion& query_covering) -> CoveringQueryDetail {
      CoveringQueryDetail detail;
      for (const S2CellId query_cell : query_covering) {
        const uint64_t lo = query_cell.range_min().id();
        const uint64_t hi = query_cell.range_max().id();
        ++detail.range_scans;
        for (std::size_t i = 0; i < stored_coverings.size(); ++i) {
          for (const S2CellId stored : stored_coverings[i]) {
            if (lo <= stored.id() && stored.id() <= hi)
              detail.candidates.insert(docs[i].id);
          }
        }
        for (int level = query_cell.level() - options.level_mod(); level >= options.min_level();
             level -= options.level_mod()) {
          ++detail.ancestor_lookups;
          const S2CellId ancestor = query_cell.parent(level);
          for (std::size_t i = 0; i < stored_coverings.size(); ++i) {
            for (const S2CellId stored : stored_coverings[i]) {
              if (stored == ancestor)
                detail.candidates.insert(docs[i].id);
            }
          }
        }
      }
      return detail;
    };

    const auto query_by_terms = [&](const std::vector<std::string>& query_terms) {
      std::set<int> candidates;
      for (const std::string& term : query_terms) {
        const auto it = inverted_index.find(term);
        if (it == inverted_index.end())
          continue;
        std::cout << "      命中 term " << term << " -> doc " << JoinIds(it->second) << '\n';
        candidates.insert(it->second.begin(), it->second.end());
      }
      return candidates;
    };

    const S2Point inside_point = PointFromLngLat(120.5, 30.5);  // A/B/C 内部
    const S2Point far_point = PointFromLngLat(121.9, 31.4);     // 只在 D 内部
    auto small_box = MakeBox(120.4, 30.4, 120.7, 30.7);         // A 内部
    ASSERT_TRUE(small_box->IsValid());

    // 点查询的查询 covering：把退化区域（单个叶子）交给同一个 coverer，
    // 得到的就是该点所在的 true_max_level cell，与 GetQueryTerms(point) 的出发点一致。
    const S2CellUnion inside_point_covering =
        coverer.GetCovering(S2CellUnion(std::vector<S2CellId>{ S2CellId(inside_point) }));
    const S2CellUnion far_point_covering = coverer.GetCovering(S2CellUnion(std::vector<S2CellId>{ S2CellId(far_point) }));

    // 三条查询与上面四个文档的关系：
    //   Q1 点(120.5,30.5)：落在 A∩B∩C 内部，三个文档都该被找到（都是「比查询大」，
    //      观察 covering term 方向）；
    //   Q2 小盒(120.4,30.4)-(120.7,30.7)：在 A 内、被 C 包住、与 B 部分重叠；查询
    //      covering 更细，能同时观察到 ancestor term 与 covering term 两侧命中；
    //   Q3 点(121.9,31.4)：只在 D 内，Q1 的三个文档都不该出现；
    //   Q4 直接用 C 的区域查询「C 内部有哪些 doc」：A/B/C 由查询侧生成的 ancestor
    //      term（k345 / k35b4）命中，而不是靠 C 的 covering term k$341；C 的覆盖
    //      还超出 C 的真实边界，D 会以假阳性进入候选，再由精确判定剔除。
    // 点查询走 GetQueryTerms(point)，把该点所在的 true_max_level cell 当查询 covering；
    // 区域查询走 GetQueryTerms(region)，用实际 covering cell 出发。
    std::vector<QueryCase> queries;
    queries.push_back(QueryCase{ "Q1 point(120.5, 30.5)",
                                 indexer.GetQueryTerms(inside_point, prefix),
                                 inside_point_covering,
                                 inside_point,
                                 nullptr });
    queries.push_back(QueryCase{ "Q2 box(120.4,30.4)-(120.7,30.7)",
                                 indexer.GetQueryTerms(*small_box, prefix),
                                 coverer.GetCovering(*small_box),
                                 S2Point(),
                                 std::move(small_box) });
    queries.push_back(QueryCase{
        "Q3 point(121.9, 31.4)", indexer.GetQueryTerms(far_point, prefix), far_point_covering, far_point, nullptr });
    auto query_big_box = MakeBox(110.0, 25.0, 121.2, 31.2);  // 与文档 C 同一块区域
    queries.push_back(QueryCase{ "Q4 box C(110,25)-(121.2,31.2)",
                                 indexer.GetQueryTerms(*query_big_box, prefix),
                                 coverer.GetCovering(*query_big_box),
                                 S2Point(),
                                 std::move(query_big_box) });

    std::cout << "\n[2] 同一批查询：term 倒排索引 vs covering\n";
    std::vector<std::set<int>> term_candidates_per_query;
    for (const QueryCase& query : queries) {
      std::cout << "  " << query.label << '\n'
                << "    term 方案: query covering=" << query.query_covering.size() << " 个 cell -> "
                << query.query_terms.size() << " 个 query term\n";
      term_candidates_per_query.push_back(query_by_terms(query.query_terms));

      const CoveringQueryDetail detail = query_by_covering(query.query_covering);
      std::cout << "    covering 方案: " << detail.range_scans << " 次 range scan + " << detail.ancestor_lookups
                << " 次祖先等值点查\n"
                << "    候选 doc: term=" << JoinIds(term_candidates_per_query.back())
                << " covering=" << JoinIds(detail.candidates) << '\n';

      // 两种编码的候选集必须一致（覆盖近似的假阳性也一致）。
      EXPECT_EQ(term_candidates_per_query.back(), detail.candidates) << query.label;

      // 候选是精确相交的超集：多出来的就是覆盖近似带来的假阳性，需要回表做精确判定。
      std::set<int> exact;
      for (const Doc& doc : docs) {
        const bool hit = (query.box != nullptr) ? query.box->Intersects(*doc.polygon) : doc.polygon->Contains(query.point);
        if (hit)
          exact.insert(doc.id);
      }
      EXPECT_TRUE(std::includes(
          term_candidates_per_query.back().begin(), term_candidates_per_query.back().end(), exact.begin(), exact.end()))
          << query.label;
      std::cout << "    精确相交 doc=" << JoinIds(exact) << "（候选 ⊇ 精确相交）\n";
    }

    ASSERT_EQ(term_candidates_per_query.size(), 4u);
    EXPECT_NE(term_candidates_per_query[0].count(0), 0u);  // 查询点在 A 内，A 必须被找到。
    EXPECT_NE(term_candidates_per_query[2].count(3), 0u);  // 查询点在 D 内，D 必须被找到。
    // Q4「查 C 内部」：A/B/C 都必须被找到（由查询侧的 ancestor term k345 / k35b4 命中）。
    EXPECT_NE(term_candidates_per_query[3].count(0), 0u);
    EXPECT_NE(term_candidates_per_query[3].count(1), 0u);
    EXPECT_NE(term_candidates_per_query[3].count(2), 0u);

    // prefix 只是调用方的命名空间约定：换一个前缀，查询 term 数不变，但零命中，
    // 库不会报任何错。索引与查询必须使用同一个前缀。
    std::set<int> mismatched;
    for (const std::string& term : indexer.GetQueryTerms(inside_point, "x")) {
      const auto it = inverted_index.find(term);
      if (it != inverted_index.end())
        mismatched.insert(it->second.begin(), it->second.end());
    }
    EXPECT_TRUE(mismatched.empty());
    std::cout << "\n[3] 同一查询改用 prefix=\"x\": 零命中（term 字符串对不上，库不报错）\n";
  }

  TEST(S2RegionTermIndexerTest, OptionTradeoffs) {
    const std::string prefix = "p";
    const S2RegionTermIndexer::Options defaults;
    auto box = MakeBox(120.0, 30.0, 121.0, 31.0);
    ASSERT_TRUE(box->IsValid());
    const S2Point point = PointFromLngLat(120.5, 30.5);

    std::cout << "\n[4] 同一个 box 文档 / 点文档在不同选项下的 term 数量\n";

    S2RegionTermIndexer default_indexer(defaults);
    const auto default_index_terms = default_indexer.GetIndexTerms(*box, prefix);
    const auto default_query_terms = default_indexer.GetQueryTerms(*box, prefix);
    std::cout << "  默认:               index term=" << default_index_terms.size()
              << ", query term=" << default_query_terms.size() << '\n';

    // optimize_for_space: 文档侧少存祖先 term（覆盖 cell 自身不再单独建 ancestor term），
    // 查询侧把查询 cell 自身也当 covering term 查，代价从索引转移到查询。
    S2RegionTermIndexer::Options space_options;
    space_options.set_optimize_for_space(true);
    S2RegionTermIndexer space_indexer(space_options);
    const auto space_index_terms = space_indexer.GetIndexTerms(*box, prefix);
    const auto space_query_terms = space_indexer.GetQueryTerms(*box, prefix);
    std::cout << "  optimize_for_space: index term=" << space_index_terms.size()
              << ", query term=" << space_query_terms.size() << '\n';
    EXPECT_LT(space_index_terms.size(), default_index_terms.size());
    EXPECT_GT(space_query_terms.size(), default_query_terms.size());

    // points-only: 索引里只有点，查询无需 covering term（不会有比查询区域更大的文档）。
    S2RegionTermIndexer::Options points_only_options;
    points_only_options.set_index_contains_points_only(true);
    S2RegionTermIndexer points_only_indexer(points_only_options);
    const auto point_index_terms = points_only_indexer.GetIndexTerms(point, prefix);
    const auto point_query_terms = points_only_indexer.GetQueryTerms(point, prefix);
    const std::size_t expected_point_terms =
        static_cast<std::size_t>((defaults.true_max_level() - defaults.min_level()) / defaults.level_mod() + 1);
    std::cout << "  points-only 点文档: index term=" << point_index_terms.size() << "（= 层数 " << defaults.min_level()
              << ".." << defaults.true_max_level() << "）, query term=" << point_query_terms.size() << '\n';
    for (const std::string& term : point_index_terms) {
      std::cout << "    " << DescribeTerm(term, prefix, defaults.marker_character()) << '\n';
    }
    std::cout << "    点查询 term: " << DescribeTerm(point_query_terms.front(), prefix, defaults.marker_character()) << '\n';
    ASSERT_EQ(point_query_terms.size(), 1u);
    ASSERT_EQ(point_index_terms.size(), expected_point_terms);

    // 同一个 L16 cell 内的另一个点在查询时命中，另一个 cell 的点不命中：
    // points-only 查询退化成一次等值查找，与 covering 方案对 L16 cell 做一次点查等价。
    const S2Point same_cell_point = S2CellId(point).parent(defaults.true_max_level()).child(0).ToPoint();
    const S2Point other_cell_point = PointFromLngLat(0.0, 0.0);
    std::unordered_map<std::string, std::vector<int>> index;
    for (const std::string& term : points_only_indexer.GetIndexTerms(same_cell_point, prefix)) {
      index[term].push_back(0);
    }
    for (const std::string& term : points_only_indexer.GetIndexTerms(other_cell_point, prefix)) {
      index[term].push_back(1);
    }
    std::set<int> hits;
    for (const std::string& term : points_only_indexer.GetQueryTerms(point, prefix)) {
      const auto it = index.find(term);
      if (it != index.end())
        hits.insert(it->second.begin(), it->second.end());
    }
    std::cout << "    查询候选 doc=" << JoinIds(hits) << "（0=同 cell 的点, 1=另一个 cell 的点）\n";
    EXPECT_NE(hits.count(0), 0u);
    EXPECT_EQ(hits.count(1), 0u);

    // max_level 决定点文档的 term 数：term 数 ≈ 层数，这正是「索引侧物化祖先闭包」的放大倍数。
    for (const int max_level : { 16, 20, 30 }) {
      S2RegionTermIndexer::Options options;
      options.set_max_level(max_level);
      S2RegionTermIndexer indexer(options);
      std::cout << "  max_level=" << max_level << ": 点文档 index term=" << indexer.GetIndexTerms(point, prefix).size()
                << ", 点查询 query term=" << indexer.GetQueryTerms(point, prefix).size() << '\n';
    }
  }
}  // namespace
