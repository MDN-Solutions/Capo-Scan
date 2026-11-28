#include "capo/obd/obd_response.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace capo::obd {

namespace {

constexpr std::size_t kHeaderLength = 2;            // <service id> <PID>
constexpr std::size_t kNegativeResponseLength = 3;  // 7F <service id> <NRC>

std::string hex(std::uint8_t byte) {
    constexpr std::string_view kDigits = "0123456789ABCDEF";
    return {kDigits[(byte >> 4U) & 0xFU], kDigits[byte & 0xFU]};
}

ObdResult<ObdResponse> fail(ObdErrorCode code, std::string message) {
    return ObdResult<ObdResponse>::failure(code, std::move(message));
}

ObdResult<ObdResponse> parseNegative(std::span<const std::uint8_t> raw, ObdMode mode) {
    if (raw.size() != kNegativeResponseLength) {
        return fail(ObdErrorCode::MalformedResponse, "negative response has " +
                                                         std::to_string(raw.size()) +
                                                         " bytes, expected 3 (7F <service> <NRC>)");
    }
    if (raw[1] != toByte(mode)) {
        return fail(
            ObdErrorCode::UnexpectedResponse,
            "negative response for service " + hex(raw[1]) + ", requested " + hex(toByte(mode)));
    }
    const std::uint8_t nrc = raw[2];
    return ObdResult<ObdResponse>::failure(ObdError{
        .code = ObdErrorCode::NegativeResponse,
        .message = "ECU rejected service " + hex(toByte(mode)) + " with NRC " + hex(nrc),
        .negativeResponseCode = nrc,
    });
}

}  // namespace

ObdResult<ObdResponse> ObdResponse::parse(std::span<const std::uint8_t> raw, ObdMode mode,
                                          std::uint8_t pid, std::size_t dataLength) {
    if (raw.empty()) {
        return fail(ObdErrorCode::NoData, "empty response");
    }
    if (raw[0] == kNegativeResponseId) {
        return parseNegative(raw, mode);
    }
    if (raw[0] != positiveResponseId(mode)) {
        return fail(
            ObdErrorCode::UnexpectedResponse,
            "service identifier " + hex(raw[0]) + ", expected " + hex(positiveResponseId(mode)));
    }
    if (raw.size() < kHeaderLength) {
        return fail(ObdErrorCode::ResponseTooShort, "response has no PID");
    }
    if (raw[1] != pid) {
        return fail(ObdErrorCode::PidMismatch,
                    "response for PID " + hex(raw[1]) + ", requested " + hex(pid));
    }

    const std::span<const std::uint8_t> data = raw.subspan(kHeaderLength);
    if (data.size() < dataLength) {
        return fail(ObdErrorCode::ResponseTooShort,
                    "PID " + hex(pid) + " has " + std::to_string(data.size()) +
                        " data bytes, expected " + std::to_string(dataLength));
    }
    if (data.size() > dataLength) {
        return fail(ObdErrorCode::MalformedResponse,
                    "PID " + hex(pid) + " has " + std::to_string(data.size()) +
                        " data bytes, expected " + std::to_string(dataLength));
    }

    return ObdResult<ObdResponse>::success(ObdResponse{
        .mode = mode,
        .pid = pid,
        .data = std::vector<std::uint8_t>(data.begin(), data.end()),
    });
}

}  // namespace capo::obd
