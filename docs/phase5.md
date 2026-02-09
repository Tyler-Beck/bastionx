# Bastionx Phase 5: Full Database Encryption (SQLCipher)

## Context

Phases 0-4 complete (75 tests passing). Currently, note/settings **content** is encrypted at the application layer (XChaCha20-Poly1305), but the SQLite database file itself is plaintext. Anyone with file access can see:
- Table schemas, column names, row counts
- Timestamps (created_at, updated_at) in the clear
- KDF parameters and salt in vault_meta
- WAL journal contents

**Goal**: Encrypt the entire `.db` file at rest using SQLCipher (AES-256 page-level encryption) so it's indistinguishable from random bytes. Combined with existing app-layer encryption, this gives **defense in depth** — two independent encryption layers.

---

## Architecture: Raw Key via Argon2id (not SQLCipher's PBKDF2)

SQLCipher normally uses PBKDF2-SHA512 (256k iterations). We already use **Argon2id** (memory-hard, GPU-resistant), which is strictly stronger. So we bypass SQLCipher's internal KDF entirely:

- New constant: `SUBKEY_DATABASE = 4` in CryptoService
- Derive `db_subkey = derive_subkey(master_key, 4)` — raw 32 bytes
- Pass via `sqlite3_key(db, raw_key, 32)` after every `sqlite3_open()`

### Salt Sidecar File

**Problem**: Salt is currently stored *inside* the DB (vault_meta). With full-DB encryption, we can't read it without the key we're trying to derive.

**Solution**: Move salt to a **sidecar file** (`vault.salt`, 16 bytes, plaintext next to `vault.db`). Salt is not secret — it only prevents rainbow tables. Standard practice (libsodium, KeePass, etc.)

### Unlock Flow (after migration)

```
1. Read 16-byte salt from vault.salt
2. Argon2id(password, salt) -> master_key
3. derive_subkey(master_key, SUBKEY_DATABASE=4) -> db_subkey (32 bytes)
4. sqlite3_open("vault.db") -> sqlite3_key(db, db_subkey, 32)
5. DB now accessible -- derive notes/settings/verify subkeys as before
```

### Security PRAGMAs (set after keying every connection)

```sql
PRAGMA cipher_memory_security = ON;   -- Wipe memory on free (like sodium_memzero)
PRAGMA journal_mode = WAL;            -- WAL files also encrypted with same key
```

---

## Files to Modify (10)

| File | Changes |
|------|---------|
| `vcpkg.json` | Replace `"sqlite3"` with `"sqlcipher"` |
| `CMakeLists.txt` | Update find_package / link targets for SQLCipher |
| `include/bastionx/crypto/CryptoService.h` | Add `SUBKEY_DATABASE = 4` |
| `include/bastionx/vault/VaultService.h` | Add `db_subkey_`, salt file I/O, update ScopedDb |
| `src/vault/VaultService.cpp` | Key every DB connection, salt file R/W, migration helper |
| `include/bastionx/storage/NotesRepository.h` | Add `db_key` parameter to constructor |
| `src/storage/NotesRepository.cpp` | Call `sqlite3_key()` after open |
| `src/ui/MainWindow.cpp` | Pass `db_subkey` when creating NotesRepository |
| `tests/storage/NotesRepositoryTest.cpp` | Pass key to constructor; key direct sqlite3 connections |
| `tests/vault/PasswordChangeTest.cpp` | Pass key to NotesRepository constructor |

## Files to Create (1)

| File | Purpose |
|------|---------|
| `tests/vault/SQLCipherTest.cpp` | Verify encryption is active, raw file is opaque, wrong key fails |

---

## Step-by-Step Implementation

### Step 1: Install SQLCipher + Build System

Install:
```
vcpkg install sqlcipher:x64-windows --x-install-root="c:/Users/17326/spring2026/bastionx/vcpkg_installed"
```

`vcpkg.json`: replace `"sqlite3"` with `"sqlcipher"`

`CMakeLists.txt`: Update find_package and link target (verify exact target name after install -- may be `SQLCipher::SQLCipher` or `SQLite::SQLite3` depending on SQLCipher version).

**Gate**: Build compiles with SQLCipher linked, all 75 existing tests still pass (no code changes yet -- SQLCipher is API-compatible).

---

### Step 2: Add SUBKEY_DATABASE = 4

`include/bastionx/crypto/CryptoService.h`:
```cpp
static constexpr uint64_t SUBKEY_DATABASE = 4;  // Full-DB encryption key
```

---

### Step 3: VaultService -- Salt Sidecar + DB Keying

**VaultService.h** new members/methods:
```cpp
std::optional<crypto::SecureKey> db_subkey_;

const crypto::SecureKey& db_subkey() const;  // throws if locked

// Private helpers
static std::string salt_path(const std::string& vault_path);  // returns vault_path + ".salt"
void write_salt_file(const std::array<uint8_t, crypto::CryptoService::SALT_BYTES>& salt);
bool read_salt_file(std::array<uint8_t, crypto::CryptoService::SALT_BYTES>& salt);
```

**ScopedDb** -- accept optional raw key:
```cpp
ScopedDb(const std::string& path, const crypto::SecureKey* db_key = nullptr) {
    sqlite3_open(path, &db_);
    if (db_key) {
        sqlite3_key(db_, db_key->data(), static_cast<int>(db_key->size()));
        exec_sql(db_, "PRAGMA cipher_memory_security = ON;");
    }
}
```

