#include "capo/version.hpp"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <span>
#include <string_view>

namespace {

constexpr std::string_view kUsage =
    "Usage: mdn-scan [--version | --help]\n"
    "\n"
    "MDN Scan - offline automotive diagnostics.\n";

}  // namespace

int main(int argc, char* argv[]) {
    const std::span<char*> args(argv, static_cast<std::size_t>(argc));

    if (args.size() == 2) {
        const std::string_view arg = args[1];
        if (arg == "--version" || arg == "-V") {
            std::cout << "mdn-scan " << capo::version_string() << '\n';
            return EXIT_SUCCESS;
        }
        if (arg == "--help" || arg == "-h") {
            std::cout << kUsage;
            return EXIT_SUCCESS;
        }
    }

    std::cerr << kUsage;
    return 2;
}
