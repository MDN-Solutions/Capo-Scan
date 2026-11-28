#include "capo/obd/obd_mode.hpp"
#include "capo/obd/obd_pid.hpp"
#include "capo/obd/obd_result.hpp"

#include <gtest/gtest.h>

#include <variant>

namespace {

using namespace capo::obd;

// The formulas and constants are constexpr: checked at compile time.
static_assert(formula::engineRpm(0x1A, 0xF8) == 1726.0);
static_assert(formula::engineRpm(0x00, 0x00) == 0.0);
static_assert(formula::vehicleSpeedKmh(0x3C) == 60);
static_assert(formula::coolantTemperatureCelsius(0x00) == -40);
static_assert(formula::coolantTemperatureCelsius(0xFF) == 215);
static_assert(dataLength(Mode01Pid::EngineRpm) == 2);
static_assert(dataLength(Mode01Pid::VehicleSpeed) == 1);
static_assert(dataLength(Mode01Pid::CoolantTemperature) == 1);
static_assert(toByte(Mode01Pid::EngineRpm) == 0x0C);
static_assert(toByte(Mode01Pid::VehicleSpeed) == 0x0D);
static_assert(toByte(Mode01Pid::CoolantTemperature) == 0x05);
static_assert(positiveResponseId(ObdMode::CurrentData) == 0x41);

TEST(ObdResult, HoldsAValueOrAnError) {
    const auto ok = ObdResult<int>::success(42);
    ASSERT_TRUE(ok.hasValue());
    EXPECT_TRUE(static_cast<bool>(ok));
    EXPECT_EQ(ok.value(), 42);

    const auto bad = ObdResult<int>::failure(ObdErrorCode::Timeout, "no answer");
    ASSERT_FALSE(bad.hasValue());
    EXPECT_EQ(bad.error().code, ObdErrorCode::Timeout);
    EXPECT_EQ(bad.error().message, "no answer");
    EXPECT_FALSE(bad.error().negativeResponseCode.has_value());
}

TEST(ObdResult, WrongAccessorThrowsInsteadOfReturningGarbage) {
    const auto bad = ObdResult<int>::failure(ObdErrorCode::NoData, "");
    EXPECT_THROW((void)bad.value(), std::bad_variant_access);
    const auto ok = ObdResult<int>::success(1);
    EXPECT_THROW((void)ok.error(), std::bad_variant_access);
}

TEST(ObdResult, ErrorCodesHaveNames) {
    EXPECT_EQ(toString(ObdErrorCode::NegativeResponse), "negative response");
    EXPECT_EQ(toString(ObdErrorCode::PidMismatch), "PID mismatch");
}

}  // namespace
