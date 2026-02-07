#ifndef BASTIONX_CRYPTO_SECUREMEMORY_H
#define BASTIONX_CRYPTO_SECUREMEMORY_H

#include <sodium.h>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>

namespace bastionx {
namespace crypto {

/**
 * @brief RAII wrapper for libsodium secure memory allocation
 *
 * SecureBuffer provides automatic memory management for sensitive data:
 * - Allocates memory using sodium_malloc() (locked, with guard pages)
 * - Automatically zeros memory on destruction using sodium_memzero()
 * - Non-copyable to prevent accidental key duplication
 * - Movable for efficient transfer of ownership
 *
 * @tparam T The type of data to store (typically unsigned char)
 */
template<typename T>
class SecureBuffer {
public:
    /**
     * @brief Allocate secure memory for count elements
     * @param count Number of elements to allocate
     * @throws std::runtime_error if allocation fails
     */
    explicit SecureBuffer(size_t count)
        : data_(nullptr), size_(count) {

        if (count == 0) {
            // Allow zero-size buffers (empty state)
            return;
        }

        // Allocate using sodium_malloc (locked memory with guard pages)
        data_ = static_cast<T*>(sodium_malloc(count * sizeof(T)));

        if (data_ == nullptr) {
            throw std::runtime_error("Failed to allocate secure memory");
        }
    }

    /**
     * @brief Destructor - zeros and frees memory
     */
    ~SecureBuffer() {
        if (data_ != nullptr) {
            // Zero memory before freeing (critical for key material)
            sodium_memzero(data_, size_ * sizeof(T));
            sodium_free(data_);
            data_ = nullptr;
            size_ = 0;
        }
    }

    // Delete copy constructor and assignment (prevent key duplication)
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

    /**
     * @brief Move constructor - transfers ownership
     */
    SecureBuffer(SecureBuffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    /**
     * @brief Move assignment - transfers ownership
     */
    SecureBuffer& operator=(SecureBuffer&& other) noexcept {
        if (this != &other) {
            // Clean up existing data
            if (data_ != nullptr) {
                sodium_memzero(data_, size_ * sizeof(T));
                sodium_free(data_);
            }

            // Transfer ownership
            data_ = other.data_;
            size_ = other.size_;

            // Leave other in valid but empty state
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    /**
     * @brief Get raw pointer to data
     * @return Pointer to the first element, or nullptr if empty
     */
    T* data() noexcept {
        return data_;
    }

    /**
     * @brief Get const raw pointer to data
     * @return Const pointer to the first element, or nullptr if empty
     */
    const T* data() const noexcept {
        return data_;
    }

    /**
     * @brief Get the number of elements
     * @return Number of elements in the buffer
     */
    size_t size() const noexcept {
        return size_;
    }

    /**
     * @brief Check if buffer is empty
     * @return true if size is 0 or data is null
     */
    bool empty() const noexcept {
        return size_ == 0 || data_ == nullptr;
    }

    /**
     * @brief Get a span view of the data (mutable)
     * @return std::span<T> view of the buffer
     */
    std::span<T> span() noexcept {
        if (data_ == nullptr) {
            return std::span<T>();
        }
        return std::span<T>(data_, size_);
    }

    /**
     * @brief Get a span view of the data (const)
     * @return std::span<const T> view of the buffer
     */
    std::span<const T> span() const noexcept {
        if (data_ == nullptr) {
            return std::span<const T>();
        }
        return std::span<const T>(data_, size_);
    }

    /**
     * @brief Array subscript operator
     * @param index Index of the element
     * @return Reference to the element at index
     * @warning No bounds checking - use with caution
     */
    T& operator[](size_t index) noexcept {
        return data_[index];
    }

    /**
     * @brief Const array subscript operator
     * @param index Index of the element
     * @return Const reference to the element at index
     * @warning No bounds checking - use with caution
     */
    const T& operator[](size_t index) const noexcept {
        return data_[index];
    }

private:
    T* data_;       ///< Pointer to secure memory
    size_t size_;   ///< Number of elements
};

/**
 * @brief Type alias for cryptographic key material
 *
 * SecureKey is the primary type used throughout the application
 * for storing sensitive cryptographic keys in memory.
 */
using SecureKey = SecureBuffer<unsigned char>;

}  // namespace crypto
}  // namespace bastionx

#endif  // BASTIONX_CRYPTO_SECUREMEMORY_H
