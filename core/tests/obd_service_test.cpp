#include "capo/obd/obd_service.hpp"

#include "capo/obd/obd_result.hpp"
#include "fake_obd_transport.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace {

using capo::obd::ObdErrorCode;
using capo::obd::ObdService;
using capo::obd::test::FakeObdTransport;
using Bytes = std::vector<std::uint8_t>;

// --- Valid responses ---------------------------------------------------------

TEST(ObdService, EngineRpmUsesTheBytesAfterTheHeader) {
    // 41 0C 1A F8: A = 0x1A, B = 0xF8 -> (26 * 256 + 248) / 4 = 1726 rpm.
    auto transport = FakeObdTransport::replying({0x41, 0x0C, 0x1A, 0xF8});
    ObdService service(transport);

    const auto rpm = service.getEngineRpm();
    ASSERT_TRUE(rpm.hasValue()) << rpm.error().message;
    EXPECT_DOUBLE_EQ(rpm.value(), 1726.0);
    EXPECT_EQ(transport.requests(), (std::vector<Bytes>{{0x01, 0x0C}}));
}

TEST(ObdService, EngineRpmKeepsQuarterRpmResolutionAndFullRange) {
    auto quarter = FakeObdTransport::replying({0x41, 0x0C, 0x00, 0x01});
    EXPECT_DOUBLE_EQ(ObdService(quarter).getEngineRpm().value(), 0.25);

    auto maximum = FakeObdTransport::replying({0x41, 0x0C, 0xFF, 0xFF});
    EXPECT_DOUBLE_EQ(ObdService(maximum).getEngineRpm().value(), 16383.75);
}

TEST(ObdService, VehicleSpeedIsTheDataByteInKmh) {
    auto transport = FakeObdTransport::replying({0x41, 0x0D, 0x3C});
    ObdService service(transport);

    const auto speed = service.getVehicleSpeed();
    ASSERT_TRUE(speed.hasValue()) << speed.error().message;
    EXPECT_EQ(speed.value(), 60);
    EXPECT_EQ(transport.requests(), (std::vector<Bytes>{{0x01, 0x0D}}));

    auto maximum = FakeObdTransport::replying({0x41, 0x0D, 0xFF});
    EXPECT_EQ(ObdService(maximum).getVehicleSpeed().value(), 255);
}

TEST(ObdService, CoolantTemperatureIsTheDataByteMinus40) {
    auto transport = FakeObdTransport::replying({0x41, 0x05, 0x7B});
    ObdService service(transport);

    const auto temperature = service.getCoolantTemperature();
    ASSERT_TRUE(temperature.hasValue()) << temperature.error().message;
    EXPECT_DOUBLE_EQ(temperature.value(), 83.0);  // 0x7B = 123
    EXPECT_EQ(transport.requests(), (std::vector<Bytes>{{0x01, 0x05}}));

    auto minimum = FakeObdTransport::replying({0x41, 0x05, 0x00});
    EXPECT_DOUBLE_EQ(ObdService(minimum).getCoolantTemperature().value(), -40.0);
    auto maximum = FakeObdTransport::replying({0x41, 0x05, 0xFF});
    EXPECT_DOUBLE_EQ(ObdService(maximum).getCoolantTemperature().value(), 215.0);
}

// --- Invalid responses are errors, never values ------------------------------

TEST(ObdService, InvalidHeaderIsAnError) {
    auto transport = FakeObdTransport::replying({0x42, 0x0C, 0x1A, 0xF8});
    const auto rpm = ObdService(transport).getEngineRpm();
    ASSERT_FALSE(rpm.hasValue());
    EXPECT_EQ(rpm.error().code, ObdErrorCode::UnexpectedResponse);
}

TEST(ObdService, ResponseForAnotherPidIsAnError) {
    // A valid speed response must not be decoded as RPM.
    auto transport = FakeObdTransport::replying({0x41, 0x0D, 0x3C});
    const auto rpm = ObdService(transport).getEngineRpm();
    ASSERT_FALSE(rpm.hasValue());
    EXPECT_EQ(rpm.error().code, ObdErrorCode::PidMismatch);
}

TEST(ObdService, NegativeResponseIsReportedWithItsNrc) {
    auto transport = FakeObdTransport::replying({0x7F, 0x01, 0x31});
    const auto speed = ObdService(transport).getVehicleSpeed();
    ASSERT_FALSE(speed.hasValue());
    EXPECT_EQ(speed.error().code, ObdErrorCode::NegativeResponse);
    EXPECT_EQ(speed.error().negativeResponseCode, std::optional<std::uint8_t>{0x31});
}

TEST(ObdService, InsufficientDataIsAnError) {
    auto transport = FakeObdTransport::replying({0x41, 0x0C, 0x1A});
    const auto rpm = ObdService(transport).getEngineRpm();
    ASSERT_FALSE(rpm.hasValue());
    EXPECT_EQ(rpm.error().code, ObdErrorCode::ResponseTooShort);
}

TEST(ObdService, MalformedResponseIsAnError) {
    auto transport = FakeObdTransport::replying({0x41, 0x05, 0x7B, 0x00});
    const auto temperature = ObdService(transport).getCoolantTemperature();
    ASSERT_FALSE(temperature.hasValue());
    EXPECT_EQ(temperature.error().code, ObdErrorCode::MalformedResponse);
}

TEST(ObdService, EmptyResponseIsNoData) {
    auto transport = FakeObdTransport::replying({});
    const auto speed = ObdService(transport).getVehicleSpeed();
    ASSERT_FALSE(speed.hasValue());
    EXPECT_EQ(speed.error().code, ObdErrorCode::NoData);
}

// --- Transport errors are passed through unchanged ---------------------------

TEST(ObdService, TransportErrorsArePassedThrough) {
    for (const ObdErrorCode code :
         {ObdErrorCode::TransportFailure, ObdErrorCode::Timeout, ObdErrorCode::NoData}) {
        auto transport = FakeObdTransport::failing(code);
        ObdService service(transport);

        const auto rpm = service.getEngineRpm();
        ASSERT_FALSE(rpm.hasValue());
        EXPECT_EQ(rpm.error().code, code);
        EXPECT_EQ(rpm.error().message, "fake transport error");

        EXPECT_EQ(service.getVehicleSpeed().error().code, code);
        EXPECT_EQ(service.getCoolantTemperature().error().code, code);
    }
}

}  // namespace
