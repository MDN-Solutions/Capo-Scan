#pragma once

#include "capo/obd/obd_result.hpp"
#include "capo/obd/obd_transport.hpp"

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace capo::obd::test {

/// Transport test double: returns a scripted result and records every request.
class FakeObdTransport final : public ObdTransport {
public:
    using Bytes = std::vector<std::uint8_t>;

    explicit FakeObdTransport(ObdResult<Bytes> reply) : reply_(std::move(reply)) {}

    static FakeObdTransport replying(Bytes response) {
        return FakeObdTransport(ObdResult<Bytes>::success(std::move(response)));
    }

    static FakeObdTransport failing(ObdErrorCode code) {
        return FakeObdTransport(ObdResult<Bytes>::failure(code, "fake transport error"));
    }

    ObdResult<Bytes> send(std::span<const std::uint8_t> request) override {
        requests_.emplace_back(request.begin(), request.end());
        return reply_;
    }

    [[nodiscard]] const std::vector<Bytes>& requests() const { return requests_; }

private:
    ObdResult<Bytes> reply_;
    std::vector<Bytes> requests_;
};

}  // namespace capo::obd::test
