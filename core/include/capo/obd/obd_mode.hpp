#pragma once

#include <cstdint>

namespace capo::obd {

/// OBD-II services ("modes", SAE J1979 / ISO 15031-5) implemented by capo_core.
/// Only the services listed here are supported.
enum class ObdMode : std::uint8_t {
    /// Mode 01: current powertrain data (live sensor values).
    CurrentData = 0x01,
};

/// A positive response carries the requested service identifier plus 0x40
/// (request 01 -> response 41).
inline constexpr std::uint8_t kPositiveResponseOffset = 0x40;

/// First byte of a negative response: 7F <requested service> <NRC>.
inline constexpr std::uint8_t kNegativeResponseId = 0x7F;

[[nodiscard]] constexpr std::uint8_t toByte(ObdMode mode) noexcept {
    return static_cast<std::uint8_t>(mode);
}

/// Service identifier of a positive response to `mode`.
[[nodiscard]] constexpr std::uint8_t positiveResponseId(ObdMode mode) noexcept {
    return static_cast<std::uint8_t>(toByte(mode) + kPositiveResponseOffset);
}

}  // namespace capo::obd
