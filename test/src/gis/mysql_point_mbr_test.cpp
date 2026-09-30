// Mirrors the kPoint branch of MySQL's MBR computation.
//
//   sql/gis/mbr_utils.cc:111-115  cartesian_envelope, kPoint:
//                                 bg::envelope(Cartesian_point, Cartesian_box)
//   sql/gis/mbr_utils.cc:196-198  geographic_envelope, kPoint:
//                                 bg::envelope(Geographic_point, Geographic_box)  (no strategy)
//   sql/gis/mbr_utils.cc:269-281  box_envelope dispatches on Geometry::coordinate_system()
//   sql/gis/rtree_support.cc:388-396 stores the geographic MBR after srs->from_radians()
//
// The types below repeat the Boost.Geometry models that
// sql/gis/geometries_traits.h registers for gis::Cartesian_point and
// gis::Geographic_point, so the envelopes computed here are the ones MySQL
// computes.

#include <gtest/gtest.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <cmath>
#include <iostream>
#include <string>

namespace bg = boost::geometry;

namespace {
  // sql/gis/geometries_traits.h: cs::cartesian for Cartesian, and
  // cs::geographic<radian> for Geographic. The in-memory geographic
  // coordinates are radians (sql/gis/wkb.cc:128-153).
  using CartesianPoint = bg::model::point<double, 2, bg::cs::cartesian>;
  using GeographicPoint = bg::model::point<double, 2, bg::cs::geographic<bg::radian>>;
  using CartesianBox = bg::model::box<CartesianPoint>;
  using GeographicBox = bg::model::box<GeographicPoint>;
  using GeographicLinestring = bg::model::linestring<GeographicPoint>;
  // sql/gis/geometries_traits.h registers rings as closed and counterclockwise.
  using CartesianPolygon = bg::model::polygon<CartesianPoint, false, true>;
  using GeographicPolygon = bg::model::polygon<GeographicPoint, false, true>;

  // MySQL converts user coordinates with dd::Spatial_reference_system, where a
  // degree based SRS has angular unit pi/180. M_PI is not visible in a strict
  // -std=c++17 build, so spell out the constant.
  constexpr double kPi = 3.14159265358979323846;
  constexpr double kRadiansPerDegree = kPi / 180.0;

  auto ToRadians(double degrees) -> double {
    return degrees * kRadiansPerDegree;
  }
  auto FromRadians(double radians) -> double {
    return radians / kRadiansPerDegree;
  }

  template<typename Box>
  auto MinX(const Box& mbr) -> double {
    return bg::get<bg::min_corner, 0>(mbr);
  }

  template<typename Box>
  auto MinY(const Box& mbr) -> double {
    return bg::get<bg::min_corner, 1>(mbr);
  }

  template<typename Box>
  auto MaxX(const Box& mbr) -> double {
    return bg::get<bg::max_corner, 0>(mbr);
  }

  template<typename Box>
  auto MaxY(const Box& mbr) -> double {
    return bg::get<bg::max_corner, 1>(mbr);
  }

  // sql/gis/mbr_utils.cc:62-75.
  template<typename Box>
  auto MbrIsEmpty(const Box& mbr) -> bool {
    return std::isnan(MinX(mbr)) && std::isnan(MinY(mbr)) && std::isnan(MaxX(mbr)) && std::isnan(MaxY(mbr));
  }

  template<typename Box>
  auto MbrIsPoint(const Box& mbr) -> bool {
    return MinX(mbr) == MaxX(mbr) && MinY(mbr) == MaxY(mbr);
  }

  template<typename Box>
  auto MbrIsLine(const Box& mbr) -> bool {
    return (MinX(mbr) == MaxX(mbr)) != (MinY(mbr) == MaxY(mbr));
  }

  auto Format(const CartesianBox& mbr) -> std::string {
    return "min=(" + std::to_string(MinX(mbr)) + ", " + std::to_string(MinY(mbr)) + ") max=(" + std::to_string(MaxX(mbr)) +
           ", " + std::to_string(MaxY(mbr)) + ")";
  }

  // Radians are what MySQL keeps in memory, degrees are what it stores in the index.
  auto FormatDegrees(const GeographicBox& mbr) -> std::string {
    return "min=(" + std::to_string(FromRadians(MinX(mbr))) + " deg, " + std::to_string(FromRadians(MinY(mbr))) +
           " deg) max=(" + std::to_string(FromRadians(MaxX(mbr))) + " deg, " + std::to_string(FromRadians(MaxY(mbr))) +
           " deg)";
  }

