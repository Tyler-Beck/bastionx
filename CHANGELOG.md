# Changelog

All notable changes to BastionX will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Planned
- Future UI/UX enhancements and optimizations

## [0.7.0] - 2026-02-13 - Phase 8 Complete

### Added
- **Phase 8: Amber Glassmorphic Cyberpunk UI**
  - Complete color palette transformation from green to amber
  - Amber accent colors (#f59e0b, #fbbf24, #fcd34d)
  - Warm dark backgrounds (#0f0a08, #171210, #1f1a16)
  - Amber-tinted glassmorphic selection states
  - Updated all 27 color instances across UI
  - Cyberpunk/terminal aesthetic with professional security feel

### Changed
- Transformed visual identity from Matrix-green to amber cyberpunk
- Updated StyleSheet.h header to Phase 8 designation
- Fixed UnlockScreen title color from green to amber
- All UI components now use warm amber accent palette

## [0.6.0] - 2026-02-13 - Phase 6-7 Complete

### Added
- **Phase 6: Search & Organization**
  - Full-text search across encrypted notes with 300ms debounce
  - SearchPanel UI component with results preview
  - Search across title, body, and tags
  - Tags system for note organization
  - TagsWidget with chip UI and inline editing
  - Find/replace functionality in note editor
  - FindBar component with match counting

- **Phase 7: Premium Dark UI**
  - Centralized QSS stylesheet (734 lines) in StyleSheet.h
  - Activity bar navigation (Notes/Search/Settings modes)
  - Multi-tab editor with close buttons and undo history
  - Rich text formatting toolbar (bold, italic, headings, lists, code, etc.)
  - Status bar with encryption indicator and word/character count
  - Component-specific styling for all UI elements
  - Improved visual hierarchy and spacing

### Changed
- UI completely restyled with consistent dark theme
- All components now use centralized stylesheet

## [0.5.0] - 2026-02-12 - Phase 5 Complete

### Added
- SQLCipher database-level encryption (defense-in-depth)
- Database re-keying on password change via PRAGMA rekey
- Secure PRAGMA configuration (256k iterations, SHA512 HMAC)

### Changed
- Migrated from sqlite3 to sqlcipher dependency
- Enhanced password change to atomically re-encrypt database
- Database key derived from master key using crypto_kdf

### Security
- Database files now encrypted at rest (defense-in-depth)
- PRAGMA key never written to disk
- Atomic re-keying prevents partial encryption states

## [0.4.0] - Phase 4 Complete

### Added
- Settings dialog with configuration UI
- Password change functionality with vault re-encryption
- Auto-lock with configurable inactivity timeout
- Inactivity detection via event filtering
- Clipboard guard for sensitive data protection
- Memory wiping on lock (sodium_memzero)

### Changed
- Vault lifecycle management enhanced with auto-lock
- All cryptographic keys wiped on lock event

## [0.3.0] - Phase 3 Complete

### Added
- NotesPanel main UI orchestrator
- Sidebar with ActivityBar (Notes/Search/Settings)
- NotesList widget with filtering
- NoteEditor with title and rich text body
- MainWindow with toolbar and lock button
- UnlockScreen for vault password entry
- Multi-tab support for editing multiple notes

### Changed
- UI framework fully integrated with backend services

## [0.2.0] - Phase 2 Complete

### Added
- VaultService for vault lifecycle (create/unlock/lock)
- NotesRepository for encrypted note storage
- SQLite database integration (pre-SQLCipher)
- Settings persistence (encrypted)
- Encrypted token for password verification
- Note CRUD operations (create, read, update, delete)

### Security
- Master key derived from password via Argon2id
- Subkey derivation for notes and settings
- Password verification without storing plaintext password

## [0.1.0] - 2026-02-05 - Phase 0-1 Complete

### Added
- Project infrastructure and build system
- CMake build configuration
- vcpkg dependency management
- Cryptographic core (libsodium integration)
- CryptoService with Argon2id KDF
- XChaCha20-Poly1305 AEAD encryption/decryption
- SecureMemory RAII wrapper
- Comprehensive test suite (23 tests)
- Test vectors for cryptographic operations
- Documentation (CLAUDE.md, BUILD.md, CRYPTO_SPEC.md, THREAT_MODEL.md)

### Security
- Zero-knowledge encryption design
- Secure memory management with sodium_malloc/memzero
- Associated authenticated data (AAD) prevents ciphertext swapping

---

**Legend:**
- **Added**: New features
- **Changed**: Changes to existing functionality
- **Deprecated**: Soon-to-be removed features
- **Removed**: Removed features
- **Fixed**: Bug fixes
- **Security**: Security-related changes
