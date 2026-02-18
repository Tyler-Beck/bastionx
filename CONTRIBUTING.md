# Contributing to BastionX

Thanks for your interest in contributing. BastionX is a privacy-first, local encryption notes app built in C++20 with Qt6. Contributions of all kinds are welcome — bug reports, security reviews, feature suggestions, documentation, and code.

---

## Before You Start

Read the [SECURITY.md](SECURITY.md) file. If your contribution touches cryptographic code or the vault logic, please read [docs/CRYPTO_SPEC.md](docs/CRYPTO_SPEC.md) and [docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) first.

**Security vulnerabilities must not be reported as public Issues.** See [SECURITY.md](SECURITY.md) for the private disclosure process.

---

## Ways to Contribute

### Bug Reports
Open a [GitHub Issue](../../issues/new?template=bug_report.md) using the bug report template. Include:
- What you expected to happen
- What actually happened
- Steps to reproduce
- Your environment (Windows version, build config Debug/Release)

### Feature Requests
Open a [GitHub Issue](../../issues/new?template=feature_request.md) using the feature request template. Describe the use case, not just the feature.

### Security Reviews
Reading through the crypto code and spot-checking the implementation against [docs/CRYPTO_SPEC.md](docs/CRYPTO_SPEC.md) is one of the most valuable things you can do. If you find something that looks wrong, report it privately per [SECURITY.md](SECURITY.md).

### Code Contributions
See the workflow below.

---

## Development Setup

### Prerequisites
- Windows 10/11
- Visual Studio 2022 (Desktop development with C++ workload)
- CMake 3.21+
- vcpkg — see [docs/BUILD.md](docs/BUILD.md) for the full setup guide

### Building

```powershell
git clone https://github.com/<your-fork>/bastionx.git
cd bastionx
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
```

### Running Tests

```powershell
# From the build directory
.\Debug\bastionx_tests.exe
```

All tests must pass before submitting a PR. If you're adding a feature, add tests for it.

---

## Code Style

- **Language**: C++20
- **Formatting**: Follow the style of surrounding code. No reformatting unrelated files.
- **Headers**: Keep public API in `include/bastionx/`, implementation in `src/`
- **Crypto rule**: All cryptographic operations go through `CryptoService`. The UI layer must never handle raw keys or ciphertext.
- **Memory rule**: Key material must be stored in `SecureKey` / `SecureBuffer<T>` (backed by `sodium_malloc`). Never use `std::string` or stack arrays for key material.
- **No new dependencies** without discussion in an Issue first — the current dependency footprint is intentional.

---

## Pull Request Process

1. Fork the repo and create a branch from `main`
2. Make your changes with tests
3. Ensure all tests pass (`.\Debug\bastionx_tests.exe`)
4. Open a PR with a clear description of what changed and why
5. Reference any related Issues in the PR description

PRs that modify cryptographic code will receive extra scrutiny and may take longer to review. This is intentional and not a sign of rejection.

---

## What Won't Be Accepted

To preserve the project's design goals, the following will not be merged:

- Cloud sync, cloud storage, or any network connectivity
- User account systems or server-side components
- Telemetry, analytics, or crash reporting
- Any third-party service integration
- Features that require storing or transmitting the master password
- Replacing libsodium primitives with custom cryptography

These are not limitations — they are the point of the project.

---

## License

By contributing to BastionX, you agree that your contributions will be licensed under the [MIT License](LICENSE).
