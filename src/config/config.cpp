#include "config.hpp"
#include "koalabox/util/util.hpp"

#include <fstream>

namespace config {

    Config read(Path path) { // NOLINT(performance-unnecessary-value-param)
        if (not std::filesystem::exists(path)) {
            return {};
        }

        try {
            std::ifstream ifs(path);
            nlohmann::json json = nlohmann::json::parse(ifs, nullptr, true, true);

            return json.get<Config>();
        } catch (const std::exception& ex) {
            util::error_box(__func__, fmt::format("Failed to parse config file: {}", ex.what()));
            exit(::GetLastError()); // NOLINT(cppcoreguidelines-narrowing-conversions)
        }
    }

}
