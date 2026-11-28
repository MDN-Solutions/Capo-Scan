#pragma once

#include <cstdint>
#include <string>

namespace capo::obd {

/// System a diagnostic trouble code belongs to (first letter of the code).
enum class DtcCategory : std::uint8_t {
    Powertrain,  ///< P
    Chassis,     ///< C
    Body,        ///< B
    Network,     ///< U
};

/// A standard OBD-II diagnostic trouble code (SAE J2012 / ISO 15031-6), as
/// encoded in two bytes by Modes 03, 07 and 0A:
///
///   first byte  bits 7-6: category (00 P, 01 C, 10 B, 11 U)
///               bits 5-4: first digit (0-3)
///               bits 3-0: second digit (hex)
///   second byte bits 7-4 and 3-0: third and fourth digits (hex)
///
/// Example: 01 33 -> "P0133". Every 16-bit value decodes to a well-formed code.
/// 00 00 is the padding sent where there is no code, not a real DTC.
class Dtc {
public:
    constexpr Dtc(std::uint8_t high, std::uint8_t low) noexcept
        : raw_(static_cast<std::uint16_t>((static_cast<unsigned>(high) << 8U) | low)) {}

    [[nodiscard]] constexpr DtcCategory category() const noexcept {
        return static_cast<DtcCategory>((raw_ >> 14U) & 0x3U);
    }

    /// The two encoded bytes as one big-endian value.
    [[nodiscard]] constexpr std::uint16_t raw() const noexcept { return raw_; }

    /// True for 00 00, the padding used where there is no code.
    [[nodiscard]] constexpr bool isPadding() const noexcept { return raw_ == 0; }

    /// The code as written in manuals, for example "P0133" or "U0100".
    [[nodiscard]] std::string toString() const;

    friend constexpr bool operator==(const Dtc&, const Dtc&) noexcept = default;

private:
    std::uint16_t raw_;
};

}  // namespace capo::obd
