#include <gtest/gtest.h>
#include "bastionx/crypto/SecureMemory.h"
#include <sodium.h>
#include <algorithm>

using namespace bastionx::crypto;

/**
 * @brief Test fixture for SecureMemory tests
 */
class SecureMemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // sodium_init() is called once in test_main.cpp
    }
};

// ===================================================================
// Test 1: Allocation and deallocation
// ===================================================================
TEST_F(SecureMemoryTest, AllocationAndDeallocation) {
    {
        SecureKey key(32);
        EXPECT_NE(nullptr, key.data());
        EXPECT_EQ(32, key.size());
        EXPECT_FALSE(key.empty());

        // Fill with test data
        std::fill_n(key.data(), key.size(), 0xAA);

        // Verify data was written
        EXPECT_EQ(0xAA, key[0]);
        EXPECT_EQ(0xAA, key[31]);
    }
    // key destructor called here - memory should be zeroed and freed
    // (No way to verify from user code, but this tests basic lifecycle)
}

// ===================================================================
// Test 2: Move semantics
// ===================================================================
TEST_F(SecureMemoryTest, MoveSemantics) {
    SecureKey key1(32);
    unsigned char* original_ptr = key1.data();

    // Fill with test data
    std::fill_n(key1.data(), key1.size(), 0xBB);

    // Move construct
    SecureKey key2 = std::move(key1);

    // key2 should now own the data
    EXPECT_EQ(original_ptr, key2.data());
    EXPECT_EQ(32, key2.size());
    EXPECT_EQ(0xBB, key2[0]);

    // key1 should be in valid but empty state
    EXPECT_EQ(nullptr, key1.data());
    EXPECT_EQ(0, key1.size());
    EXPECT_TRUE(key1.empty());
}

// ===================================================================
// Test 3: Move assignment
// ===================================================================
TEST_F(SecureMemoryTest, MoveAssignment) {
    SecureKey key1(32);
    SecureKey key2(16);

    // Fill with different test data
    std::fill_n(key1.data(), key1.size(), 0xCC);
    std::fill_n(key2.data(), key2.size(), 0xDD);

    unsigned char* original_ptr1 = key1.data();

    // Move assign
    key2 = std::move(key1);

    // key2 should now own key1's data
    EXPECT_EQ(original_ptr1, key2.data());
    EXPECT_EQ(32, key2.size());
    EXPECT_EQ(0xCC, key2[0]);

    // key1 should be in valid but empty state
    EXPECT_EQ(nullptr, key1.data());
    EXPECT_EQ(0, key1.size());
    EXPECT_TRUE(key1.empty());
}

// ===================================================================
// Test 4: Zero-size buffer
// ===================================================================
TEST_F(SecureMemoryTest, ZeroSizeBuffer) {
    SecureKey key(0);

    EXPECT_EQ(nullptr, key.data());
    EXPECT_EQ(0, key.size());
    EXPECT_TRUE(key.empty());

    // Should not crash when destroyed
}

// ===================================================================
// Test 5: Span access
// ===================================================================
TEST_F(SecureMemoryTest, SpanAccess) {
    SecureKey key(16);

    // Fill with test data
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i);
    }

    // Get span
    auto span = key.span();

    EXPECT_EQ(16, span.size());
    EXPECT_EQ(key.data(), span.data());

    // Verify data via span
    for (size_t i = 0; i < span.size(); ++i) {
        EXPECT_EQ(i, span[i]);
    }
}

// ===================================================================
// Test 6: Const span access
// ===================================================================
TEST_F(SecureMemoryTest, ConstSpanAccess) {
    SecureKey key(16);

    // Fill with test data
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i);
    }

    // Get const span
    const SecureKey& const_key = key;
    auto span = const_key.span();

    EXPECT_EQ(16, span.size());

    // Verify data via const span
    for (size_t i = 0; i < span.size(); ++i) {
        EXPECT_EQ(i, span[i]);
    }
}

// ===================================================================
// Test 7: Empty buffer span
// ===================================================================
TEST_F(SecureMemoryTest, EmptyBufferSpan) {
    SecureKey key(0);

    auto span = key.span();

    EXPECT_TRUE(span.empty());
    EXPECT_EQ(0, span.size());
}

// ===================================================================
// Test 8: Array subscript operator
// ===================================================================
TEST_F(SecureMemoryTest, ArraySubscriptOperator) {
    SecureKey key(10);

    // Write via subscript
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i * 2);
    }

    // Read via subscript
    for (size_t i = 0; i < key.size(); ++i) {
        EXPECT_EQ(i * 2, key[i]);
    }
}

// ===================================================================
// Test 9: Const array subscript operator
// ===================================================================
TEST_F(SecureMemoryTest, ConstArraySubscriptOperator) {
    SecureKey key(10);

    // Write data
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i * 3);
    }

    // Read via const reference
    const SecureKey& const_key = key;
    for (size_t i = 0; i < const_key.size(); ++i) {
        EXPECT_EQ(i * 3, const_key[i]);
    }
}

// ===================================================================
// Test 10: Multiple buffers independence
// ===================================================================
TEST_F(SecureMemoryTest, MultipleBuffersIndependence) {
    SecureKey key1(16);
    SecureKey key2(16);
    SecureKey key3(16);

    // Fill with different patterns
    std::fill_n(key1.data(), key1.size(), 0x11);
    std::fill_n(key2.data(), key2.size(), 0x22);
    std::fill_n(key3.data(), key3.size(), 0x33);

    // Verify independence
    EXPECT_EQ(0x11, key1[0]);
    EXPECT_EQ(0x22, key2[0]);
    EXPECT_EQ(0x33, key3[0]);

    // Different memory addresses
    EXPECT_NE(key1.data(), key2.data());
    EXPECT_NE(key2.data(), key3.data());
    EXPECT_NE(key1.data(), key3.data());
}

// ===================================================================
// Test 11: Self-move assignment (edge case)
// ===================================================================
TEST_F(SecureMemoryTest, SelfMoveAssignment) {
    SecureKey key(32);
    std::fill_n(key.data(), key.size(), 0xEE);

    unsigned char* original_ptr = key.data();

    // Self-move assignment
    key = std::move(key);

    // Should still be valid (implementation-defined, but should not crash)
    // After self-move, object is in valid but unspecified state
    // We just verify no crash occurs
    SUCCEED();
}

// ===================================================================
// Test 12: Large buffer allocation
// ===================================================================
TEST_F(SecureMemoryTest, LargeBufferAllocation) {
    // Allocate 1 MB
    SecureKey key(1024 * 1024);

    EXPECT_NE(nullptr, key.data());
    EXPECT_EQ(1024 * 1024, key.size());

    // Fill and verify (spot check)
    key[0] = 0xAA;
    key[1024] = 0xBB;
    key[1024 * 1024 - 1] = 0xCC;

    EXPECT_EQ(0xAA, key[0]);
    EXPECT_EQ(0xBB, key[1024]);
    EXPECT_EQ(0xCC, key[1024 * 1024 - 1]);
}
