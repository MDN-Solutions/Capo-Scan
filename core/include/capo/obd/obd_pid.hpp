#pragma once

#include <cstddef>
#include <cstdint>

namespace capo::obd {

/// Mode 01 PIDs implemented by capo_core (SAE J1979). Only these are supported.
enum class Mode01Pid : std::uint8_t {
    /// Engine coolant temperature: 1 byte, A - 40 [degrees C].
    CoolantTemperature = 0x05,
    /// Engine speed: 2 bytes, (256 * A + B) / 4 [rpm].
    EngineRpm = 0x0C,
    /// Vehicle speed: 1 byte, A [km/h].
    VehicleSpeed = 0x0D,
};

[[nodiscard]] constexpr std::uint8_t toByte(Mode01Pid pid) noexcept {
    return static_cast<std::uint8_t>(pid);
}

/// Number of data bytes (A, B, ...) after the "41 <PID>" header of a response.
[[nodiscard]] constexpr std::size_t dataLength(Mode01Pid pid) noexcept {
    switch (pid) {
        case Mode01Pid::CoolantTemperature:
        case Mode01Pid::VehicleSpeed:
            return 1;
        case Mode01Pid::EngineRpm:
            return 2;
    }
    return 0;  // Not reachable with the enumerators above.
}

/// Decoding formulas. They take the data bytes only: A is the first byte
/// AFTER the "41 <PID>" header, never the header itself.
namespace formula {

/// PID 0x0C: (256 * A + B) / 4, from 0 to 16383.75 rpm.
[[nodiscard]] constexpr double engineRpm(std::uint8_t a, std::uint8_t b) noexcept {
    const unsigned raw = (static_cast<unsigned>(a) * 256U) + static_cast<unsigned>(b);
    return static_cast<double>(raw) / 4.0;
}

/// PID 0x0D: A, from 0 to 255 km/h.
[[nodiscard]] constexpr int vehicleSpeedKmh(std::uint8_t a) noexcept { return static_cast<int>(a); }

/// PID 0x05: A - 40, from -40 to 215 degrees C.
[[nodiscard]] constexpr int coolantTemperatureCelsius(std::uint8_t a) noexcept {
    return static_cast<int>(a) - 40;
}

}  // namespace formula

}  // namespace capo::obd
