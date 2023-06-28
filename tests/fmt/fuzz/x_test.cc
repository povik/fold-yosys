#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"

#include <sstream>
#include "backends/cxxrtl/cxxrtl.h"
#include "libs/bigint/BigUnsigned.hh"
#include "libs/bigint/BigInteger.hh"

using namespace cxxrtl_yosys;

void Formats128BitUnsignedIntegers(chunk_t x0, chunk_t x1, chunk_t x2, chunk_t x3)
{
  // Compare output to BigUnsigned.
  value<128> v;
  v = v.blit<127, 64>(value<64>{x1, x0});
  v = v.blit<63, 0>(value<64>{x3, x2});

  std::ostringstream oss;
  oss << value_formatted<128>(v, false, false, ' ', 0, 10, false, false, false);
  auto actual = oss.str();

  BigUnsigned u;
  u.bitShiftLeft(v.slice<127, 64>().val().get<uint64_t>(), 64);
  u.bitOr(u, v.slice<63, 0>().val().get<uint64_t>());

  std::string expected;

  if (u.isZero()) {
    expected = "0";
  } else {
    while (!u.isZero()) {
      expected += '0' + (u % 10).toInt();
      u /= 10;
    }
    std::reverse(expected.begin(), expected.end());
  }

  EXPECT_EQ(actual, expected);
}
FUZZ_TEST(CxxrtlDivisionFuzz, Formats128BitUnsignedIntegers);


void Formats128BitSignedIntegers(chunk_t x0, chunk_t x1, chunk_t x2, chunk_t x3) {
  // Compare output to BigInteger.
  value<128> v;
  v = v.blit<127, 64>(value<64>{x1, x0});
  v = v.blit<63, 0>(value<64>{x3, x2});

  std::ostringstream oss;
  oss << value_formatted<128>(v, false, false, ' ', 0, 10, true, false, false);
  auto actual = oss.str();

  BigUnsigned u;
  bool negative = v.is_neg();
  if (negative)
    v = v.neg();
  u.bitShiftLeft(v.slice<127, 64>().val().get<uint64_t>(), 64);
  u.bitOr(u, v.slice<63, 0>().val().get<uint64_t>());

  std::string expected;
  if (u.isZero()) {
    expected = "0";
  } else {
    // Note that we never actually do division of negative numbers: our division
    // routines are flooring, not truncating, so dividing by 10 repeatedly won't
    // necessarily ever get to zero.
    while (!u.isZero()) {
      expected += '0' + (u % 10).toInt();
      u /= 10;
    }
    if (negative) {
      expected += '-';
    }
    std::reverse(expected.begin(), expected.end());
  }

  EXPECT_EQ(actual, expected);
}
FUZZ_TEST(CxxrtlDivisionFuzz, Formats128BitSignedIntegers);
