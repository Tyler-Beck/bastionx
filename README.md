# Bastionx

A privacy-first, local-only desktop notes application for Windows.

## Project Status

**Current Phase**: Phase 0 & 1 Implementation Complete
- ✅ Project infrastructure setup
- ✅ Cryptographic core implementation
- ⏳ Dependencies installation required
- ⏳ Initial build and testing

## Overview

Bastionx is a zero-knowledge, local-first notes application that prioritizes user privacy above all else. All data is encrypted at rest using modern cryptographic primitives, and the master password never leaves your machine.

### Key Features

- **Zero-Knowledge Encryption**: Your data is encrypted with a master key derived from your password
- **Local-First**: No cloud sync, no user accounts, no telemetry
- **Modern Cryptography**: Argon2id for key derivation, XChaCha20-Poly1305 for encryption
- **Privacy by Design**: Explicit threat model and security guarantees

### Technology Stack

- **Language**: C++20
- **UI Framework**: Qt 6 (Widgets)
- **Cryptography**: libsodium (no custom crypto)
- **Storage**: SQLite with application-layer encryption
- **Platform**: Windows (v1)

## Prerequisites

Before building Bastionx, you need:

1. **Visual Studio 2022** (Community Edition or better)
   - Workload: "Desktop development with C++"
   - Ensure C++20 support is enabled
   - Include CMake tools for Windows

2. **CMake 3.21+** (included with Visual Studio or install separately)

3. **vcpkg** (C++ package manager) - see installation below

## Quick Start

### 1. Install vcpkg

vcpkg is Microsoft's C++ package manager. Install it once and use it for all C++ projects.

```powershell
# Clone vcpkg to a permanent location
git clone https://github.com/Microsoft/vcpkg.git C:\dev\vcpkg

# Navigate to vcpkg directory
cd C:\dev\vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Integrate with Visual Studio (one-time setup)
.\vcpkg integrate install
```

### 2. Install Dependencies

From the vcpkg directory, install all required libraries:

```powershell
cd C:\dev\vcpkg

# Install dependencies (this will take 30-60 minutes, especially Qt 6)
.\vcpkg install qt6-base:x64-windows
.\vcpkg install libsodium:x64-windows
.\vcpkg install sqlite3:x64-windows
.\vcpkg install gtest:x64-windows
```

**Note**: Qt 6 installation is large (~2-3 GB) and takes significant time. Be patient!

### 3. Build the Project

Navigate to the Bastionx directory and build:

```powershell
# Navigate to project root
cd C:\Users\17326\spring2026\bastionx

# Create build directory
mkdir build
cd build

# Configure with CMake (pointing to vcpkg toolchain)
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build (Debug configuration)
cmake --build . --config Debug

# Or build Release configuration
cmake --build . --config Release
```

### 4. Run Tests

After building, run the test suite to verify everything works:

```powershell
# From the build directory
cd C:\Users\17326\spring2026\bastionx\build

# Run tests (Debug)
.\Debug\bastionx_tests.exe

# Or run tests (Release)
.\Release\bastionx_tests.exe
```

## Project Structure

```
bastionx/
├── CLAUDE.md                    # Project specification
├── README.md                    # This file
├── CMakeLists.txt              # Build configuration
├── vcpkg.json                  # Dependency manifest
│
├── include/bastionx/           # Public headers
│   └── crypto/
│       ├── CryptoService.h     # Core cryptographic API
│       └── SecureMemory.h      # Secure memory RAII wrapper
│
├── src/                        # Implementation
│   └── crypto/
│       ├── CryptoService.cpp   # Crypto implementation
│       └── SecureMemory.cpp    # Memory management
│
├── tests/                      # Unit tests
│   ├── test_main.cpp
│   └── crypto/
│       ├── CryptoServiceTest.cpp
│       ├── SecureMemoryTest.cpp
│       └── test_vectors.h
│
└── docs/                       # Documentation
    ├── CRYPTO_SPEC.md          # Cryptographic specification
    ├── BUILD.md                # Detailed build guide
    └── THREAT_MODEL.md         # Security threat model
```

## Current Implementation Status

### Phase 0: Project Infrastructure ✅
- [x] Directory structure
- [x] CMake build system
- [x] vcpkg dependency management
- [x] Test framework setup

### Phase 1: Cryptographic Core ✅
- [x] SecureMemory (RAII wrapper for libsodium)
- [x] CryptoService (key derivation, encryption, decryption)
- [x] Unit tests (10 core tests + memory safety tests)
- [x] Test vectors

### Phase 2: Storage Layer ⏳
- [ ] VaultService (lock/unlock, password validation)
- [ ] NotesRepository (SQLite schema, encrypted CRUD)
- [ ] Integration tests

### Phase 3: UI MVP ⏳
- [ ] Qt Widgets UI
- [ ] Vault unlock screen
- [ ] Notes list
- [ ] Note editor

### Phase 4: Hardening ⏳
- [ ] Auto-lock timer
- [ ] Memory wiping on lock
- [ ] Security documentation

## Cryptographic Design

Bastionx uses libsodium exclusively for all cryptographic operations:

- **Key Derivation**: Argon2id (MODERATE settings)
  - Protects against offline password attacks
  - ~100-500ms derivation time (intentional)

- **Encryption**: XChaCha20-Poly1305 (AEAD)
  - 24-byte random nonce per encryption
  - 16-byte Poly1305 MAC for authentication
  - Associated data (AAD) prevents ciphertext swapping

- **Subkey Derivation**: crypto_kdf
  - Separate subkeys for notes, settings, etc.
  - Prevents cross-use of cryptographic material

See [docs/CRYPTO_SPEC.md](docs/CRYPTO_SPEC.md) for detailed cryptographic specification.

## Threat Model

### Protected Against
- Attacker copying the SQLite database file
- Curious local user without the master password
- Disk inspection or backups

### Not Protected Against
- Compromised operating system
- Keyloggers or malware
- Weak user passwords
- Physical access while app is unlocked

See [docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) for complete threat model.

## Development Workflow

### Running Tests
```powershell
# Run all tests
.\build\Debug\bastionx_tests.exe

# Run tests with verbose output
.\build\Debug\bastionx_tests.exe --gtest_verbose

# Run specific test
.\build\Debug\bastionx_tests.exe --gtest_filter=CryptoServiceTest.EncryptDecryptRoundTrip
```

### Rebuilding
```powershell
# Clean build
cd build
cmake --build . --config Debug --clean-first

# Rebuild only changed files
cmake --build . --config Debug
```

## Contributing

This is currently a personal project for educational and portfolio purposes. If you have suggestions or find security issues, please open an issue.

## License

To be determined.

## Acknowledgments

- [libsodium](https://libsodium.gitbook.io/) for providing safe, modern cryptographic primitives
- [Qt Framework](https://www.qt.io/) for cross-platform UI capabilities
- [vcpkg](https://vcpkg.io/) for C++ package management

## Security Note

⚠️ **This software is in early development and has not undergone a security audit.**

While every effort has been made to follow cryptographic best practices, this software should not be used for highly sensitive data without proper review and auditing. Use at your own risk.

---

**Documentation:**
- [CLAUDE.md](CLAUDE.md) - Complete project specification
- [docs/BUILD.md](docs/BUILD.md) - Detailed build instructions
- [docs/CRYPTO_SPEC.md](docs/CRYPTO_SPEC.md) - Cryptographic specification
- [docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) - Security threat model
