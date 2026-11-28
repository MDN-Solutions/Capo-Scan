#include "capo/obd/obd_dtc.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace {

using capo::obd::Dtc;
using capo::obd::DtcCategory;

TEST(Dtc, DecodesTheStandardExamples) {
    EXPECT_EQ(Dtc(0x01, 0x33).toString(), "P0133");
    EXPECT_EQ(Dtc(0x03, 0x01).toString(), "P0301");
    EXPECT_EQ(Dtc(0x41, 0x23).toString(), "C0123");
    EXPECT_EQ(Dtc(0x80, 0x01).toString(), "B0001");
    EXPECT_EQ(Dtc(0xC1, 0x00).toString(), "U0100");
}

TEST(Dtc, DecodesTheCategoryFromTheTopTwoBits) {
    EXPECT_EQ(Dtc(0x00, 0x00).category(), DtcCategory::Powertrain);
    EXPECT_EQ(Dtc(0x40, 0x00).category(), DtcCategory::Chassis);
    EXPECT_EQ(Dtc(0x80, 0x00).category(), DtcCategory::Body);
    EXPECT_EQ(Dtc(0xC0, 0x00).category(), DtcCategory::Network);
}

TEST(Dtc, FirstDigitIsZeroToThreeAndTheOthersAreHex) {
    EXPECT_EQ(Dtc(0x10, 0x00).toString(), "P1000");
    EXPECT_EQ(Dtc(0x3F, 0xFF).toString(), "P3FFF");
    EXPECT_EQ(Dtc(0x0A, 0x0F).toString(), "P0A0F");
    EXPECT_EQ(Dtc(0xFF, 0xFF).toString(), "U3FFF");
}

TEST(Dtc, KeepsTheRawValueAndComparesByIt) {
    constexpr Dtc kCode(0x01, 0x33);
    static_assert(kCode.raw() == 0x0133);
    static_assert(kCode == Dtc(0x01, 0x33));
    static_assert(kCode != Dtc(0x01, 0x34));
}

TEST(Dtc, ZeroIsPaddingNotACode) {
    EXPECT_TRUE(Dtc(0x00, 0x00).isPadding());
    EXPECT_FALSE(Dtc(0x01, 0x33).isPadding());
}

TEST(Dtc, EveryEncodingProducesAFiveCharacterCode) {
    for (unsigned value = 0; value <= 0xFFFFU; ++value) {
        const Dtc code(static_cast<std::uint8_t>(value >> 8U), static_cast<std::uint8_t>(value));
        const std::string text = code.toString();
        ASSERT_EQ(text.size(), 5U) << value;
        ASSERT_NE(std::string_view("PCBU").find(text[0]), std::string_view::npos) << value;
        ASSERT_GE(text[1], '0');
        ASSERT_LE(text[1], '3');
    }
}

}  // namespace
