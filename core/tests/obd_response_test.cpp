#include "capo/obd/obd_response.hpp"

#include "capo/obd/obd_mode.hpp"
#include "capo/obd/obd_result.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace {

using capo::obd::ObdErrorCode;
using capo::obd::ObdMode;
using capo::obd::ObdResponse;
using Bytes = std::vector<std::uint8_t>;

capo::obd::ObdResult<ObdResponse> parseRpm(const Bytes& raw) {
    return ObdResponse::parse(raw, ObdMode::CurrentData, 0x0C, 2);
}

TEST(ObdResponse, ExtractsOnlyTheDataBytesAfterTheHeader) {
    const auto result = parseRpm({0x41, 0x0C, 0x1A, 0xF8});
    ASSERT_TRUE(result.hasValue()) << result.error().message;
    EXPECT_EQ(result.value().mode, ObdMode::CurrentData);
    EXPECT_EQ(result.value().pid, 0x0C);
    EXPECT_EQ(result.value().data, (Bytes{0x1A, 0xF8}));
}

TEST(ObdResponse, EmptyResponseIsNoData) {
    const auto result = parseRpm({});
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().code, ObdErrorCode::NoData);
}

TEST(ObdResponse, WrongServiceIdentifierIsUnexpected) {
    for (const Bytes& raw : {Bytes{0x42, 0x0C, 0x1A, 0xF8},  // Mode 02 response
                             Bytes{0x01, 0x0C, 0x1A, 0xF8},  // Echo of the request
                             Bytes{0x0C, 0x1A, 0xF8}}) {     // Header missing
        const auto result = parseRpm(raw);
        ASSERT_FALSE(result.hasValue());
        EXPECT_EQ(result.error().code, ObdErrorCode::UnexpectedResponse);
    }
}

TEST(ObdResponse, DifferentPidIsAMismatch) {
    const auto result = parseRpm({0x41, 0x0D, 0x1A, 0xF8});
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().code, ObdErrorCode::PidMismatch);
}

TEST(ObdResponse, NegativeResponseCarriesTheNrc) {
    const auto result = parseRpm({0x7F, 0x01, 0x12});
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().code, ObdErrorCode::NegativeResponse);
    EXPECT_EQ(result.error().negativeResponseCode, std::optional<std::uint8_t>{0x12});
}

TEST(ObdResponse, NegativeResponseForAnotherServiceIsUnexpected) {
    const auto result = parseRpm({0x7F, 0x09, 0x11});
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().code, ObdErrorCode::UnexpectedResponse);
}

TEST(ObdResponse, NegativeResponseWithWrongLengthIsMalformed) {
    for (const Bytes& raw : {Bytes{0x7F}, Bytes{0x7F, 0x01}, Bytes{0x7F, 0x01, 0x12, 0x00}}) {
        const auto result = parseRpm(raw);
        ASSERT_FALSE(result.hasValue());
        EXPECT_EQ(result.error().code, ObdErrorCode::MalformedResponse);
    }
}

TEST(ObdResponse, MissingPidIsTooShort) {
    const auto result = parseRpm({0x41});
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().code, ObdErrorCode::ResponseTooShort);
}

TEST(ObdResponse, MissingDataBytesAreTooShort) {
    for (const Bytes& raw : {Bytes{0x41, 0x0C}, Bytes{0x41, 0x0C, 0x1A}}) {
        const auto result = parseRpm(raw);
        ASSERT_FALSE(result.hasValue());
        EXPECT_EQ(result.error().code, ObdErrorCode::ResponseTooShort);
    }
}

TEST(ObdResponse, TrailingBytesAreMalformed) {
    const auto result = parseRpm({0x41, 0x0C, 0x1A, 0xF8, 0x00});
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().code, ObdErrorCode::MalformedResponse);
}

TEST(ObdResponse, ErrorMessagesNameTheBytes) {
    const auto result = parseRpm({0x41, 0x0D, 0x1A, 0xF8});
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().message, "response for PID 0D, requested 0C");
}

}  // namespace
