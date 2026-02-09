#include "bastionx/vault/VaultSettings.h"
#include <nlohmann/json.hpp>
#include <algorithm>

namespace bastionx {
namespace vault {

std::string VaultSettings::to_json() const {
    nlohmann::json j;
    j["auto_lock_minutes"] = auto_lock_minutes;
    j["clipboard_clear_enabled"] = clipboard_clear_enabled;
    j["clipboard_clear_seconds"] = clipboard_clear_seconds;
    return j.dump();
}

VaultSettings VaultSettings::from_json(const std::string& json_str) {
    VaultSettings s = defaults();
    try {
        auto j = nlohmann::json::parse(json_str);

        if (j.contains("auto_lock_minutes") && j["auto_lock_minutes"].is_number_integer()) {
            s.auto_lock_minutes = std::clamp(j["auto_lock_minutes"].get<int>(), 1, 60);
        }
        if (j.contains("clipboard_clear_enabled") && j["clipboard_clear_enabled"].is_boolean()) {
            s.clipboard_clear_enabled = j["clipboard_clear_enabled"].get<bool>();
        }
        if (j.contains("clipboard_clear_seconds") && j["clipboard_clear_seconds"].is_number_integer()) {
            s.clipboard_clear_seconds = std::clamp(j["clipboard_clear_seconds"].get<int>(), 10, 120);
        }
    } catch (...) {
        return defaults();
    }
    return s;
}

VaultSettings VaultSettings::defaults() {
    return VaultSettings{5, true, 30};
}

bool VaultSettings::operator==(const VaultSettings& other) const {
    return auto_lock_minutes == other.auto_lock_minutes &&
           clipboard_clear_enabled == other.clipboard_clear_enabled &&
           clipboard_clear_seconds == other.clipboard_clear_seconds;
}

}  // namespace vault
}  // namespace bastionx
