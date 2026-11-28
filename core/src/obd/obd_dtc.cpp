#include "capo/obd/obd_dtc.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace capo::obd {

namespace {

constexpr std::array<char, 4> kCategoryLetters{'P', 'C', 'B', 'U'};
constexpr std::string_view kHexDigits = "0123456789ABCDEF";

/// Hex digit of the bits of `raw` selected by `shift` and `mask` (mask <= 0xF).
char digit(std::uint16_t raw, unsigned shift, unsigned mask) {
    return kHexDigits[static_cast<std::size_t>((raw >> shift) & mask)];
}

}  // namespace

std::string Dtc::toString() const {
    return {
        kCategoryLetters.at(static_cast<std::size_t>(category())),
        digit(raw_, 12U, 0x3U),  // First digit: 0-3 only.
        digit(raw_, 8U, 0xFU),
        digit(raw_, 4U, 0xFU),
        digit(raw_, 0U, 0xFU),
    };
}

}  // namespace capo::obd
