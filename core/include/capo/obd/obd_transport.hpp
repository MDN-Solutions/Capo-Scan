#pragma once

#include "capo/obd/obd_result.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace capo::obd {

/// Hardware-independent link to the vehicle (ELM327, J2534, a test double, ...).
///
/// A transport moves OBD-II messages and nothing else. It sends one request
/// (service identifier + parameters, for example 01 0C) and returns the response
/// of one ECU as raw bytes (for example 41 0C 1A F8). It does not interpret
/// them: addressing, CAN/ISO-TP framing, padding, checksums, adapter text
/// (hex, "SEARCHING...", prompts) and "response pending" (7F xx 78) retries are
/// handled inside the transport and never reach the caller.
///
/// Errors: TransportFailure, Timeout or NoData. Returning an empty byte vector
/// is also treated as NoData by ObdService.
class ObdTransport {
public:
    virtual ~ObdTransport() = default;

    [[nodiscard]] virtual ObdResult<std::vector<std::uint8_t>> send(
        std::span<const std::uint8_t> request) = 0;
};

}  // namespace capo::obd
