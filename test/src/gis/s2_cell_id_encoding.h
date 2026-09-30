#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

#include "s2/s2cell_id.h"

namespace s2_test {
  struct CellIdEncoding {
    int face;
    int level;
    int marker_bit;  // Bit positions count from zero at the least significant bit.
    uint64_t path;
    uint64_t marker;
  };

  // Decode the documented 64-bit layout directly from a valid library CellID:
  // [face:3][Hilbert child path:2*level][marker:1][zeros:60-2*level].
  inline auto DecodeCellId(S2CellId cell) -> CellIdEncoding {
    assert(cell.is_valid());
    const uint64_t id = cell.id();
    int marker_bit = 0;
    for (uint64_t bits = id; (bits & 1) == 0; bits >>= 1) {
      ++marker_bit;
    }
    const uint64_t face_position = id & ((uint64_t{ 1 } << 61) - 1);
    return { static_cast<int>(id >> 61),
             (60 - marker_bit) / 2,
             marker_bit,
             face_position >> (marker_bit + 1),
             uint64_t{ 1 } << marker_bit };
  }

  inline auto FormatCellIdEncoding(S2CellId cell) -> std::string {
    const auto decoded = DecodeCellId(cell);
    const std::string bits = std::bitset<64>(cell.id()).to_string();
    std::string pairs;
    for (int level = 0; level < decoded.level; ++level) {
      if (!pairs.empty()) {
        pairs += ' ';
      }
      pairs += bits.substr(3 + 2 * level, 2);
    }
    std::ostringstream out;
    out << "    hex16=0x" << std::hex << std::setw(16) << std::setfill('0') << cell.id() << std::dec << '\n'
        << "    bits64=" << bits << '\n'
        << "    fields(face/path/marker/zeros)=[" << bits.substr(0, 3) << "][" << pairs << "][1]["
        << std::string(decoded.marker_bit, '0') << "]  (3 + " << 2 * decoded.level << " + 1 + " << decoded.marker_bit
        << " = 64)\n"
        << "    decode: face=" << decoded.face << ", level=(60-" << decoded.marker_bit << ")/2=" << decoded.level
        << ", lsb=2^" << decoded.marker_bit << '=' << decoded.marker << '\n';
    return out.str();
  }
}  // namespace s2_test
