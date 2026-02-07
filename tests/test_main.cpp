#include <gtest/gtest.h>
#include <sodium.h>
#include <iostream>

int main(int argc, char** argv) {
    // Initialize libsodium ONCE for all tests
    if (sodium_init() < 0) {
        std::cerr << "FATAL: libsodium initialization failed\n";
        return 1;
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
