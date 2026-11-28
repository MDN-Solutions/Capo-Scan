#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace capo::obd {

/// Why an OBD-II operation failed.
enum class ObdErrorCode : std::uint8_t {
    /// The transport could not send the request or read the response
    /// (adapter disconnected, I/O error, ...).
    TransportFailure,
    /// No response arrived in time.
    Timeout,
    /// The request was sent but no ECU answered (for example ELM327 "NO DATA").
    NoData,
    /// The ECU rejected the request (0x7F); see ObdError::negativeResponseCode.
    NegativeResponse,
    /// The response belongs to another service (wrong service identifier).
    UnexpectedResponse,
    /// The response is for another PID than the one requested.
    PidMismatch,
    /// The response ends before all the expected bytes.
    ResponseTooShort,
    /// The response is structurally invalid (for example unexpected trailing bytes).
    MalformedResponse,
};

/// Stable, human-readable name of an error code (for logs; not localized).
[[nodiscard]] constexpr std::string_view toString(ObdErrorCode code) noexcept {
    switch (code) {
        case ObdErrorCode::TransportFailure:
            return "transport failure";
        case ObdErrorCode::Timeout:
            return "timeout";
        case ObdErrorCode::NoData:
            return "no data";
        case ObdErrorCode::NegativeResponse:
            return "negative response";
        case ObdErrorCode::UnexpectedResponse:
            return "unexpected response";
        case ObdErrorCode::PidMismatch:
            return "PID mismatch";
        case ObdErrorCode::ResponseTooShort:
            return "response too short";
        case ObdErrorCode::MalformedResponse:
            return "malformed response";
    }
    return "unknown error";
}

/// A failed OBD-II operation.
struct ObdError {
    ObdErrorCode code;
    /// Details for logs and diagnostics (English, not meant for the end user).
    std::string message;
    /// Negative response code (NRC) sent by the ECU; set only for
    /// ObdErrorCode::NegativeResponse.
    std::optional<std::uint8_t> negativeResponseCode;
};

/// Either a value of type T or an ObdError (a C++20 stand-in for std::expected).
///
/// value() and error() require hasValue() to be true or false respectively;
/// calling the wrong one throws std::bad_variant_access.
template <typename T>
class [[nodiscard]] ObdResult {
    static_assert(!std::is_reference_v<T>, "ObdResult cannot hold a reference");

public:
    static ObdResult success(T value) { return ObdResult(kValueIndex, std::move(value)); }

    static ObdResult failure(ObdError error) { return ObdResult(kErrorIndex, std::move(error)); }

    static ObdResult failure(ObdErrorCode code, std::string message) {
        return failure(
            ObdError{.code = code, .message = std::move(message), .negativeResponseCode = {}});
    }

    [[nodiscard]] bool hasValue() const noexcept { return data_.index() == 0; }

    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

    [[nodiscard]] const T& value() const& { return std::get<0>(data_); }

    /// Moves the value out of a temporary result.
    [[nodiscard]] T value() && { return std::get<0>(std::move(data_)); }

    [[nodiscard]] const ObdError& error() const& { return std::get<1>(data_); }

    [[nodiscard]] ObdError error() && { return std::get<1>(std::move(data_)); }

private:
    static constexpr std::in_place_index_t<0> kValueIndex{};
    static constexpr std::in_place_index_t<1> kErrorIndex{};

    template <std::size_t Index, typename U>
    ObdResult(std::in_place_index_t<Index> index, U&& content)
        : data_(index, std::forward<U>(content)) {}

    std::variant<T, ObdError> data_;
};

}  // namespace capo::obd
