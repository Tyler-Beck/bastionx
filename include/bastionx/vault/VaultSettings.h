#ifndef BASTIONX_VAULT_VAULTSETTINGS_H
#define BASTIONX_VAULT_VAULTSETTINGS_H

#include <string>

namespace bastionx {
namespace vault {

/**
 * @brief User-configurable vault settings
 *
 * Serialized to/from JSON and stored encrypted in the vault_settings table.
 */
struct VaultSettings {
    int auto_lock_minutes = 5;           // Range: 1-60
    bool clipboard_clear_enabled = true;
    int clipboard_clear_seconds = 30;    // Range: 10-120

    /// Serialize to JSON string
    std::string to_json() const;

    /// Deserialize from JSON string; returns defaults() on parse failure
    static VaultSettings from_json(const std::string& json_str);

    /// Factory returning default settings
    static VaultSettings defaults();

    bool operator==(const VaultSettings& other) const;
};

}  // namespace vault
}  // namespace bastionx

#endif  // BASTIONX_VAULT_VAULTSETTINGS_H
