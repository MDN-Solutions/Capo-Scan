#include "capo/obd/obd_service.hpp"

#include "capo/obd/obd_pid.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

namespace capo::obd {

namespace {

template <typename T>
ObdResult<T> forwardError(ObdResult<ObdResponse>&& response) {
    return ObdResult<T>::failure(std::move(response).error());
}

}  // namespace

ObdService::ObdService(ObdTransport& transport) noexcept : transport_(&transport) {}

ObdResult<double> ObdService::getEngineRpm() {
    constexpr Mode01Pid kPid = Mode01Pid::EngineRpm;
    auto response = request(ObdMode::CurrentData, toByte(kPid), dataLength(kPid));
    if (!response) {
        return forwardError<double>(std::move(response));
    }
    // parse() guarantees exactly dataLength(kPid) == 2 data bytes.
    const auto& data = response.value().data;
    return ObdResult<double>::success(formula::engineRpm(data[0], data[1]));
}

ObdResult<int> ObdService::getVehicleSpeed() {
    constexpr Mode01Pid kPid = Mode01Pid::VehicleSpeed;
    auto response = request(ObdMode::CurrentData, toByte(kPid), dataLength(kPid));
    if (!response) {
        return forwardError<int>(std::move(response));
    }
    const auto& data = response.value().data;  // Exactly 1 byte.
    return ObdResult<int>::success(formula::vehicleSpeedKmh(data[0]));
}

ObdResult<double> ObdService::getCoolantTemperature() {
    constexpr Mode01Pid kPid = Mode01Pid::CoolantTemperature;
    auto response = request(ObdMode::CurrentData, toByte(kPid), dataLength(kPid));
    if (!response) {
        return forwardError<double>(std::move(response));
    }
    const auto& data = response.value().data;  // Exactly 1 byte.
    return ObdResult<double>::success(
        static_cast<double>(formula::coolantTemperatureCelsius(data[0])));
}

ObdResult<ObdResponse> ObdService::request(ObdMode mode, std::uint8_t pid, std::size_t dataLength) {
    const std::array<std::uint8_t, 2> message{toByte(mode), pid};
    auto raw = transport_->send(message);
    if (!raw) {
        return ObdResult<ObdResponse>::failure(std::move(raw).error());
    }
    return ObdResponse::parse(raw.value(), mode, pid, dataLength);
}

}  // namespace capo::obd
