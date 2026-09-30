#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "s2/s2cell_id.h"
#include "s2/s2cell_union.h"
#include "s2/s2latlng.h"
#include "s2/s2loop.h"
#include "s2/s2polygon.h"
#include "s2/s2region_coverer.h"

// Test coordinates use (longitude, latitude) in degrees.
namespace {
  auto PointFromLngLat(double lng_degrees, double lat_degrees) -> S2Point {
    // S2's factory takes latitude first, unlike the (lng, lat) inputs here.
    return S2LatLng::FromDegrees(lat_degrees, lng_degrees).ToPoint();
  }

  auto MakeBox(double west_lng, double south_lat, double east_lng, double north_lat) -> std::unique_ptr<S2Polygon> {
    // These examples do not cross the antimeridian. Edges are great-circle
    // arcs between the four corners; they do not follow latitude parallels.
    // S2Loop closes the ring implicitly; do not duplicate the first vertex.
    const std::vector<S2Point> vertices = { PointFromLngLat(west_lng, south_lat),
                                            PointFromLngLat(east_lng, south_lat),
                                            PointFromLngLat(east_lng, north_lat),
                                            PointFromLngLat(west_lng, north_lat) };
    auto loop = std::make_unique<S2Loop>(vertices);
    loop->Normalize();  // Choose the side with area <= half the sphere.
    return std::make_unique<S2Polygon>(std::move(loop));
  }

  auto Cover(const S2Polygon& polygon, int max_cells) -> S2CellUnion {
    S2RegionCoverer::Options options;
    options.set_max_cells(max_cells);
    options.set_max_level(S2CellId::kMaxLevel);
    options.set_level_mod(1);
    return S2RegionCoverer(options).GetCovering(polygon);
  }

  TEST(S2LatLngTest, LongitudeLatitudeToUnitSphere) {
    EXPECT_EQ(PointFromLngLat(0.0, 0.0), S2Point(1.0, 0.0, 0.0));

    // Distinct longitude and latitude values catch accidental argument swaps.
    const S2Point point = PointFromLngLat(120.5, 30.5);
    EXPECT_NEAR(point.Norm(), 1.0, 1e-15);
    const S2LatLng round_trip(point);
    ASSERT_TRUE(round_trip.is_valid());
    EXPECT_NEAR(round_trip.lng().degrees(), 120.5, 1e-12);
    EXPECT_NEAR(round_trip.lat().degrees(), 30.5, 1e-12);

    const S2CellId leaf(point);
    ASSERT_TRUE(leaf.is_valid());
    EXPECT_EQ(leaf.level(), S2CellId::kMaxLevel);

    // Geographic coordinates can lie on different S2 cube faces.
    EXPECT_EQ(S2CellId(PointFromLngLat(0.0, 0.0)).face(), 0);
    EXPECT_EQ(S2CellId(PointFromLngLat(90.0, 0.0)).face(), 1);
    EXPECT_EQ(S2CellId(PointFromLngLat(0.0, 90.0)).face(), 2);
  }

  TEST(S2LatLngTest, PointInPolygon) {
    // Four corners spanning 120..121 degrees east and 30..31 degrees north.
    const auto polygon = MakeBox(120.0, 30.0, 121.0, 31.0);
    ASSERT_TRUE(polygon->IsValid());
    // Avoid boundary points: S2 uses its own boundary ownership rules.
    EXPECT_TRUE(polygon->Contains(PointFromLngLat(120.5, 30.5)));
    EXPECT_FALSE(polygon->Contains(PointFromLngLat(121.5, 31.5)));
  }

  TEST(S2LatLngTest, NestedPolygons) {
    const auto outer = MakeBox(120.0, 30.0, 121.0, 31.0);
    const auto inner = MakeBox(120.2, 30.2, 120.8, 30.8);
    ASSERT_TRUE(outer->IsValid());
    ASSERT_TRUE(inner->IsValid());
    EXPECT_TRUE(outer->Intersects(*inner));
    EXPECT_TRUE(outer->Contains(*inner));
    EXPECT_FALSE(inner->Contains(*outer));
  }

  TEST(S2LatLngTest, OverlappingPolygons) {
    const auto a = MakeBox(120.0, 30.0, 121.0, 31.0);
    const auto b = MakeBox(120.8, 30.8, 121.5, 31.5);
    ASSERT_TRUE(a->IsValid());
    ASSERT_TRUE(b->IsValid());
    EXPECT_TRUE(a->Intersects(*b));
    EXPECT_FALSE(a->Contains(*b));
    EXPECT_FALSE(b->Contains(*a));
  }

  TEST(S2LatLngTest, DisjointPolygons) {
    const auto a = MakeBox(120.0, 30.0, 121.0, 31.0);
    const auto b = MakeBox(122.0, 32.0, 123.0, 33.0);
    ASSERT_TRUE(a->IsValid());
    ASSERT_TRUE(b->IsValid());
    EXPECT_FALSE(a->Intersects(*b));
    EXPECT_FALSE(a->Contains(*b));
    EXPECT_FALSE(b->Contains(*a));
  }

