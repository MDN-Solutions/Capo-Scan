#include "capo/version.hpp"

namespace capo {

Version version() noexcept {
    return Version{
        .major = CAPO_VERSION_MAJOR, .minor = CAPO_VERSION_MINOR, .patch = CAPO_VERSION_PATCH};
}

std::string_view version_string() noexcept { return CAPO_VERSION_STRING; }

}  // namespace capo
