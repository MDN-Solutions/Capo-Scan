// Fuzzes ObdResponse::parse and Dtc decoding with arbitrary bytes.
//
// Input layout: <PID> <expected data length> <raw response...>

#include "capo/obd/obd_dtc.hpp"
#include "capo/obd/obd_mode.hpp"
#include "capo/obd/obd_response.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <span>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::span<const std::uint8_t> input(data, size);
    if (input.size() < 2) {
        return 0;
    }
    const std::uint8_t pid = input[0];
    const std::size_t dataLength = input[1] % 8U;
    const std::span<const std::uint8_t> raw = input.subspan(2);

    const auto result =
        capo::obd::ObdResponse::parse(raw, capo::obd::ObdMode::CurrentData, pid, dataLength);
    if (result.hasValue()) {
        // A success must have exactly the requested data and the header removed.
        if (result.value().pid != pid || result.value().data.size() != dataLength ||
            raw.size() != dataLength + 2) {
            std::abort();
        }
    } else if (result.error().message.empty()) {
        std::abort();
    }

    if (raw.size() >= 2) {
        const capo::obd::Dtc code(raw[0], raw[1]);
        if (code.toString().size() != 5) {
            std::abort();
        }
    }
    return 0;
}
