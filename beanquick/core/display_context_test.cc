#include "display_context.h"

#include "absl/strings/str_split.h"
#include "beanquick/core/amount.h"
#include "beanquick/core/decimal.h"
#include "absl/types/variant.h"
#include "gtest/gtest.h"


namespace beanquick {
namespace {
#define D Decimal
#define A Amount

// -----------------------------------------------------------------------------
// DistributionTest
// -----------------------------------------------------------------------------
TEST(TestInt, DistributionTest) {
  Distribution<int> dist;
  EXPECT_TRUE(dist.Empty());
  dist.Update(1);
  dist.Update(1);
  dist.Update(2);
  dist.Update(2);
  dist.Update(2);
  dist.Update(3);
  dist.Update(3);
  dist.Update(4);
  EXPECT_EQ(2, dist.Mode());
  EXPECT_EQ(1, dist.Min());
  EXPECT_EQ(4, dist.Max());
  EXPECT_TRUE(!dist.Empty());
}

// -----------------------------------------------------------------------------
// CurrencyContextTest
// -----------------------------------------------------------------------------
TEST(TestNoSign, CurrencyContextTest) {
  CurrencyContext ccontext;
  ccontext.Update(Decimal("1.23"));
  ccontext.Update(Decimal("12.234"));
  ccontext.Update(Decimal("123.235"));
  ccontext.Update(Decimal("1234.23456"));
  ccontext.Update(Decimal("12345.234567"));
  EXPECT_FALSE(ccontext.HasSign());
  EXPECT_EQ(ccontext.Integer(), 5);
  EXPECT_EQ(ccontext.Fractional(DisplayPrecision::MOST_COMMON), 3);
  EXPECT_EQ(ccontext.Fractional(DisplayPrecision::MAXIMUM), 6);

  ::testing::internal::CaptureStdout();
  ccontext.DebugPrint();
  string output = testing::internal::GetCapturedStdout();
  EXPECT_EQ(
      output,
      "sign=0 integer_max=5 frac_common=3 frac_max=6 00000.000 00000.000000");
}

TEST(TestHasSign, CurrencyContextTest) {
  CurrencyContext ccontext;
  ccontext.Update(Decimal("1.23"));
  ccontext.Update(Decimal("12.234"));
  ccontext.Update(Decimal("123.235"));
  ccontext.Update(Decimal("1234.23456"));
  ccontext.Update(Decimal("-12345.234567"));
  EXPECT_TRUE(ccontext.HasSign());
  EXPECT_EQ(ccontext.Integer(), 5);
  EXPECT_EQ(ccontext.Fractional(DisplayPrecision::MOST_COMMON), 3);
  EXPECT_EQ(ccontext.Fractional(DisplayPrecision::MAXIMUM), 6);

  ::testing::internal::CaptureStdout();
  ccontext.DebugPrint();
  string output = testing::internal::GetCapturedStdout();
  EXPECT_EQ(
      output,
      "sign=1 integer_max=5 frac_common=3 frac_max=6 -00000.000 -00000.000000");
}

//
// -----------------------------------------------------------------------------
// DisplayContextTest
// -----------------------------------------------------------------------------
typedef absl::variant<Amount, string> Variant;

std::vector<Amount> decimalize(const std::vector<Variant>& number) {
  std::vector<Amount> ret;
  for (auto& v : number) {
    if (auto ptr = absl::get_if<Amount>(&v)) {
      ret.push_back(*ptr);
    }
    else if (auto ptr = absl::get_if<string>(&v)) {
      ret.push_back(Amount(*ptr, DisplayContext::kDefulatCurrency));
    }
  }
  return ret;
}

using ::testing::ElementsAreArray;

class DisplayContextTest : public ::testing::Test {
 protected:
  void AssertFormatNumbers(const std::vector<Variant>& number_strings,
                           const std::vector<string>& expected_fmt_number) {
    DisplayContext dcontext;
    std::vector<Amount> amounts = decimalize(number_strings);
    for (auto& amount : amounts) {
      if (!config_.noinit) {
        dcontext.Update(amount);
      }
    }
    auto dformatter = dcontext.Build(config_);
    std::vector<string> fmt;
    for (auto& amount : amounts) {
      fmt.push_back(dformatter.Format(amount.Number()));
    }
    EXPECT_THAT(fmt, ElementsAreArray(expected_fmt_number));
  }

