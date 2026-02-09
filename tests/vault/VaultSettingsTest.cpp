#include <gtest/gtest.h>
#include "bastionx/vault/VaultSettings.h"

using bastionx::vault::VaultSettings;

TEST(VaultSettingsTest, DefaultsAreCorrect) {
    auto s = VaultSettings::defaults();
    EXPECT_EQ(s.auto_lock_minutes, 5);
    EXPECT_TRUE(s.clipboard_clear_enabled);
    EXPECT_EQ(s.clipboard_clear_seconds, 30);
}

TEST(VaultSettingsTest, RoundTrip) {
    VaultSettings original;
    original.auto_lock_minutes = 10;
    original.clipboard_clear_enabled = false;
    original.clipboard_clear_seconds = 60;

    std::string json = original.to_json();
    VaultSettings restored = VaultSettings::from_json(json);

    EXPECT_EQ(original, restored);
}

TEST(VaultSettingsTest, RoundTripDefaults) {
    VaultSettings original = VaultSettings::defaults();
    std::string json = original.to_json();
    VaultSettings restored = VaultSettings::from_json(json);
    EXPECT_EQ(original, restored);
}

TEST(VaultSettingsTest, ClampsOutOfRangeValues) {
    // auto_lock_minutes below min
    auto s = VaultSettings::from_json(R"({"auto_lock_minutes":0})");
    EXPECT_EQ(s.auto_lock_minutes, 1);

    // auto_lock_minutes above max
    s = VaultSettings::from_json(R"({"auto_lock_minutes":999})");
    EXPECT_EQ(s.auto_lock_minutes, 60);

    // clipboard_clear_seconds below min
    s = VaultSettings::from_json(R"({"clipboard_clear_seconds":1})");
    EXPECT_EQ(s.clipboard_clear_seconds, 10);

    // clipboard_clear_seconds above max
    s = VaultSettings::from_json(R"({"clipboard_clear_seconds":500})");
    EXPECT_EQ(s.clipboard_clear_seconds, 120);
}

TEST(VaultSettingsTest, InvalidJsonReturnsDefaults) {
    auto s = VaultSettings::from_json("not json at all");
    EXPECT_EQ(s, VaultSettings::defaults());
}

TEST(VaultSettingsTest, EmptyJsonReturnsDefaults) {
    auto s = VaultSettings::from_json("{}");
    EXPECT_EQ(s, VaultSettings::defaults());
}

TEST(VaultSettingsTest, PartialJsonPreservesDefaults) {
    auto s = VaultSettings::from_json(R"({"auto_lock_minutes":15})");
    EXPECT_EQ(s.auto_lock_minutes, 15);
    EXPECT_TRUE(s.clipboard_clear_enabled);   // default
    EXPECT_EQ(s.clipboard_clear_seconds, 30); // default
}

TEST(VaultSettingsTest, WrongTypesIgnored) {
    auto s = VaultSettings::from_json(R"({"auto_lock_minutes":"ten","clipboard_clear_enabled":42})");
    EXPECT_EQ(s, VaultSettings::defaults());
}
