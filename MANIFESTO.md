# MANIFESTO

You might be wondering why I built this application. Truth be told, at the 
end of the day, it's just another notes app with no real advantage over any of the competitors. So, you might be asking, why should I use this?

Short answer, you shouldn't. I'm not an expert in the field with decades of experience in cryptography, secure design and implementation, and secure coding practices. I'm a senior Computer Science student trying to figure this whole thing out. With that said, if you decide to try out this app because you just want to see it for yourself, or find vulnerabilities which I'm not seeing, I welcome you with open arms.

This manifesto is one part an overview of my rationale and motivation for building this, one part what the future might look like, and one part my unfiltered and unorganized thoughts on the actual building of the application. Because with full transparency, I co-build this with Artificial Intelligence, and am now looking back on this project and writing about it to see if I actually know what I'm talking about. There is a good possibility the words I put here are in no way accurate about the current implementation of the project, so I warn you again to question everything, and do your own audit before using it yourself. I've done my best to secure everything as much as I possibly can, build with the idea that people may actually use this, and input information they deem critical, and I would be ashamed should that data ever be compromised. Thus, I warn you one more time, please be cognisant of what you get yourself into before getting into it, and that is not only speaking for this project.

## Motivation
I find it funny how we have gone to extreme measures as a community to build things that are deemed "impenetrable" and completely secure. Unfortunately, in modern day technology, there is no such thing. I remember when I was getting into the security aspect of software and technology, and becoming more and more paranoid about where my data was going, who was listening to my conversations, how my data was being used, the list goes on. I stumbled upon a website that I cannot remember at the moment, but if it comes back around, I'll be sure to give them their credit. Their page was about becoming "invisible" on the Internet. In their intro, they said something along the lines of "The Internet is the only game where the only way to win is to not play at all" and ever since then, I have gone down rabbit holes about security and privacy. 

You can use all the VPNs, Protons, Mullvads, Kodachis, Tails, Whonixs, and home non-Internet facing servers you can possibly want to achieve this idea of invisibility. Unfortunately, it's a game of not inches, not millimeters, but bits. One rearranged 0 and 1 can be the difference between invisible and compromised. 

You might be wondering "If there is no such thing as invisibilty on the Internet, then doesn't that make this application effectively rendered moot?" And to that I say, yes you're absolutely correct. The goal of Bastion isn't to be invisible, it's to increase the layers of defense making it more secure as a result. It's just an alternative to Google Docs or Microsoft Word. No clound sync, no account creation, encryption every which way, all ways to just make it a little bit harder to crack your data. Ultimately, this is a place you come where you want to keep information on the Internet that you would prefer didn't get seen. We're not talking about putting the nuclear codes in these docs, 1 that would be stupid, and 2 if you did have the nuclear codes what're you doing here?

Nonetheless, my motivation for this application ultimately comes down to this. I wanted to explore how secure I could build a notes app to see how comfortable I would feel putting my own sensitive data on it. 

## Implementation
Now, this is really why I'm writing this manifesto. I want to see if I really know the ins and outs of this project and it's capabilities and limitations. At probably the highest level of abstraction, Bastionx is a privacy based notes application where the goal was to have the functionality of a Google Docs with the aesthetics of a VSCode. Now, let's get into the meat and potatoes. 

The Stack
- C++20
- Qt6 Widgets
- libsodium for general encryption
- SQLCipher for Database encryption
- JSON data with AEAD encryption

Using C++ for a privacy and security centered application seems a little oxymoronic. To that I'd say, why do all of the mission critical space flights and other critical software services, tools, and products all use C/C++? Well, one reason is speed. Especially in places like medical environments, or space explorations, milliseconds mean everything. Which makes it understandable why they have to use languages like C or C++. Nowadays, Rust is making waves as the next big thing, though that is a conversation for another day. Back to why I picked C++. Well, I want to be good at C++... crazy I know. But, as I said before, I'm a senior CS guy just trying to get a job in this tumultuous market. Thus, I have been doing all my DSA practice in C++, so it just made sense to pick C++ for this project. It also helps combat the speed sacrifices made for all the encryption. If this was built in Python or something similar, it would take forever just to get into your vault...

