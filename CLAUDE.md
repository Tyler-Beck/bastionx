# Bastionx – CLAUDE.md

> **Project Name:** Bastionx
> **Category:** Privacy-first, local-only desktop application
> **Platform:** Windows (primary and only supported target for v1)
> **Language / Framework:** C++20 + Qt 6
> **Crypto:** libsodium
> **Storage:** SQLCipher (encrypted SQLite database with defense-in-depth)

---

## 1. Project Purpose & Philosophy

Bastionx is a **local-first, zero-knowledge, privacy-respecting notes application** designed for users who do not trust cloud services with sensitive information.

This project intentionally avoids:

* Cloud sync
* User accounts
* Telemetry or analytics
* External APIs

The goal is **maximum privacy with minimum complexity**, while remaining usable enough for daily note-taking.

This is both:

* A real, usable application for privacy-conscious users
* A demonstrable software engineering & applied security project

---

## 2. Core Design Principles

### 2.1 Local-First

* All data is stored locally on disk
* No background services or network calls
* App works fully offline

### 2.2 Zero-Knowledge

* The application never stores or transmits plaintext data
* The developer (you) cannot decrypt user data
* Loss of the master password = permanent loss of data

### 2.3 Minimal Attack Surface

* Small dependency set
* No plugins or scripting engines
* No embedded browser

### 2.4 Explicit Threat Modeling

* Clear documentation of what *is* and *is not* protected
* No false claims of “perfect security”

---

## 3. Threat Model

### 3.1 Assets to Protect

* Note contents (titles, body, tags)
* Metadata relationships between notes
* User intent (what they write and store)

### 3.2 Attacker Assumptions

**Protected Against:**

* Attacker copying the SQLite database file
* Curious local user without the master password
* Disk inspection or backups

**Not Protected Against:**

* Compromised operating system
* Keyloggers or malware
* Weak user passwords
* Physical access while app is unlocked

---

## 4. Technology Stack (Locked)

### 4.1 Application Stack

* **C++20** – core logic and crypto boundary
* **Qt 6 (Widgets)** – UI framework
* **SQLite** – structured local storage
* **libsodium** – cryptographic primitives

### 4.2 Platform Scope

* **Windows only** (v1)
* No macOS or Linux considerations
* Installer-based deployment (MSI or portable EXE)

---

## 5. Cryptographic Design

### 5.1 Library Choice

**libsodium** is used exclusively for all cryptography due to:

* Modern primitives
* Safe defaults
* Active maintenance
* Clear API contracts

No custom cryptography is implemented.

---

### 5.2 Key Derivation

* Algorithm: **Argon2id**
* Function: `crypto_pwhash()`
* Parameters:

  * `OPSLIMIT_MODERATE`
  * `MEMLIMIT_MODERATE`
  * Algorithm: `ARGON2ID13`

Each vault generates:

* A unique 16-byte random salt

The derived **master key** is 32 bytes and exists **only in memory** while the vault is unlocked.

---

### 5.3 Key Separation

The master key is never used directly.

Subkeys are derived using `crypto_kdf_derive_from_key()`:

* `k_notes` – encrypt/decrypt note payloads
* `k_settings` – future expansion

This prevents cross-use of cryptographic material.

---

### 5.4 Encryption Algorithm

* **XChaCha20-Poly1305 (AEAD)**
* Function family: `crypto_aead_xchacha20poly1305_ietf_*`
* Nonce size: 24 bytes (random per record)

Associated Data (AAD):

* Note ID
* Updated timestamp

This ensures integrity and prevents ciphertext swapping.

---

### 5.5 Memory Handling

* Keys allocated via `sodium_malloc()` when possible
* Keys wiped using `sodium_memzero()` on:

  * Auto-lock
  * Manual lock
  * Application exit

---

## 6. Storage Design

### 6.1 SQLite Strategy

SQLite is used as a **container only**.

All sensitive data is encrypted **before** being written to disk.

No plaintext note data is ever stored.

---

### 6.2 Database Schema

#### `vault_meta`

* `version` (INTEGER)
* `salt` (BLOB)
* `kdf_opslimit` (INTEGER)
* `kdf_memlimit` (INTEGER)
* `created_at` (INTEGER)

#### `notes`

* `id` (INTEGER PRIMARY KEY)
* `nonce` (BLOB, 24 bytes)
* `ciphertext` (BLOB)
* `created_at` (INTEGER)
* `updated_at` (INTEGER)

---

### 6.3 Encrypted Payload Format

Each note payload contains:

* `title` (string)
* `body` (string)
* `tags` (array of strings)
* `version` (int)

Payloads are serialized (binary or JSON) *before encryption*.

---

## 7. Application Architecture

### 7.1 Core Components

* **CryptoService**

  * Key derivation
  * Encryption / decryption
  * Memory wiping

* **VaultService**

  * Lock / unlock vault
  * Password validation
  * Auto-lock timer

* **NotesRepository**

  * SQLite access
  * CRUD operations on encrypted blobs

* **UI Layer**

  * No cryptographic logic
  * Receives plaintext only when unlocked

Clear separation is enforced.

---

## 8. UI / UX Design

### 8.1 Visual Philosophy

* Minimal
* Monotone
* No gradients
* No unnecessary icons
* No animations beyond basic transitions

Design inspiration:

