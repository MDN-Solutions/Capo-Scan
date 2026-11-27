#pragma once

#include <string_view>

namespace capo {

struct Version {
    int major;
    int minor;
    int patch;
};

/// Version of the diagnostic core (kept in sync with the release by release-please).
[[nodiscard]] Version version() noexcept;

/// Same version as "major.minor.patch".
[[nodiscard]] std::string_view version_string() noexcept;

}  // namespace capo
