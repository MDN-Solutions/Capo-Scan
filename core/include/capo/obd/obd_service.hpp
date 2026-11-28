#pragma once

#include "capo/obd/obd_mode.hpp"
#include "capo/obd/obd_response.hpp"
#include "capo/obd/obd_result.hpp"
#include "capo/obd/obd_transport.hpp"

#include <cstddef>
#include <cstdint>

namespace capo::obd {

/// High-level OBD-II operations on top of an ObdTransport.
///
/// Every response is validated (service, PID, length, negative responses)
/// before it is decoded; failures are reported as ObdError, never as a
/// sentinel value. The transport is borrowed: it must outlive the service.
/// Not thread-safe: use one service per transport from one thread at a time.
class ObdService {
public:
    explicit ObdService(ObdTransport& transport) noexcept;

    /// Mode 01 PID 0C, in rpm.
    [[nodiscard]] ObdResult<double> getEngineRpm();

    /// Mode 01 PID 0D, in km/h.
    [[nodiscard]] ObdResult<int> getVehicleSpeed();

    /// Mode 01 PID 05, in degrees Celsius.
    [[nodiscard]] ObdResult<double> getCoolantTemperature();

private:
    /// Sends "<mode> <pid>" and returns the validated response.
    [[nodiscard]] ObdResult<ObdResponse> request(ObdMode mode, std::uint8_t pid,
                                                 std::size_t dataLength);

    ObdTransport* transport_;  // Never null (set from a reference).
};

}  // namespace capo::obd
