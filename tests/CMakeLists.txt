find_package(GTest CONFIG REQUIRED)

# Test executable
add_executable(bastionx_tests
    test_main.cpp
    crypto/CryptoServiceTest.cpp
    crypto/SecureMemoryTest.cpp
    vault/VaultServiceTest.cpp
    vault/VaultSettingsTest.cpp
    vault/PasswordChangeTest.cpp
    vault/SQLCipherTest.cpp
    storage/NotesRepositoryTest.cpp
    storage/SearchTest.cpp
    integration/IntegrationTest.cpp
)

target_link_libraries(bastionx_tests PRIVATE
    bastionx_core
    GTest::gtest
    GTest::gtest_main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(bastionx_tests)

# UI tests (require Qt6::Widgets for QTextDocument Markdown support)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(bastionx_ui_tests
    ui/ui_test_main.cpp
    ui/MarkdownRoundTripTest.cpp
)

target_link_libraries(bastionx_ui_tests PRIVATE
    GTest::gtest
    Qt6::Widgets
)

# Use gtest_add_tests (source parsing) instead of gtest_discover_tests (exe run)
# because the Qt platform plugin isn't available at build-time discovery
gtest_add_tests(TARGET bastionx_ui_tests)
