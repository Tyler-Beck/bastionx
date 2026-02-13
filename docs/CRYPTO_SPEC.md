# Bastionx Cryptographic Specification

This document provides a detailed specification of all cryptographic operations in Bastionx.

## Table of Contents

1. [Design Philosophy](#design-philosophy)
2. [Cryptographic Library](#cryptographic-library)
3. [Key Derivation](#key-derivation)
4. [Subkey Derivation](#subkey-derivation)
5. [Encryption](#encryption)
6. [Decryption](#decryption)
7. [Memory Management](#memory-management)
8. [Associated Data Format](#associated-data-format)
9. [Security Guarantees](#security-guarantees)
10. [Test Vectors](#test-vectors)

---

## Design Philosophy

Bastionx follows these cryptographic principles:

1. **No Custom Cryptography**: All cryptographic operations use libsodium exclusively
2. **Modern Algorithms**: Only post-2010 algorithms with active security research
3. **Fail-Safe Defaults**: Use recommended parameters from libsodium documentation
4. **Defense in Depth**: Multiple layers of protection (AAD, key separation, etc.)
5. **Explicit Over Implicit**: All security decisions are documented and testable

---

## Cryptographic Library

### libsodium

**Version**: 1.0.18 or later (via vcpkg)

**Why libsodium?**
- Audited, widely-used cryptographic library
- Safe defaults that prevent common mistakes
- Active maintenance and security updates
- Cross-platform support
- Excellent documentation

**Alternatives Considered and Rejected**:
- OpenSSL: Too complex, easy to misuse
- Botan: Less widely audited
- Crypto++: Verbose API, more error-prone

---

## Key Derivation

### Algorithm: Argon2id

**Function**: `crypto_pwhash()`

**Purpose**: Derive a master key from a user password

### Parameters

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Algorithm | `crypto_pwhash_ALG_ARGON2ID13` | Hybrid mode resistant to both side-channel and GPU attacks |
| OpsLimit | `crypto_pwhash_OPSLIMIT_MODERATE` | Balances security vs usability (~100-500ms) |
| MemLimit | `crypto_pwhash_MEMLIMIT_MODERATE` | Balances security vs usability (~256 MB RAM) |
| Salt Size | 16 bytes | Minimum recommended by libsodium |
| Output Size | 32 bytes | Compatible with KDF (crypto_kdf_KEYBYTES) |

### Concrete Values (libsodium 1.0.18+)

- **OpsLimit (MODERATE)**: 3 operations
- **MemLimit (MODERATE)**: 268,435,456 bytes (~256 MB)

### Why Argon2id?

Argon2id combines the benefits of:
- **Argon2d**: Data-dependent memory access (resists GPU attacks)
- **Argon2i**: Data-independent memory access (resists side-channel attacks)

### Why MODERATE Settings?

- **INTERACTIVE** (~100ms): Too weak for offline attacks
- **MODERATE** (~500ms): Good balance for local-first app
- **SENSITIVE** (~3-5s): Too slow for user experience

### Implementation

```cpp
int crypto_pwhash(
    unsigned char* out,           // Output: 32-byte master key
    unsigned long long outlen,    // 32 (crypto_kdf_KEYBYTES)
    const char* passwd,           // Input: user password (UTF-8)
    unsigned long long passwdlen, // Password length
    const unsigned char* salt,    // 16-byte random salt
    unsigned long long opslimit,  // crypto_pwhash_OPSLIMIT_MODERATE
    size_t memlimit,              // crypto_pwhash_MEMLIMIT_MODERATE
    int alg                       // crypto_pwhash_ALG_ARGON2ID13
);
```

### Salt Generation

Salts are generated using libsodium's CSPRNG:

```cpp
randombytes_buf(salt, 16);
```

**Source**: Uses the best available system entropy:
- Windows: `BCryptGenRandom()` (CryptGenRandom in older versions)

### Key Derivation Flow

```
User Password (UTF-8 string)
    + Random Salt (16 bytes)
    ↓
[ Argon2id ]
    - OpsLimit: MODERATE
    - MemLimit: MODERATE
    - Time: ~100-500ms
    ↓
Master Key (32 bytes, in secure memory)
```

---

## Subkey Derivation

### Algorithm: KDF (Key Derivation Function)

**Function**: `crypto_kdf_derive_from_key()`

**Purpose**: Derive context-specific subkeys from the master key

### Why Separate Subkeys?

- **Separation of Concerns**: Notes use different keys than settings
- **Cryptographic Hygiene**: Prevents cross-use of key material
- **Future-Proof**: Easy to add new contexts without changing master key

### Parameters

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Subkey Size | 16 bytes | Minimum size for XChaCha20-Poly1305 |
| Context String | "BastionX" (8 bytes) | Application identifier (must be exactly 8 bytes) |
| Context ID | 1, 2, 3, ... | Numeric identifier for each use case |

### Defined Contexts

| Context ID | Purpose | Phase |
|------------|---------|-------|
| 1 | Note encryption/decryption | Phase 1 |
| 2 | Settings encryption | Phase 2 |
| 3-999 | Reserved for future use | - |

### Implementation

```cpp
int crypto_kdf_derive_from_key(
    unsigned char* subkey,        // Output: 16-byte subkey
    size_t subkey_len,            // 16 (crypto_kdf_BYTES_MIN)
    uint64_t subkey_id,           // Context ID (1, 2, 3, ...)
    const char ctx[8],            // "BastionX" (exactly 8 bytes)
    const unsigned char* key      // Input: 32-byte master key
);
```

### Subkey Derivation Flow

```
Master Key (32 bytes)
    + Context String ("BastionX")
    + Context ID (1 = notes, 2 = settings)
    ↓
[ crypto_kdf ]
    ↓
Subkey (16 bytes, in secure memory)
```

---

## Encryption

### Algorithm: XChaCha20-Poly1305 (AEAD)

**Function**: `crypto_aead_xchacha20poly1305_ietf_encrypt()`

**Purpose**: Encrypt plaintext with authenticated additional data

### Why XChaCha20-Poly1305?

- **XChaCha20**: Extended nonce variant of ChaCha20 (allows random nonces)
- **Poly1305**: MAC providing authentication (prevents tampering)
- **AEAD**: Authenticated Encryption with Associated Data

### Parameters

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Nonce Size | 24 bytes | Allows random nonces (no counter management) |
| Key Size | 16+ bytes | We use 16 bytes from KDF |
| MAC Size | 16 bytes | Poly1305 authentication tag |

### Nonce Generation

Nonces are generated randomly for each encryption:

```cpp
randombytes_buf(nonce, 24);
```

**Why Random Nonces?**
- XChaCha20 has 192-bit nonces (24 bytes)
- Collision probability is negligible (< 2^-96 for billions of encryptions)
- No need to maintain counters or state

### Associated Data (AAD)

AAD is authenticated but not encrypted. Used to bind ciphertext to context.

For notes, AAD format is:
```
[ note_id (4 bytes) ][ updated_at (8 bytes) ]
```

**Why AAD?**
- Prevents ciphertext swapping between notes
- Binds ciphertext to specific note ID and timestamp
- Detects if attacker moves ciphertext to different record

### Implementation

```cpp
int crypto_aead_xchacha20poly1305_ietf_encrypt(
    unsigned char* c,             // Output: ciphertext + tag
    unsigned long long* clen_p,   // Output: ciphertext length
    const unsigned char* m,       // Input: plaintext
    unsigned long long mlen,      // Plaintext length
    const unsigned char* ad,      // Additional authenticated data (AAD)
    unsigned long long adlen,     // AAD length
    const unsigned char* nsec,    // NULL (not used)
    const unsigned char* npub,    // 24-byte nonce
    const unsigned char* k        // 16-byte (or more) key
);
```

### Output Format

```
Ciphertext = Encrypted Data + MAC Tag (16 bytes)
Length = plaintext_length + 16 bytes
```

### Encryption Flow

```
Plaintext (variable size)
    + Subkey (16 bytes)
    + Random Nonce (24 bytes)
    + AAD (note_id + timestamp)
    ↓
[ XChaCha20-Poly1305 AEAD ]
    ↓
Ciphertext (plaintext_size + 16 bytes MAC)
```

---

## Decryption

### Algorithm: XChaCha20-Poly1305 (AEAD)

**Function**: `crypto_aead_xchacha20poly1305_ietf_decrypt()`

**Purpose**: Decrypt ciphertext and verify authentication

### Authentication Checks

Decryption fails if:
1. **MAC verification fails**: Wrong key, tampered ciphertext, or corrupted data
2. **AAD mismatch**: Ciphertext was encrypted with different AAD
3. **Truncated ciphertext**: Ciphertext too short (< 16 bytes for MAC)

### Implementation

```cpp
int crypto_aead_xchacha20poly1305_ietf_decrypt(
    unsigned char* m,             // Output: plaintext
    unsigned long long* mlen_p,   // Output: plaintext length
    unsigned char* nsec,          // NULL (not used)
    const unsigned char* c,       // Input: ciphertext + tag
    unsigned long long clen,      // Ciphertext length
    const unsigned char* ad,      // Additional authenticated data (must match)
    unsigned long long adlen,     // AAD length
    const unsigned char* npub,    // 24-byte nonce (from encryption)
    const unsigned char* k        // 16-byte key (must match encryption key)
);
```

### Return Values

- **0**: Decryption succeeded, plaintext is valid
- **-1**: Authentication failed (wrong key, tampered data, or AAD mismatch)

### Decryption Flow

```
Ciphertext (plaintext_size + 16 bytes MAC)
    + Subkey (16 bytes)
    + Nonce (24 bytes, from storage)
    + AAD (note_id + timestamp, from storage)
    ↓
[ XChaCha20-Poly1305 AEAD Decrypt ]
    ↓
MAC Verification
    ↓
Success: Plaintext (variable size)
Failure: std::nullopt (authentication failed)
```

---

## Memory Management

### Secure Memory Allocation

**Functions**:
- `sodium_malloc()`: Allocate secure memory
- `sodium_free()`: Free secure memory
- `sodium_memzero()`: Zero memory before freeing

### Properties of Secure Memory

1. **Memory Locking**: Prevents swapping to disk (on supported platforms)
2. **Guard Pages**: Detects buffer overflows
3. **Canaries**: Detects buffer overflows
4. **Zeroing on Free**: Prevents key material from persisting in memory

### RAII Wrapper

We use `SecureBuffer<T>` template class:

```cpp
template<typename T>
class SecureBuffer {
    T* data_;
    size_t size_;

    // Allocates with sodium_malloc()
    explicit SecureBuffer(size_t count);

    // Zeros with sodium_memzero() and frees with sodium_free()
    ~SecureBuffer();

    // Non-copyable, movable
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer(SecureBuffer&&) noexcept;
};
```

### Key Lifecycle

```
1. User enters password
    ↓
2. Derive master key → SecureBuffer<uint8_t> (sodium_malloc)
    ↓
3. Derive subkeys → SecureBuffer<uint8_t> (sodium_malloc)
    ↓
4. Use for encryption/decryption
    ↓
5. On lock/exit → SecureBuffer destroyed
    ↓
6. sodium_memzero() → All key bytes set to 0x00
    ↓
7. sodium_free() → Memory returned to OS
```

### Memory Safety Guarantees

- **No Swapping**: Keys never written to pagefile (if mlock succeeds)
- **No Core Dumps**: Memory is non-dumpable
- **No Leaks**: RAII ensures memory is freed
- **No Persistence**: Keys wiped on destruction

---

## Associated Data Format

### Note Encryption AAD

```
Byte Offset | Size | Field
------------|------|----------
0-3         | 4    | note_id (uint32_t, little-endian)
4-11        | 8    | updated_at (uint64_t, little-endian, UNIX timestamp)
```

**Total Size**: 12 bytes

### Why Include Timestamp?

- Binds ciphertext to specific version of note
- Prevents rollback attacks (replacing new version with old ciphertext)
- Detects if attacker swaps ciphertexts between note updates

### AAD Construction Example

```cpp
std::vector<uint8_t> aad(12);

// note_id = 42 (0x0000002A)
aad[0] = 0x2A;
aad[1] = 0x00;
aad[2] = 0x00;
aad[3] = 0x00;

// updated_at = 1707091200 (0x00000000659D8800)
aad[4] = 0x00;
aad[5] = 0x88;
aad[6] = 0x9D;
aad[7] = 0x65;
aad[8] = 0x00;
aad[9] = 0x00;
aad[10] = 0x00;
aad[11] = 0x00;
```

---

## Security Guarantees

### What Bastionx Cryptography Provides

- **Confidentiality**: Plaintext cannot be recovered without the correct password
- **Integrity**: Tampering with ciphertext is detected (MAC verification fails)
- **Authenticity**: Ciphertext cannot be forged without the key
- **Binding**: Ciphertext is bound to specific note ID and timestamp (via AAD)
- **Key Separation**: Notes and settings use different subkeys
- **Forward Secrecy**: Old ciphertexts remain secure even if current key is compromised (password change)

### What Bastionx Cryptography Does NOT Provide

- **Protection Against Weak Passwords**: User must choose strong password
- **Protection While Unlocked**: Keys are in memory and vulnerable to memory dumps
- **Protection Against OS Compromise**: Keyloggers, malware can steal password
- **Protection Against Physical Access**: Cold boot attacks, hardware keyloggers
- **Password Recovery**: If password is forgotten, data is permanently lost

---

## Test Vectors

### Key Derivation Test Vector

**Input**:
- Password: `"correct horse battery staple"`
- Salt: `0x0123456789ABCDEFFEDCBA9876543210` (16 bytes)

**Expected Output** (32 bytes, hex):
```
<To be computed with reference implementation>
```

**Notes**:
- Use Argon2id with MODERATE settings
- Verify against libsodium test suite
- Deterministic: Same password + salt = same key

### Encryption Test Vector

**Input**:
- Plaintext: `"Hello, Bastionx!"`
- Key: `0x808182838485868788898A8B8C8D8E8F` (16 bytes)
- Nonce: `0x070000004041424344454647484900000000000000000000` (24 bytes)
- AAD: `0x01000000` (note_id=1, 4 bytes)

**Expected Output** (ciphertext + MAC, hex):
```
<To be computed with reference implementation>
```

**Notes**:
- Use XChaCha20-Poly1305
- Output length = 16 (plaintext) + 16 (MAC) = 32 bytes
- Deterministic with fixed nonce (for testing only!)

---

## SQLCipher Database Encryption (Phase 5)

**Implementation**: Bastionx uses SQLCipher for database-level encryption, providing defense-in-depth beyond application-layer note encryption.

**Key Derivation**:
```cpp
// Database key derived from vault master key using crypto_kdf
const std::string context = "sqlcipher-key";
std::vector<uint8_t> db_key = crypto.derive_key(vault_key, context);
```

**SQLCipher Configuration**:
```cpp
// Set encryption key
std::string key_hex = "x'" + bytes_to_hex(db_key) + "'";
PRAGMA key = key_hex;

// PRAGMA settings for security
PRAGMA cipher_page_size = 4096;
PRAGMA kdf_iter = 256000;  // PBKDF2 iterations
PRAGMA cipher_hmac_algorithm = HMAC_SHA512;
PRAGMA cipher_kdf_algorithm = PBKDF2_HMAC_SHA512;
```

**Password Change Process**:
1. Derive new database key from new master password
2. Execute `PRAGMA rekey` to re-encrypt database
3. Re-encrypt all note content with new note subkey
4. Re-encrypt vault settings
5. Atomic commit - all or nothing

**Security Properties**:
- Database files are encrypted at rest
- PRAGMA key never written to disk
- Re-keying is atomic (PRAGMA rekey uses temporary database)
- Keys wiped from memory after use via sodium_memzero

---

## References

1. **libsodium Documentation**: https://libsodium.gitbook.io/
2. **Argon2 RFC**: https://datatracker.ietf.org/doc/html/rfc9106
3. **XChaCha20-Poly1305**: https://datatracker.ietf.org/doc/html/draft-irtf-cfrg-xchacha
4. **ChaCha20-Poly1305 RFC**: https://datatracker.ietf.org/doc/html/rfc8439

---

## Changelog

- **2026-02-13**: Added SQLCipher section (Phase 5-7 completion)
- **2026-02-05**: Initial specification for Phase 0 & 1

---

**Last Updated**: 2026-02-13
**Bastionx Version**: 0.7.0 (Phase 8 Complete)
**Document Version**: 1.1
