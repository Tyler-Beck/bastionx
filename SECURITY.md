# Security Policy

## Supported Versions

| Version | Supported |
|---|---|
| 0.7.x (current) | ✅ Active development |
| < 0.7.0 | ❌ No longer supported |

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub Issues.**

If you discover a security vulnerability, please report it privately so it can be addressed before public disclosure. Use one of the following methods:

1. **GitHub Private Vulnerability Reporting** (preferred) — use the "Report a vulnerability" button on the [Security tab](../../security/advisories/new) of this repository.
2. **Email** — if you prefer email, open a public Issue asking for a contact address and one will be provided.

### What to Include

A useful report includes:
- A description of the vulnerability
- Steps to reproduce or a proof-of-concept
- Potential impact assessment
- Any suggested mitigations (optional)

### Response Timeline

- **Acknowledgment**: Within 3 business days of receiving the report
- **Initial assessment**: Within 7 days
- **Fix or mitigation**: Depends on severity and complexity — critical issues are prioritized

### Disclosure Policy

BastionX follows **coordinated disclosure**:

1. The reporter submits the vulnerability privately
2. A fix is developed and tested
3. A patched release is published
4. A public security advisory is issued with credit to the reporter (unless anonymity is requested)

The minimum embargo period before public disclosure is **90 days**, but we aim to fix critical issues much faster.

---

## Security Design Notes

BastionX is a **local-only, offline application** with no network connectivity. There are no servers, no APIs, and no cloud components.

### What BastionX Is Designed to Protect Against
- An attacker who obtains a copy of your vault `.db` file
- Disk inspection or file system forensics
- Casual physical access to your machine while the app is locked

### Known Limitations (By Design)
- A **compromised operating system** defeats all local encryption
- **Keyloggers or screen-capture malware** can capture your master password at entry
- **Physical access while the app is unlocked** bypasses all protections
- **Weak master passwords** reduce the effectiveness of Argon2id

See [docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) for the complete threat model.

### Cryptographic Stack
- **Key derivation**: Argon2id via libsodium (`crypto_pwhash`, MODERATE parameters)
- **Encryption**: XChaCha20-Poly1305 AEAD via libsodium
- **Database encryption**: SQLCipher (PBKDF2-HMAC-SHA512, 256k iterations)
- **Memory security**: `sodium_malloc` / `sodium_memzero` for key material
- **No custom cryptography**: All primitives are from libsodium

### Audit Status

This software has **not undergone a formal third-party security audit**. It follows well-established cryptographic practices and uses audited libraries (libsodium, SQLCipher), but community review is welcomed and encouraged. If you find something questionable, please report it.
