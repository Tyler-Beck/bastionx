# Bastionx Threat Model

This document explicitly defines what Bastionx protects against and what it does **not** protect against.

## Table of Contents

1. [Threat Modeling Philosophy](#threat-modeling-philosophy)
2. [Assets to Protect](#assets-to-protect)
3. [Attacker Models](#attacker-models)
4. [Protected Scenarios](#protected-scenarios)
5. [Unprotected Scenarios](#unprotected-scenarios)
6. [Attack Surface Analysis](#attack-surface-analysis)
7. [Mitigations](#mitigations)
8. [Limitations](#limitations)
9. [Future Hardening](#future-hardening)

---

## Threat Modeling Philosophy

Bastionx follows these principles when evaluating security:

1. **Honesty Over Marketing**: We explicitly state what we cannot protect against
2. **Realistic Attacker Models**: We design for real-world threats, not theoretical perfection
3. **Layered Defense**: Multiple independent security measures
4. **Fail Securely**: When protections fail, data remains encrypted
5. **No False Promises**: We don't claim "military-grade" or "unbreakable" security

---

## Assets to Protect

### Primary Assets

1. **Note Contents**
   - Titles
   - Body text
   - Tags

2. **Note Metadata**
   - Relationships between notes
   - Note creation/update timestamps
   - Note IDs (when possible)

3. **User Intent**
   - What the user chooses to store
   - How they organize their information

### Secondary Assets

4. **Vault Master Password**
   - Never stored, only derived key
   - Protects all other assets

### Non-Assets (Not Protected)

- **Note Count**: Number of notes visible (encrypted, but count is visible)
- **Note Size**: Approximate size visible from ciphertext length
- **Access Patterns**: When notes are accessed (no protection)
- **Application Usage**: Fact that user is using Bastionx

---

## Attacker Models

### Attacker A: Passive Local Observer

**Capabilities**:
- Can read files from disk (e.g., backup, lost laptop)
- Can copy the SQLite database
- Cannot modify files (passive only)
- No access while application is unlocked

**Examples**:
- Thief steals laptop (powered off)
- Cloud backup provider reads backup
- System administrator reads home directory

**Bastionx Protection**: **PROTECTED**

### Attacker B: Active Local User (No Password)

**Capabilities**:
- Can read AND modify files
- Can run Bastionx application
- Does not know master password
- No OS-level privileges

**Examples**:
- Curious roommate
- Shared computer user
- Data forensics (without password cracking)

**Bastionx Protection**: **PROTECTED**

### Attacker C: Active Local User (With Physical Access While Unlocked)

**Capabilities**:
- Can access computer while Bastionx is running and unlocked
- Can read screen
- Can take screenshots
- Can access decrypted notes in memory

**Examples**:
- Attacker accesses computer while user is away (unlocked)
- Over-the-shoulder observation

**Bastionx Protection**: **NOT PROTECTED**

**Mitigation**: Auto-lock timer (Phase 4)

### Attacker D: Malware / OS Compromise

**Capabilities**:
- Can execute arbitrary code
- Can read all memory
- Can install keyloggers
- Can intercept all I/O
- Can tamper with Bastionx executable

**Examples**:
- Trojan horse
- Rootkit
- OS-level malware

**Bastionx Protection**: **NOT PROTECTED**

**Rationale**: No application can protect against OS compromise

### Attacker E: Weak Password Attacker

**Capabilities**:
- Can obtain encrypted database
- Can perform offline password guessing
- Has access to password cracking hardware (GPUs, ASICs)

**Examples**:
- User chooses password "123456" or "password"
- Attacker uses dictionary attack
- Attacker uses rainbow tables

**Bastionx Protection**: **PARTIAL PROTECTION**

**Mitigation**: Argon2id MODERATE settings slow down guessing (but cannot prevent weak passwords)

### Attacker F: Nation-State / Advanced Persistent Threat

**Capabilities**:
- Unlimited resources
- Access to zero-day exploits
- Hardware backdoors
- Supply chain attacks
- Physical device tampering

**Examples**:
- Government surveillance
- Sophisticated targeted attacks

**Bastionx Protection**: **NOT PROTECTED**

**Rationale**: Bastionx is not designed for high-threat environments

---

## Protected Scenarios

### Scenario 1: Lost or Stolen Laptop

**Situation**: Laptop is stolen while powered off

**Attacker Actions**:
1. Boot laptop
2. Access file system
3. Copy SQLite database
4. Attempt to decrypt

**Bastionx Protection**:
- All note data is encrypted with strong key (derived from password)
- Argon2id makes password cracking expensive
- No plaintext metadata leakage (beyond note count/size)

**Requirements**:
- User must have strong password (12+ characters, mixed case, numbers, symbols)

---

### Scenario 2: Cloud Backup Inspection

**Situation**: User backs up Bastionx database to cloud (e.g., Dropbox, OneDrive)

**Attacker Actions**:
1. Cloud provider scans backup files
2. Reads SQLite database
3. Attempts to extract information

**Bastionx Protection**:
- All sensitive data is encrypted before storage
- Cloud provider sees only ciphertext
- No way to decrypt without password

---

### Scenario 3: Disk Forensics (Without Password)

**Situation**: Forensics investigator analyzes hard drive

**Attacker Actions**:
1. Recover deleted files
2. Scan unallocated space
3. Analyze SQLite database
4. Look for plaintext remnants

**Bastionx Protection**:
- No plaintext data stored on disk
- Keys exist only in memory (wiped on exit)
- SQLite database contains only ciphertext

**Note**: This assumes investigator does NOT have:
- Password
- Memory dump from while app was unlocked
- Compromised OS with keylogger

---

### Scenario 4: Ciphertext Tampering

**Situation**: Attacker modifies encrypted database

**Attacker Actions**:
1. Obtain database file
2. Modify ciphertext bytes
3. Replace original database
4. User attempts to decrypt

**Bastionx Protection**:
- Poly1305 MAC authentication detects tampering
- Decryption fails with authentication error
- No corrupted plaintext returned

---

### Scenario 5: Ciphertext Swapping

**Situation**: Attacker swaps ciphertext between notes

**Attacker Actions**:
1. Copy ciphertext from Note A
2. Replace ciphertext of Note B with Note A's ciphertext
3. User opens Note B

**Bastionx Protection**:
- AAD (Additional Authenticated Data) includes note ID
- Decryption fails because AAD mismatch
- Prevents ciphertext confusion attacks

---

## Unprotected Scenarios

### Scenario 6: Keylogger Attack

**Situation**: Malware logs keystrokes

**Attacker Actions**:
1. Install keylogger via phishing, drive-by download, etc.
2. Wait for user to enter password
3. Capture password
4. Unlock vault with captured password

**Why Not Protected**:
- OS compromise is outside application security boundary
- No application can protect against this

**Mitigation**:
- Keep OS and antivirus updated
- Don't run untrusted software
- Use hardware-based authentication (future consideration)

---

### Scenario 7: Memory Dump While Unlocked

**Situation**: Attacker gains memory dump while vault is unlocked

**Attacker Actions**:
1. Use DLL injection, debugger, or OS exploit
2. Dump Bastionx process memory
3. Search for decrypted keys or plaintext

**Why Not Protected**:
- Keys must exist in memory to decrypt notes
- Memory protection is OS-level concern
- No practical mitigation for compromised OS

**Partial Mitigation**:
- Use secure memory (sodium_malloc with memory locking)
- Wipe keys on lock/exit (sodium_memzero)

---

### Scenario 8: Weak Password Brute Force

**Situation**: User chooses weak password, attacker brute forces

**Attacker Actions**:
1. Obtain encrypted database
2. Use password cracking tool (hashcat, John the Ripper)
3. Try common passwords (dictionary attack)
4. Find password and decrypt

**Why Not Protected**:
- Argon2id slows down guessing but cannot prevent it
- Weak passwords are inherently vulnerable

**Example**:
- Password: "password123"
- Argon2id @ 500ms per guess
- Time to crack: Minutes (at most)

**Mitigation**:
- Encourage strong passwords (12+ characters)
- Consider password strength meter (UI feature)
- Educate users on password security

---

### Scenario 9: Cold Boot Attack

**Situation**: Attacker reboots computer and extracts RAM

**Attacker Actions**:
1. Freeze RAM with liquid nitrogen (or use cold boot)
2. Reboot to forensics OS
3. Dump RAM contents
4. Search for encryption keys

**Why Not Protected**:
- RAM contents persist briefly after power loss
- Keys in memory are vulnerable during this window
- Requires physical access and specialized tools

**Mitigation**:
- Close Bastionx when not in use
- Use full-disk encryption (OS-level, e.g., BitLocker)

---

### Scenario 10: Side-Channel Attacks

**Situation**: Attacker measures timing, power, or EM emissions

**Attacker Actions**:
1. Measure CPU cache timing during key derivation
2. Analyze power consumption patterns
3. Extract key material via side channels

**Why Not Protected**:
- libsodium uses Argon2id (some side-channel resistance)
- Full protection requires hardware security modules
- Practical attacks require physical proximity

**Mitigation**:
- Use Argon2id (hybrid mode with some side-channel resistance)
- Physical security of device

---

## Attack Surface Analysis

### Current Attack Surface (Phase 1)

| Component | Attack Vectors | Mitigations |
|-----------|----------------|-------------|
| Password Input | Keylogger, shoulder surfing | None (OS-level concern) |
| Key Derivation | Weak password brute force | Argon2id MODERATE (slows guessing) |
| Encryption | Implementation bugs | Use libsodium (audited), unit tests |
| Memory | Memory dumps, cold boot | sodium_malloc (memory locking), sodium_memzero |
| Storage | Disk access, backups | Encrypt before storage |

### Future Attack Surface (Phase 2-4)

| Component | Attack Vectors | Planned Mitigations |
|-----------|----------------|---------------------|
| Auto-Lock | Bypass, race conditions | Secure timer implementation |
| Clipboard | Clipboard history, malware | Clear clipboard on lock (optional) |
| UI | Screenshots, screen recording | None (OS-level concern) |
| Settings | Weak settings, misconfiguration | Secure defaults |

---

## Mitigations

### Implemented Mitigations (Phase 1)

1. **Argon2id MODERATE**: Slows password cracking to ~500ms per guess
2. **XChaCha20-Poly1305**: Provides confidentiality + authentication
3. **Random Nonces**: Prevents nonce reuse (collision probability < 2^-96)
4. **AAD**: Prevents ciphertext swapping between notes
5. **Key Separation**: Notes and settings use different subkeys
6. **Secure Memory**: Keys allocated with sodium_malloc (locked, guarded)
7. **Memory Wiping**: Keys zeroed with sodium_memzero on destruction

### Planned Mitigations (Phase 2-4)

8. **Auto-Lock Timer**: Automatically lock vault after inactivity
9. **Manual Lock Button**: User can immediately lock vault
10. **Clipboard Clearing**: Optionally clear clipboard on lock
11. **Password Change**: Allow user to re-encrypt with new password

---

## Limitations

### Explicit Non-Goals

Bastionx **intentionally does NOT**:

1. Protect against compromised operating systems
2. Protect against hardware keyloggers
3. Protect against physical access while unlocked
4. Protect against weak user-chosen passwords
5. Provide password recovery (by design)
6. Protect against nation-state attackers
7. Provide plausible deniability (hidden volumes)
8. Protect against rubber-hose cryptanalysis (coercion)

### Why These Limitations?

- **Focused Scope**: Trying to protect against everything protects against nothing
- **Realistic Threat Model**: Most users face opportunistic attackers, not APTs
- **Honesty**: Users deserve to know what we cannot protect against
- **Simplicity**: Complex defenses increase attack surface

---

## Future Hardening (Phase 4)

### Potential Enhancements

1. **Password Strength Meter**: Warn users about weak passwords
2. **YubiKey Support**: Hardware-based authentication (future research)
3. **Secure Input**: Consider OS-level secure input (if available)
4. **Anti-Debugging**: Detect debuggers and refuse to decrypt (debatable value)
5. **Code Signing**: Sign executable to detect tampering
6. **Reproducible Builds**: Allow users to verify binary integrity

### Enhancements NOT Planned

- **Plausible Deniability**: Adds complexity, limited real-world value
- **Anti-Forensics**: Not a goal (we trust encryption, not obscurity)
- **Steganography**: Out of scope

---

## Threat Model Summary

### Bastionx is Designed For:

- **Privacy-conscious users** who want local-only note storage
- **Protection against disk access** (lost laptop, backups, forensics)
- **Protection against unauthorized local users** (shared computers)
- **Defense against opportunistic attackers** (thieves, curious roommates)

### Bastionx is NOT Designed For:

- **High-threat environments** (nation-state surveillance)
- **Protection against OS compromise** (malware, rootkits)
- **Protection against weak passwords** (user responsibility)
- **Protection while unlocked** (physical access, screen capture)

---

## User Guidance

### How to Stay Secure with Bastionx

1. **Use a Strong Password**:
   - At least 12 characters
   - Mix of uppercase, lowercase, numbers, symbols
   - Consider passphrase (e.g., "correct horse battery staple")

2. **Keep Your OS Secure**:
   - Install security updates
   - Use antivirus/antimalware
   - Don't run untrusted software

3. **Lock When Not in Use**:
   - Lock Bastionx when stepping away
   - Enable auto-lock (Phase 4)

4. **Secure Your Backups**:
   - Backup encrypted database (it's safe to backup)
   - Store backups securely
   - Test backup restoration

5. **Don't Share Passwords**:
   - Never write down password in plaintext
   - Don't share vault password

---

## Acknowledgments

This threat model is influenced by:
- [OWASP Threat Modeling](https://owasp.org/www-community/Threat_Modeling)
- [Microsoft STRIDE](https://en.wikipedia.org/wiki/STRIDE_(security))
- [Bruce Schneier's Security Mindset](https://www.schneier.com/essays/archives/2008/03/the_security_mindset.html)

---

**Last Updated**: 2026-02-13
**Bastionx Version**: 0.7.0 (Phase 8 Complete)
**Document Version**: 1.2