  DisplayConfig config_;
  DisplayAlignment alignment_ = DisplayAlignment::NONE;
};

// clang-format off

TEST_F(DisplayContextTest, TestNaturalUninitialized) {
  alignment_ = DisplayAlignment::NATURAL;
  AssertFormatNumbers({"1.2345", "764", "-7409.01", "0.00000125"},
                      {"1.2345", "764", "-7409.01", "0.00000125"});
}

// TEST_F(TestNaturalNoClearMode, DisplayContextTest) {
//   AssertFormatNumbers(
//       {"1.2345", "764", "-7409.01", "0.00000125"},
//       {"1.23450000", "764.00000000", "-7409.01000000", "0.00000125"});
// }

// TEST_F(TestNaturalClearMode, DisplayContextTest) {
//   AssertFormatNumbers({"1.2345", "1.23", "234.26", "38.019"},
//                       {"1.23", "1.23", "234.26", "38.02"});
// }

// TEST_F(TestNaturalMaximum, DisplayContextTest) {
//   config_.precision = DisplayPrecision::MAXIMUM;
//   AssertFormatNumbers({"1.2345", "1.23", "234.26", "38.019"},
//                       {"1.2345", "1.2300", "234.2600", "38.0190"});
// }

// TEST_F(TestNaturalCommas, DisplayContextTest) {
//   AssertFormatNumbers({"1.2345", "1.23", "234.26", "38.019"},
//                       {"1.2345", "1.2300", "234.2600", "38.0190"});
// }

// TEST_F(TestNaturalReserved, DisplayContextTest) {
//   config_.reserved = 10;
//   AssertFormatNumbers({"1.2345", "1.23", "234.26", "38.019"},
//                       {"1.23", "1.23", "234.26", "38.02"});
// }

// // class DisplayContextAlignRightTest : public  DisplayContextTest {
// //  protected:
// //   this->alignment_ = DisplayAlignment::Right;
// // };

// // TEST_F(TestRightUnlitialized, DisplayContextAlignRightTest) {
// //   config_.noinit = true;
// //   AssertFormatNumbers(
// //       {"1.2345", "764", "-7409.01", "0.00000125"},
// //       {"1.2345",
// //        "764",
// //        "-7409.01",
// //        "0.00000125"});
// // }

// // TEST_F(TestRightSign, DisplayContextAlignRightTest) {
// //   AssertFormatNumbers(
// //       {"7409.01", "0.1"},
// //       {"7409.01",
// //        "   0.10"});

// //   AssertFormatNumbers(
// //       {"-7409.01", "0.1"},
// //       {"-7409.01",
// //        "    0.10"});
// // }

// // TEST_F(TestRightInteger, DisplayContextAlignRightTest) {
// //   AssertFormatNumbers(
// //       {"1", "20", "300", "4000", "50000"},
// //       {"    1",
// //        "   20",
// //        "  300",
// //        " 4000",
// //        "50000"});

// //   config_.precision = DisplayPrecision::MAXIMUM;
// //   AssertFormatNumbers(
// //       {"1", "20", "300", "4000", "50000", "0.001"},
// //       {"    1.000",
// //        "   20.000",
// //        "  300.000",
// //        " 4000.000",
// //        "50000.000",
// //        "    0.001",});
// // }

// // TEST_F(TestRightInteger, DisplayContextAlignRightTest) {
// //   config_.commas = true;
// //   alignment_ = DisplayAlignment::Right;
// //   AssertFormatNumbers(
// //       {"1", "20", "300", "4000", "50000"},
// //       {"     1",
// //        "    20",
// //        "   300",
// //        " 4,000",
// //        "50,000"});
// // }

// // TEST_F(TestRightFractional, DisplayContextAlignRightTest) {
// //   alignment_ = DisplayAlignment::Right;
// //   AssertFormatNumbers(
// //       {"4000", "0.01", "0.02", "0.0002"},
// //       {"4000.00",
// //        "   0.01",
// //        "   0.02",
// //        "   0.00"});
// // }

// // TEST_F(TestRightFractionalCommas, DisplayContextAlignRightTest) {
// //   AssertFormatNumbers(
// //       {"4000", "0.01", "0.02", "0.0002"},
// //       {"4,000.00",
// //        "    0.01",
// //        "    0.02",
// //        "    0.00"});
// // }

// // TEST_F(TestRightDifferentCurrenies, DisplayContextAlignRightTest) {
// //   // config.precision = DisplayPrecision::MAXIMUM;
// //   AssertFormatNumbers(
// //       {A("0.12", "USD"), A("1.98", "USD"), A("0.013", "RMB"), A("0.4567",
// // "RMB")},
// //       {"0.013",
// //        "    0.01",
// //        "    0.02",
// //        "    0.00"});
// // }

// //
// // -----------------------------------------------------------------------------
// // DisplayContextAlignDotTest
// //
// // -----------------------------------------------------------------------------

// // class DisplayContextAlignDotTest : public  DisplayContextTest {
// //  protected:
// //   this->alignment_ = DisplayAlignment::Dot;
// // };

// // TEST_F(TestDotUninitialized, DisplayContextAlignDotTest) {
// //   config_.noinit = true;
// //   AssertFormatNumbers(
// //       {"1.2345", "764", "-7409.01", "0.00000125"},
// //       {"1.23450000",
// //        "764.00000000",
// //        "-7409.01000000",
// //        "0.00000125"});
// // }

// // TEST_F(TestDotBasic, DisplayContextAlignDotTest) {
// //   AssertFormatNumbers(
// //       {"1.2345", "764", "-7409.01", "0.00", "0.00000125"},
// //       {"    1.23", "  764.00", "-7409.01", "    0.00", "    0.00"});
// // }

// // TEST_F(TestDotBasicMulti, DisplayContextAlignDotTest) {
// //   AssertFormatNumbers(
// //       {A("1.2345", "USD"),
// //        A("764", "CAD"),
// //        A("-7409.01", "EUR"),
// //        A("0.00", "XAU"),
// //        A("0.00000125", "RBFF")},
// //       {"    1.2345    ",
// //        "  764         ",
// //        "-7409.01      ",
// //        "    0.00      ",
// //        "    0.00000125"});
// // };

// // TEST_F(TestDotSign, DisplayContextAlignDotTest) {
// //   AssertFormatNumbers(
// //       {A("7409.01", "USD"), "0.1"},
// //       {"7409.01",
// //        "   0.1 "});

// //   AssertFormatNumbers(
// //       {A("-7409.01", "USD"), "0.1"},
// //       {"-7409.01",
// //        "    0.1 "});
// // };

// // TEST_F(TestDotInteger, DisplayContextAlignDotTest) {
// //   AssertFormatNumbers(
// //       {"1", "20", "300", "4000", "50000"},
// //       {"    1",
// //        "   20",
// //        "  300",
// //        " 4000",
// //        "50000"});

// //   this->config_.precision = DisplayPrecision::MAXIMUM;
// //   AssertFormatNumbers(
// //       {"1", "20", "300", "4000", "50000", "0.001", A("0.1", "USD")},
// //       {"    1.000",
// //        "   20.000",
// //        "  300.000",
// //        " 4000.000",
// //        "50000.000",
// //        "    0.001",
// //        "    0.1  ",});
// // };

// // TEST_F(TestDotIntegerCommas, DisplayContextAlignDotTest) {
// //   // this->config_.commas = true;
// //   AssertFormatNumbers(
// //       {"1", "20", "300", "4000", "50000"},
// //       {"     1",
// //        "    20",
// //        "   300",
// //        " 4,000",
// //        "50,000"});
// // };

// // TEST_F(TestDotFractional, DisplayContextAlignDotTest) {
// //     AssertFormatNumbers(
// //         {A("4000", "USD"), "0.01", "0.02", "0.0002"},
// //         {"4000   ",
// //          "   0.01",
// //          "   0.02",
// //          "   0.00"});
// // };

// // TEST_F(TestDotFractionalCommas, DisplayContextAlignDotTest) {
// //   AssertFormatNumbers(
// //       {A("4000", "USD"), "0.01", "0.02", "0.0002"},
// //       {"4,000   ",
// //        "    0.01",
// //        "    0.02",
// //        "    0.00"});
// // };

// // clang-format on

// // TEST(TestQuantizeBasic, DistributionTest) {
// //   DisplayContext dcontext;
// //   dcontext.update(Decimal("1.23"), "USD");
// //   EXPECT_EQ(Decimal("3.23"), dcontext.quantize(Decimal("3.23253343"),
// //   "USD"));

// //   dcontext.update(Decimal("1.2301"), "USD");
// //   dcontext.update(Decimal("1.2302"), "USD");
// //   EXPECT_EQ(Decimal("3.2325"), dcontext.quantize(Decimal("3.23253343"),
// //   "USD"));
// // };

#undef D
#undef A
} //
}  // namespace beanquick
