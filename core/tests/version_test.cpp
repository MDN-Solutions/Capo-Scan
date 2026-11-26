#include "capo/version.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

TEST(Version, StringMatchesComponents) {
    const capo::Version v = capo::version();
    const std::string expected =
        std::to_string(v.major) + "." + std::to_string(v.minor) + "." + std::to_string(v.patch);
    EXPECT_EQ(capo::version_string(), expected);
}

TEST(Version, ComponentsAreNotNegative) {
    const capo::Version v = capo::version();
    EXPECT_GE(v.major, 0);
    EXPECT_GE(v.minor, 0);
    EXPECT_GE(v.patch, 0);
}

}  // namespace