  auto AndoyerEnvelopeStrategy() {
    return bg::strategy::envelope::geographic<bg::strategy::andoyer, bg::srs::spheroid<double>>(bg::srs::spheroid<double>(
        6378137.0,           /* WGS 84 semi-major axis */
        6356752.314245179)); /* WGS 84 semi-minor axis */
  }

  TEST(MysqlPointMbrTest, CartesianPointEnvelopeIsACollapsedBox) {
    const CartesianPoint point(120.5, 30.5);
    CartesianBox mbr;
    bg::envelope(point, mbr);  // mbr_utils.cc:114

    std::cout << "\n[1] kCartesian + kPoint: bg::envelope(Cartesian_point)\n"
              << "  点 = (120.5, 30.5)，MBR " << Format(mbr) << '\n'
              << "  mbr_is_point=" << MbrIsPoint(mbr) << " mbr_is_line=" << MbrIsLine(mbr)
              << " mbr_is_empty=" << MbrIsEmpty(mbr) << '\n';

    EXPECT_DOUBLE_EQ(MinX(mbr), 120.5);
    EXPECT_DOUBLE_EQ(MinY(mbr), 30.5);
    EXPECT_DOUBLE_EQ(MaxX(mbr), 120.5);
    EXPECT_DOUBLE_EQ(MaxY(mbr), 30.5);
    // 两种 CS 下点 MBR 都是 collapsed box，区别只在坐标单位与 CS 标记。
    EXPECT_TRUE(MbrIsPoint(mbr));
    EXPECT_FALSE(MbrIsLine(mbr));
    EXPECT_FALSE(MbrIsEmpty(mbr));
  }

  TEST(MysqlPointMbrTest, GeographicPointEnvelopeIsACollapsedBoxInRadians) {
    const double longitude_degrees = 120.5;
    const double latitude_degrees = 30.5;
    const GeographicPoint point(ToRadians(longitude_degrees), ToRadians(latitude_degrees));
    GeographicBox mbr;
    bg::envelope(point, mbr);  // mbr_utils.cc:197

    std::cout << "\n[2] kGeographic + kPoint: bg::envelope(Geographic_point)，不传策略\n"
              << "  内存表示(弧度) = (" << MinX(mbr) << ", " << MinY(mbr) << ")\n"
              << "  转回度 = " << FormatDegrees(mbr) << '\n';

    // 内存里的 MBR 就是点自身，只是坐标是弧度。
    EXPECT_DOUBLE_EQ(MinX(mbr), ToRadians(longitude_degrees));
    EXPECT_DOUBLE_EQ(MinY(mbr), ToRadians(latitude_degrees));
    EXPECT_DOUBLE_EQ(MaxX(mbr), ToRadians(longitude_degrees));
    EXPECT_DOUBLE_EQ(MaxY(mbr), ToRadians(latitude_degrees));

    EXPECT_TRUE(MbrIsPoint(mbr));
    EXPECT_FALSE(MbrIsLine(mbr));
    EXPECT_FALSE(MbrIsEmpty(mbr));

    // rtree_support.cc:392-395 writes the index MBR after from_radians(), so the
    // stored numbers are the degrees the user inserted.
    EXPECT_NEAR(FromRadians(MinX(mbr)), longitude_degrees, 1e-12);
    EXPECT_NEAR(FromRadians(MaxY(mbr)), latitude_degrees, 1e-12);
  }

  TEST(MysqlPointMbrTest, GeographicStrategyDoesNotExpandAPoint) {
    const GeographicPoint point(ToRadians(120.5), ToRadians(30.5));
    const auto strategy = AndoyerEnvelopeStrategy();
    GeographicBox without_strategy;
    GeographicBox with_strategy;
    bg::envelope(point, without_strategy);         // mbr_utils.cc:197，点没有传策略
    bg::envelope(point, with_strategy, strategy);  // 折线/多边形走的分支

    std::cout << "\n[3] 单点显式传 andoyer 策略，与不传策略结果相同\n"
              << "  无策略 = " << FormatDegrees(without_strategy) << '\n'
              << "  有策略 = " << FormatDegrees(with_strategy) << '\n';

    EXPECT_DOUBLE_EQ(MinX(without_strategy), MinX(with_strategy));
    EXPECT_DOUBLE_EQ(MinY(without_strategy), MinY(with_strategy));
    EXPECT_DOUBLE_EQ(MaxX(without_strategy), MaxX(with_strategy));
    EXPECT_DOUBLE_EQ(MaxY(without_strategy), MaxY(with_strategy));
  }

