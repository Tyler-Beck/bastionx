```
██████╗  █████╗ ███████╗████████╗██╗ ██████╗ ███╗   ██╗██╗  ██╗
██╔══██╗██╔══██╗██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║╚██╗██╔╝
██████╔╝███████║███████╗   ██║   ██║██║   ██║██╔██╗ ██║ ╚███╔╝
██╔══██╗██╔══██║╚════██║   ██║   ██║██║   ██║██║╚██╗██║ ██╔██╗
██████╔╝██║  ██║███████║   ██║   ██║╚██████╔╝██║ ╚████║██╔╝ ██╗
╚═════╝ ╚═╝  ╚═╝╚══════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝
```

<div align="center">

**A privacy-first, zero-knowledge encrypted notes vault for Windows.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](docs/BUILD.md)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](CMakeLists.txt)
[![Version](https://img.shields.io/badge/version-0.7.0-amber.svg)](CHANGELOG.md)
<!-- Update the build badge URL below with your GitHub username after publishing -->
[![Build](https://github.com/Tyler-Beck/bastionx/actions/workflows/build.yml/badge.svg)](../../actions/workflows/build.yml)

</div>

---

> No cloud. No accounts. No telemetry. Your notes, encrypted, on your machine — full stop.

<!-- TODO: Add a screenshot of the amber UI here -->
<!-- ![BastionX Screenshot](docs/screenshots/main.png) -->

---

## What is BastionX?

BastionX is a desktop notes application where every note is encrypted with a master password that never leaves your machine. Think of it as a local alternative to Google Docs or Notion, built for people who want their sensitive notes to stay private — encrypted at rest, encrypted in the database, secured in memory.

Built on battle-tested cryptography from [libsodium](https://libsodium.gitbook.io/) and [SQLCipher](https://www.zetetic.net/sqlcipher/), with a zero-knowledge design: even if someone copies your vault file, they get ciphertext.

---

## Features

- **Zero-knowledge encryption** — Argon2id key derivation + XChaCha20-Poly1305 AEAD per note
- **Defense-in-depth** — SQLCipher encrypts the entire database as a second layer
- **Secure memory** — key material lives in `sodium_malloc` memory, wiped with `sodium_memzero` on lock
- **Auto-lock** — configurable inactivity timeout wipes keys from memory
- **Password change** — atomic re-encryption of all notes in a single SQLite transaction
- **Full-text search** — searches across encrypted notes (decrypt-in-memory, never stores plaintext)
- **Tags system** — organize notes with a tag chip UI
- **Find & Replace** — in-editor search with match counting
- **Rich text editor** — formatting toolbar with bold, italic, headings, lists, code blocks
- **Multi-tab editor** — open multiple notes simultaneously with per-tab undo history
- **Clipboard guard** — optional clipboard clearing after sensitive copy operations
- **Amber cyberpunk UI** — dark theme with amber accent palette

---

## Quick Start

### Download (Recommended)

Pre-built Windows binaries are available on the [Releases page](../../releases/latest).

> Download `BastionX-x64.zip`, extract, and run `bastionx.exe`. No installer required.

### Build from Source

See [docs/BUILD.md](docs/BUILD.md) for the full guide. Short version:

**Prerequisites**: Visual Studio 2022 (C++ workload), CMake 3.21+, vcpkg

```powershell
# Install vcpkg (one-time)
git clone https://github.com/Microsoft/vcpkg.git C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat
C:\dev\vcpkg\vcpkg integrate install

# Build BastionX
git clone https://github.com/<your-username>/bastionx.git
cd bastionx
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

> **Note**: Qt 6 dependency is ~2-3 GB and takes 30-60 minutes to install on first build. Be patient.

---

## Security Model

BastionX uses [libsodium](https://libsodium.gitbook.io/) exclusively — no custom cryptography.

| Layer | Primitive | Purpose |
|---|---|---|
| Key derivation | Argon2id (MODERATE: 3 passes, 256MB RAM) | Derive 32-byte master key from password |
| Subkeys | BLAKE2b KDF (`crypto_kdf`) | 4 independent subkeys from master key |
| Note encryption | XChaCha20-Poly1305 AEAD | Per-note encryption with random 24-byte nonce |
| Database encryption | SQLCipher (PBKDF2-HMAC-SHA512, 256k iter) | Full database-level encryption |
| Memory | `sodium_malloc` / `sodium_memzero` | Keys in locked, non-swappable memory; wiped on lock |

**Protected against**: copying the vault file, disk inspection, casual physical access while locked.

**Not protected against**: keyloggers, a compromised OS, physical access while unlocked.

See [docs/CRYPTO_SPEC.md](docs/CRYPTO_SPEC.md) and [docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) for the full specification.

> **Audit status**: This software has not undergone a formal third-party security audit. Community security review is welcomed — see [SECURITY.md](SECURITY.md) for responsible disclosure.

---

## Technology Stack

| Component | Technology |
|---|---|
| Language | C++20 |
| UI Framework | Qt 6 (Widgets) |
| Cryptography | libsodium 1.0.20 |
| Database | SQLCipher 4.6.1 (encrypted SQLite) |
| Serialization | nlohmann/json 3.12.0 |
| Build System | CMake 3.21+ with vcpkg |
| Testing | Google Test 1.17.0 |

---

## Project Structure

```
bastionx/
├── include/bastionx/           # Public headers
│   ├── crypto/                 # CryptoService, SecureMemory
│   ├── storage/                # NotesRepository
│   ├── vault/                  # VaultService, VaultSettings
│   └── ui/                     # All UI component headers
├── src/                        # Implementation
│   ├── crypto/                 # libsodium wrappers
│   ├── storage/                # SQLCipher CRUD
│   ├── vault/                  # Vault lifecycle and key management
│   └── ui/                     # Qt Widgets implementation (15 components)
├── tests/                      # Google Test suite
│   ├── crypto/                 # Key derivation, encryption/decryption tests
│   ├── storage/                # Repository and search tests
│   ├── vault/                  # Vault lifecycle and password change tests
│   └── integration/            # End-to-end integration tests
└── docs/                       # Documentation
    ├── CRYPTO_SPEC.md          # Cryptographic specification
    ├── THREAT_MODEL.md         # Security threat model
    └── BUILD.md                # Detailed build guide
```

---

## Running Tests

```powershell
# From the build directory
.\Debug\bastionx_tests.exe

# Filter to a specific test
.\Debug\bastionx_tests.exe --gtest_filter=CryptoServiceTest.*

# Verbose output
.\Debug\bastionx_tests.exe --gtest_verbose
```

---

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) before submitting a PR — especially the section on cryptographic code.

For security vulnerabilities, see [SECURITY.md](SECURITY.md). Do not open public Issues for security findings.

---

## Documentation

| Document | Description |
|---|---|
| [MANIFESTO.md](MANIFESTO.md) | Project motivation, design philosophy, and implementation overview |
| [docs/CRYPTO_SPEC.md](docs/CRYPTO_SPEC.md) | Detailed cryptographic specification |
| [docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) | What BastionX protects against (and doesn't) |
| [docs/BUILD.md](docs/BUILD.md) | Complete build instructions |
| [CHANGELOG.md](CHANGELOG.md) | Version history |
| [CLAUDE.md](CLAUDE.md) | Internal design decisions and implementation notes |

---

## License

BastionX is licensed under the [MIT License](LICENSE).

Third-party dependencies (Qt, libsodium, SQLCipher, nlohmann-json) have their own licenses — see [LICENSE](LICENSE) for the full list and attribution.

> **Qt LGPL note**: Qt 6 is used under LGPL v3. Binary distributions of BastionX dynamically link Qt, preserving your right to relink against a modified Qt. Source distributions (this repo) allow you to build with any compatible Qt installation.

---

## Acknowledgments

- [libsodium](https://libsodium.gitbook.io/) — safe, audited, modern cryptographic primitives
- [SQLCipher](https://www.zetetic.net/sqlcipher/) — transparent, full-database encryption for SQLite
- [Qt Framework](https://www.qt.io/) — cross-platform desktop UI
- [vcpkg](https://vcpkg.io/) — C++ package management
- [nlohmann/json](https://github.com/nlohmann/json) — clean, header-only JSON for C++