  TEST(S2LatLngTest, CoveringsContainInteriorSamples) {
    const auto polygon = MakeBox(120.0, 30.0, 121.0, 31.0);
    ASSERT_TRUE(polygon->IsValid());
    // Compare a small cell budget for storage with a larger query budget.
    const S2CellUnion stored = Cover(*polygon, 4);
    const S2CellUnion query = Cover(*polygon, 50);

    for (const auto* covering : { &stored, &query }) {
      const int max_cells = covering == &stored ? 4 : 50;
      SCOPED_TRACE(::testing::Message() << "max_cells=" << max_cells);
      ASSERT_FALSE(covering->empty());
      ASSERT_TRUE(covering->IsValid());
      EXPECT_TRUE(covering->IsNormalized());
      // max_cells is a target, not a hard limit for every possible input.
      std::cout << "max_cells=" << max_cells << ", covering size=" << covering->size() << '\n';
      for (const S2CellId id : covering->cell_ids()) {
        ASSERT_TRUE(id.is_valid());
        EXPECT_GE(id.level(), 0);
        EXPECT_LE(id.level(), S2CellId::kMaxLevel);
        const S2LatLng center(id.ToPoint());
        std::cout << "  CellId=" << id.id() << " token=" << id.ToToken() << " path=" << id.ToString()
                  << " face=" << id.face() << " level=" << id.level() << " center(lng,lat)=(" << center.lng().degrees()
                  << ", " << center.lat().degrees() << ")"
                  << " range=[" << id.range_min().id() << ", " << id.range_max().id() << "]\n";
      }

      // Check a grid strictly inside the polygon; extra coverage is allowed.
      for (double lng : { 120.1, 120.5, 120.9 }) {
        for (double lat : { 30.1, 30.5, 30.9 }) {
          SCOPED_TRACE(::testing::Message() << "point(lng,lat)=(" << lng << ", " << lat << ")");
          const S2Point point = PointFromLngLat(lng, lat);
          ASSERT_TRUE(polygon->Contains(point));
          EXPECT_TRUE(covering->Contains(point));
        }
      }
    }
    EXPECT_TRUE(stored.Intersects(query));
  }

  TEST(S2LatLngTest, CellIdAncestorAndDescendantRanges) {
    const double longitude = 120.5;
    const double latitude = 30.5;
    const S2CellId leaf(PointFromLngLat(longitude, latitude));
    ASSERT_TRUE(leaf.is_valid());
    ASSERT_TRUE(leaf.is_leaf());
    const S2CellId parent = leaf.parent(29);
    const S2CellId grandparent = parent.parent();

    const auto print_cell = [](const std::string& label, S2CellId id) {
      std::cout << "  " << label << " level=" << id.level() << " CellId=" << id.id() << " hex=0x" << std::hex << id.id()
                << std::dec << " token=" << id.ToToken() << " path=" << id.ToString() << " range=[" << id.range_min().id()
                << ", " << id.range_max().id() << "]\n";
    };

    std::cout << "\npoint(lng,lat)=(" << longitude << ", " << latitude << ")\n"
              << "Ancestor chain (coarse to fine):\n";
    print_cell("grandparent", grandparent);
    print_cell("parent", parent);
    print_cell("leaf", leaf);

    for (const S2CellId ancestor : { grandparent, parent }) {
      SCOPED_TRACE(::testing::Message() << "ancestor=" << ancestor.id());
      std::cout << "\nChildren of L" << ancestor.level() << " CellId=" << ancestor.id()
                << " (Hilbert traversal order, increasing CellId):\n";
      for (int child_index = 0; child_index < 4; ++child_index) {
        const S2CellId child = ancestor.child(child_index);
        print_cell(
            "child[" + std::to_string(child_index) + "]" + (child.contains(leaf) ? " [selected leaf path]" : ""), child);
        EXPECT_EQ(child.parent(), ancestor);
        EXPECT_TRUE(ancestor.contains(child));
        if (child_index > 0) {
          const S2CellId previous = ancestor.child(child_index - 1);
          EXPECT_LT(previous.id(), child.id());
          EXPECT_LT(previous.range_max().id(), child.range_min().id());
        }
      }

      // A parent's ID is between its middle two children's IDs. Numeric ID
      // order therefore interleaves levels instead of grouping cells by level.
      std::cout << "Numeric order including the parent:\n  " << ancestor.child(0).id() << " (child[0]) < "
                << ancestor.child(1).id() << " (child[1]) < " << ancestor.id() << " (parent) < " << ancestor.child(2).id()
                << " (child[2]) < " << ancestor.child(3).id() << " (child[3])\n";
      EXPECT_LT(ancestor.child(1).id(), ancestor.id());
      EXPECT_LT(ancestor.id(), ancestor.child(2).id());
    }

    std::cout << "\nContainment uses the inclusive descendant ID range:\n  " << parent.range_min().id()
              << " <= " << leaf.id() << " (leaf) <= " << parent.range_max().id() << '\n'
              << "  parent.contains(leaf)=" << (parent.contains(leaf) ? "true" : "false")
              << ", leaf.contains(parent)=" << (leaf.contains(parent) ? "true" : "false") << '\n'
              << "  range_min/range_max are leaf IDs; a cell's range includes itself and all valid descendants.\n";

    EXPECT_NE(parent, leaf);
    EXPECT_TRUE(parent.contains(leaf));
    EXPECT_FALSE(leaf.contains(parent));

    // A coarse stored cell can be found by querying the fine cell's ancestors.
    EXPECT_EQ(leaf.parent(parent.level()), parent);
    // A coarse query cell can find fine stored cells by scanning this ID range.
    EXPECT_LE(parent.range_min().id(), leaf.id());
    EXPECT_GE(parent.range_max().id(), leaf.id());
    EXPECT_EQ(S2CellId::FromToken(leaf.ToToken()), leaf);
  }
}  // namespace
