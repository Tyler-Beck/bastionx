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