* Terminal UIs
* Password managers
* Security tooling

---

### 8.2 Typography

Primary font:

* **Fira Mono** (preferred)

Fallbacks:

* Consolas
* JetBrains Mono
* Monospace system default

All UI text uses a monospace font.

---

### 8.3 Color Palette

**Current Implementation (Phase 7: Premium Dark UI):**

**Backgrounds:**
* Base: `#0d0f14` (near-black)
* Surface levels: `#13151c` < `#1a1d26` < `#22262f` < `#2c3040`

**Text:**
* Primary: `#e8eaf0` (off-white)
* Secondary: `#a0a4b8` (light gray)
* Tertiary: `#606478` (muted gray)

**Accents:**
* Primary: `#4ade80` (bright green) - focus states, active elements
* Secondary: `#22c55e` (medium green) - borders, accents
* Tint: `#0f2318` (dark green tint) - selection backgrounds

**Semantic Colors:**
* Error/Destructive: `#f87171` (red)
* Info: `#60a5fa` (blue) - password change operations only

**Design Philosophy:**
* Minimal, security-focused aesthetic
* No gradients (except primary buttons)
* No animations beyond basic transitions
* Terminal UI inspired

---

### 8.4 Screens

* Vault unlock screen
* Notes list (titles decrypted in memory)
* Note editor
* Settings (password change, auto-lock time)

---

## 9. UX Security Decisions

* App launches **locked** by default
* Auto-lock after inactivity (configurable)
* Manual lock button always visible
* Clipboard clearing (optional future feature)

---

## 10. Deployment (Windows)

### 10.1 Build

* Static or minimal dynamic linking
* libsodium bundled or statically linked
* SQLite bundled

### 10.2 Distribution

* Portable EXE **or** MSI installer
* No auto-updater in v1

### 10.3 Storage Location

* Vault stored in:

  * `%APPDATA%/Bastionx/`

---

## 11. Development Phases

### Phase 1 – Cryptographic Core

* Password → key derivation
* Encrypt/decrypt test vectors

### Phase 2 – Storage Layer

* SQLite schema
* Encrypted note CRUD

### Phase 3 – UI MVP

* Notes list
* Editor
* Lock/unlock flow

### Phase 4 – Hardening

* Memory wiping
* Auto-lock
* Threat model documentation

### Phase 5 – SQLCipher Database Encryption ✅

**Goal**: Replace application-layer encryption with database-level encryption using SQLCipher

**Status**: Complete (commit: 5fd9c25)

**Implementation**:
* Integrated SQLCipher via vcpkg
* Database-level encryption with key derivation from master password
* Atomic password change with database re-keying
* Maintained backward compatibility with existing note encryption

**Key Files**:
* `CMakeLists.txt` - SQLCipher dependency integration
* `include/bastionx/storage/NotesRepository.h` - Updated for SQLCipher
* `src/storage/NotesRepository.cpp` - SQLCipher initialization and pragmas

### Phase 6 – Search & Organization ✅

**Goal**: Full-text search across encrypted notes

**Status**: Complete (commits: 5716ef1, 3d2745f, cebc236)

**Implementation**:
* SearchPanel UI component with debounced input (300ms)
* Full-text search decrypts notes on-the-fly for searching
* Search across title, body, and tags
* Results display with title and preview
* Click-to-open search results in editor
* Tags system for note organization
* Tags widget with chip UI
* Find/replace functionality

**Key Files**:
* `include/bastionx/ui/SearchPanel.h`
* `src/ui/SearchPanel.cpp`
* `include/bastionx/ui/TagsWidget.h`
* `src/ui/TagsWidget.cpp`
* `include/bastionx/ui/FindBar.h`
* `src/ui/FindBar.cpp`
* `include/bastionx/storage/NotesRepository.h` - search_notes() method
* `src/storage/NotesRepository.cpp` - search implementation

### Phase 7 – Premium Dark UI ✅

**Goal**: Polished dark theme UI with comprehensive styling

**Status**: Complete

**Implementation**:
* Centralized QSS stylesheet (734 lines) in StyleSheet.h
* Green accent color scheme (see section 8.3 for exact colors)
* Component-specific styling for all UI elements
* Activity bar with Notes/Search/Settings modes
* Multi-tab editor with close buttons and undo history
* Rich text formatting toolbar (bold, italic, headings, lists, etc.)
* Find/replace bar
* Tags widget with chip UI
* Status bar with encryption indicator
* Settings dialog with password change

**Key Files**:
* `src/ui/StyleSheet.h` - Complete QSS stylesheet
* 15 UI component files: `MainWindow`, `NotesPanel`, `NoteEditor`, `NotesList`, `UnlockScreen`, `ActivityBar`, `Sidebar`, `TabBar`, `SearchPanel`, `FormattingToolbar`, `FindBar`, `TagsWidget`, `StatusBar`, `SettingsDialog`, `ClipboardGuard`

---

## 12. Non-Goals (Explicit)

* Cloud sync
* Collaboration
* Mobile support
* Recovery keys
* Password reset

---

## 13. Resume Framing

This project demonstrates:

* Applied cryptography
* Secure systems design
* Local-first architecture
* Threat modeling
* Real-world C++ application development

---

## 14. Guiding Rule

> **If a feature compromises privacy, it does not ship.**

This document is the source of truth for all design decisions.
