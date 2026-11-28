#pragma once

#include "capo/obd/obd_mode.hpp"
#include "capo/obd/obd_result.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace capo::obd {

/// A validated positive response to a single-PID request.
struct ObdResponse {
    ObdMode mode;
    std::uint8_t pid;
    /// Data bytes A, B, ... only: the service identifier and PID are removed.
    /// Its size is exactly the `dataLength` passed to parse().
    std::vector<std::uint8_t> data;

    /// Validates `raw` as the response to the request "<mode> <pid>" and
    /// extracts its data bytes. Checks, in order:
    ///
    ///   - empty response                          -> NoData
    ///   - 7F <mode> <NRC>                         -> NegativeResponse (with the NRC)
    ///   - 7F with another service or wrong length -> UnexpectedResponse / MalformedResponse
    ///   - first byte is not <mode> + 0x40         -> UnexpectedResponse
    ///   - PID missing or fewer data bytes         -> ResponseTooShort
    ///   - PID different from `pid`                -> PidMismatch
    ///   - more than `dataLength` data bytes       -> MalformedResponse
    [[nodiscard]] static ObdResult<ObdResponse> parse(std::span<const std::uint8_t> raw,
                                                      ObdMode mode, std::uint8_t pid,
                                                      std::size_t dataLength);
};

}  // namespace capo::obd