  TEST(MysqlPointMbrTest, GeographicStrategyMattersOnlyForCurvedGeometry) {
    // 沿北纬 45 度的大地线会向北凸出，端点自身的纬度反映不出 MBR 上界。
    const GeographicLinestring line = { GeographicPoint(ToRadians(0.0), ToRadians(45.0)),
                                        GeographicPoint(ToRadians(90.0), ToRadians(45.0)) };
    const auto strategy = AndoyerEnvelopeStrategy();
    GeographicBox mbr;
    bg::envelope(line, mbr, strategy);  // mbr_utils.cc:200

    const double max_latitude_degrees = FromRadians(MaxY(mbr));
    std::cout << "\n[4] kGeographic + kLinestring: (0,45)-(90,45)，用 andoyer 策略\n"
              << "  MBR = " << FormatDegrees(mbr) << '\n'
              << "  上界纬度 " << max_latitude_degrees << " 度 > 端点纬度 45 度\n";

    EXPECT_DOUBLE_EQ(FromRadians(MinX(mbr)), 0.0);
    EXPECT_DOUBLE_EQ(FromRadians(MaxX(mbr)), 90.0);
    EXPECT_DOUBLE_EQ(FromRadians(MinY(mbr)), 45.0);
    EXPECT_GT(max_latitude_degrees, 45.0);
    // 单点没有"凸出"，所以 mbr_utils.cc 的 kPoint 分支不传策略。
    EXPECT_FALSE(MbrIsPoint(mbr));
  }

  TEST(MysqlPointMbrTest, PolygonEnvelopeUsesTheGeographicStrategy) {
    // (0,45)-(90,45)-(90,55)-(0,55) 的经纬度"矩形"，环显式闭合。
    // 南、北两条边是大地线，会向北凸出；东、西两条边是经线，不改变纬度。
    CartesianPolygon cartesian_polygon;
    cartesian_polygon.outer() = { CartesianPoint(0.0, 45.0),
                                  CartesianPoint(90.0, 45.0),
                                  CartesianPoint(90.0, 55.0),
                                  CartesianPoint(0.0, 55.0),
                                  CartesianPoint(0.0, 45.0) };
    GeographicPolygon geographic_polygon;
    geographic_polygon.outer() = { GeographicPoint(ToRadians(0.0), ToRadians(45.0)),
                                   GeographicPoint(ToRadians(90.0), ToRadians(45.0)),
                                   GeographicPoint(ToRadians(90.0), ToRadians(55.0)),
                                   GeographicPoint(ToRadians(0.0), ToRadians(55.0)),
                                   GeographicPoint(ToRadians(0.0), ToRadians(45.0)) };

    // mbr_utils.cc:119-121 多边形在笛卡尔下同样不传策略。
    CartesianBox cartesian_mbr;
    bg::envelope(cartesian_polygon, cartesian_mbr);
    // mbr_utils.cc:203-205 地理多边形要传 andoyer 策略。
    const auto strategy = AndoyerEnvelopeStrategy();
    GeographicBox geographic_mbr;
    bg::envelope(geographic_polygon, geographic_mbr, strategy);

    const double geographic_max_latitude_degrees = FromRadians(MaxY(geographic_mbr));
    std::cout << "\n[5] kPolygon: (0,45)-(90,45)-(90,55)-(0,55)\n"
              << "  kCartesian  MBR " << Format(cartesian_mbr) << '\n'
              << "  kGeographic MBR " << FormatDegrees(geographic_mbr) << '\n'
              << "  北边大地线把上界纬度从 55 度抬到 " << geographic_max_latitude_degrees << " 度\n";

    // 笛卡尔：环上顶点的包围盒。
    EXPECT_DOUBLE_EQ(MinX(cartesian_mbr), 0.0);
    EXPECT_DOUBLE_EQ(MinY(cartesian_mbr), 45.0);
    EXPECT_DOUBLE_EQ(MaxX(cartesian_mbr), 90.0);
    EXPECT_DOUBLE_EQ(MaxY(cartesian_mbr), 55.0);

    // 地理：经度范围不变，纬向上界被大地线的凸出抬高。
    EXPECT_DOUBLE_EQ(FromRadians(MinX(geographic_mbr)), 0.0);
    EXPECT_DOUBLE_EQ(FromRadians(MaxX(geographic_mbr)), 90.0);
    EXPECT_DOUBLE_EQ(FromRadians(MinY(geographic_mbr)), 45.0);
    EXPECT_GT(geographic_max_latitude_degrees, 55.0);
  }
}  // namespace