I've been playing with desktop applications for a few months, and I elected to go that direction for this because I wanted to stay away from anything browser based. We go back to the privacy conversation. A web app just adds another nuance to consider when designing and applying all the security measures you want in place. This is why I elected for Qt and the widgets and features it allows.

Now for the core of this entire project, cryptography. I went for libsodium for general cryptography uses. In this project, we used libsodium in a multitude of places so I will try to lay them out here:
- Deriving master key from user password. We used the crypto_pwhash() function from Argon2id.
- Deriving 4 subkeys from master key. We used crypto_kdf_derive_from_key() which is from BLAKE2b KDF.
- Encrypting/Decrypting notes and settings. We used the XChaCha20-Poly1305AEAD primitive in the form of crypto_aead_xchacha20poly1305_ietf_encrypt/decrypt().
- Generating salts and nonces with CSPRNG randombytes_buf()
- Allocating, wiping, and freeing key material with sodium_malloc / sodium_free / sodium_memzero
- Secure key comparison (in tests) wiht sodium_memcmp() from Constant-time compare

The core architecture and flow is essentially this:
- User Password + 16-byte Random Salt
-> crypto_pwhash(Argon2id, MODERATE: 3 passes, 256MB RAM)
-> Master Key (32 bytes, in sodium_malloc's memory)
        ├─ KDF(ctx="BastionX", id=1) → k_notes 
        (encrypts note content)
        ├─ KDF(ctx="BastionX", id=2) → k_settings   (encrypts vault settings)
        ├─ KDF(ctx="BastionX", id=3) → k_verify     (password verification token)
        └─ KDF(ctx="BastionX", id=4) → k_database   (SQLCipher full-DB encryption)

Secure Memory Model (SecureBuffer<T>)
All keys live in RAII template SecureBuffer<unsigned char> which is aliased as SecureKey
- Allocation: sodium_malloc() - memory is locked and non-swappable, surrounded by guard pages
- Destruction: sodium_memzero() then sodium_free() for a guaranteed wipe
- Non-copyable: Copy constructor is deleted to prevent accidental duplication
- Keys are held as std::optional<SecureKey> in VaultService and .reset() on lock 

Encryption Flow:
Note {title, body, tags}
  → JSON serialize to bytes
  → Build AAD: note_id as 4-byte little-endian uint32_t
  → randombytes_buf(nonce, 24)  ← fresh random nonce per encrypt
  → crypto_aead_xchacha20poly1305_ietf_encrypt(plaintext, k_notes, nonce, AAD)
  → Store {nonce, ciphertext} in SQLCipher-encrypted database

The note ID in the AAD prevents ciphertext-swapping attacks. Settings use the same flow but with k_settings and empty AAD.

Password Verification:
No digital signatures, crypto_sign are used. Instead, a known plaintext token approach is taken:
- On vault creation - encrypt the marker with k_verify
- On unlock - attempt to decrypt that token with the newly derived k_verify
- If the Poly1305 MAC verifies -> password is correct

Defense in Depth Approach:
2 layers of encryption
1. Application-layer: XChaCha20-Poly1305 per note / per setting encryption
2. Database-layer: SQLCipher encrypts the entire .db file using k_database which SQLCipher further processes through its own PBKDF2-HMAC-SHA512 with 256k interations.

The PRAGMA cipher_memory_security = ON is set to tell SQLCipher to securely wipe its internal key material.

Password Change:
change_password() does a full re-key inside and exclusive SQLite transaction
1. Verify old password
2. Generate new salt, derive new master key + 4 subkeys
3. Decrypt every note with old key --> re-encrypt with new key (same AAD)
4. Re-encrypt verify token and settings
5. COMMIT (or ROLLBACK on error)
6. sqlite3_rekey() to re-encrypt the .db file
7. Write new salt to sidecar file 

Architecture:
- UI Layer (Qt): zero crypto knowledge
- VaultService: owns all SecureKey optionals and manages lifecycle
- CryptoService: static-only class, pure wrapper around libsodium
- SecureMemory: RAII template for sodium_malloc/free/memzero

CryptoService is stateless, it's a pure utility namespace around libsodium. The UI layer never touches keys or ciphertext.