**create()** changes:
1. Generate salt -> `write_salt_file(salt)`
2. Derive master_key -> derive `db_subkey_`
3. `ScopedDb db(vault_path_, &*db_subkey_)` -- DB encrypted from birth
4. `PRAGMA journal_mode=WAL` after keying
5. Rest unchanged

**unlock()** changes:
1. `read_salt_file(salt)` -- fail if missing
2. Derive master_key -> derive `db_subkey_`
3. `ScopedDb db(vault_path_, &*db_subkey_)`
4. Try `load_vault_meta()` -- if query fails, wrong password -> wipe keys, return false
5. Rest unchanged (verify token, derive subkeys, migrate_schema)

**All other ScopedDb calls** (save_settings, load_settings, change_password): pass `&*db_subkey_`

**wipe_keys()**: add `db_subkey_.reset()`

**Gate**: All 18 VaultServiceTest + 7 PasswordChangeTest pass (they create fresh vaults, so they'll be encrypted from creation).

---

### Step 4: NotesRepository -- Key Parameter

```cpp
// Header
explicit NotesRepository(const std::string& db_path,
                         const crypto::SecureKey* db_key = nullptr);

// Implementation
NotesRepository::NotesRepository(const std::string& db_path, const crypto::SecureKey* db_key)
    : db_(nullptr), db_path_(db_path) {
    sqlite3_open(db_path.c_str(), &db_);
    if (db_key) {
        sqlite3_key(db_, db_key->data(), static_cast<int>(db_key->size()));
        exec_sql(db_, "PRAGMA cipher_memory_security = ON;");
    }
    exec_sql(db_, "PRAGMA journal_mode=WAL;");
}
```

**Gate**: All 15 NotesRepositoryTest pass (after fixture update in Step 5).

---

### Step 5: MainWindow + Test Fixture Updates

**MainWindow.cpp** -- 3 locations creating NotesRepository:
```cpp
// showNotesPanel(), onPasswordChangeRequested() x2
repo_ = std::make_unique<storage::NotesRepository>(vault_->vault_path(), &vault_->db_subkey());
```

**NotesRepositoryTest.cpp** fixture:
```cpp
repo_ = std::make_unique<NotesRepository>(vault_path_, &vault_->db_subkey());
```
Plus `FreshNonceOnUpdate` direct sqlite3 access:
```cpp
sqlite3_open(vault_path_.c_str(), &db);
sqlite3_key(db, vault_->db_subkey().data(), static_cast<int>(vault_->db_subkey().size()));
```

**PasswordChangeTest.cpp**: Same pattern for NotesRepository construction.

**Gate**: All 75 existing tests pass.

---

### Step 6: Migration Utility (Unencrypted -> Encrypted)

For existing Phase 4 users. In `VaultService::unlock()`:

```cpp
// After deriving db_subkey, try encrypted open
// If first query fails AND no vault.salt exists:
//   -> Attempt plaintext open (no key)
//   -> If that succeeds, run migrate_to_encrypted()
//   -> Reopen as encrypted
```

`migrate_to_encrypted()` uses SQLCipher's built-in export:
```sql
ATTACH DATABASE 'vault_new.db' AS encrypted KEY <raw_key>;
SELECT sqlcipher_export('encrypted');
DETACH DATABASE encrypted;
```
Then atomically replace `vault.db` with `vault_new.db` and write the `vault.salt` sidecar.

---

### Step 7: SQLCipher Verification Tests

New `tests/vault/SQLCipherTest.cpp`:

| Test | Verifies |
|------|----------|
| `DatabaseFileIsOpaque` | Raw vault.db has no "SQLite format 3" header, no readable strings |
| `WrongKeyCannotOpen` | Open with wrong raw key -> first query fails |
| `CorrectKeyOpens` | Open with correct key -> queries succeed |
| `WALFileEncrypted` | WAL file (if exists) contains no plaintext |
| `SaltFileCreated` | After create(), vault.salt is exactly 16 bytes |
| `SaltFileMatchesVaultMeta` | Sidecar salt matches salt stored inside vault_meta |

---

## Implementation Order Summary

| Step | What | Gate |
|------|------|------|
| **1** | Install SQLCipher, update build system | Build compiles, 75 tests pass |
| **2** | Add `SUBKEY_DATABASE = 4` | Trivial |
| **3** | VaultService: salt sidecar + DB keying | 18 + 7 VaultService/PasswordChange tests pass |
| **4** | NotesRepository: db_key parameter | 15 NotesRepository tests pass |
| **5** | MainWindow + test fixture updates | All 75 existing tests pass |
| **6** | Migration utility (unencrypted -> encrypted) | ~2 migration tests |
| **7** | SQLCipher verification tests | ~6 new tests |

**Expected total: ~83 tests**

---

## Verification Checklist

1. **Build**: `cmake --build build --config Debug` -- zero errors
2. **All tests**: `build/tests/Debug/bastionx_tests.exe` -- all ~83 pass
3. **File opacity**: `xxd vault.db | head` -- no readable strings, no SQLite magic header
4. **strings check**: `strings vault.db` -- returns nothing meaningful
5. **Wrong password**: Lock, try wrong password -- fails, no data leaked
6. **WAL opacity**: `vault.db-wal` is also encrypted (no plaintext)
7. **Salt sidecar**: `vault.salt` exists, exactly 16 bytes
8. **Migration**: Place a Phase 4 (unencrypted) vault.db -> app auto-migrates on unlock
9. **Run app**: `run.bat` -- create vault, add notes, lock, unlock, settings -- all works
10. **Package**: Update `scripts/package.bat` to copy `sqlcipher.dll` instead of `sqlite3.dll`
