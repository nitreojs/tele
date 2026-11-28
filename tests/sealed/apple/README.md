# Apple interoperability check

Status: prepared on Windows; this Swift file has not been compiled or executed on Apple hardware. A successful run on the user's Mac is still required. This is an independent protocol test, not an iOS app or a production Apple encryption implementation.

The included `PublicFixture.json` contains freshly generated desktop C++ ciphertext and encrypted-backup output, together with one complete official public RFC 9180 test vector. Every private key, plaintext and backup password here is intentionally public test data. No account, Telegram login, Keychain entry, certificate, network request or real session is used. Never replace these fixtures with real secrets.

## Run on the Mac

Use macOS 11 or newer with Xcode or its command line tools. Copy this folder to the Mac, open Terminal in it, and run:

```sh
xcrun swift AppleInterop.swift PublicFixture.json
```

The equivalent command when `swift` is already on your path is:

```sh
swift AppleInterop.swift PublicFixture.json
```

No Swift package, app signing, account credentials or desktop build is required. Keep all files unchanged for the first run. Send back the final output line, or the compiler/error output if it fails. Success prints `Apple CryptoKit interoperability passed: ... checks` and exits with status 0. A failed check exits with status 1. An unavailable CommonCrypto module is an explicit failure, so backup tests cannot silently be skipped.

## What it checks

- The exact HPKE Auth suite: DHKEM(P-521, HKDF-SHA512), HKDF-SHA512 and AES256-GCM, identifiers `0x0012 / 0x0003 / 0x0002`, mode 2.
- Apple's 66-byte raw private scalar and 133-byte uncompressed X9.63 public point against the official vector; RFC DeriveKeyPair rejection sampling; fixed-width ECDH output; KEM and key-schedule values; all 257 official AES-GCM sequence vectors in both directions.
- Desktop output decrypted through independent Apple code: recipient and sender text/file copies, a Saved Messages self copy, SHA512 fingerprints, and a portable account-bound encrypted private-key backup.
- Canonical base64url, strict point/scalar formats, wrong sender/private key/context, cross-version text/file use, selected ciphertext/header changes, backup tampering/password/context, and canonical authenticated padding.

The AES-GCM tag is 128 bits. This test does not establish forward secrecy, hidden metadata, forensic erasure, iOS UI behavior, Apple Keychain integration, real account storage, multi-device enrollment or a complete security audit. Saved Messages and shared backups still require an actual Apple client implementation and its own native tests.

## Rebuild the public fixture

From `tests/sealed`, after building the standalone C++ harness, run:

```sh
<test-build>/sealed_crypto_test --fixtures hpke-auth-vectors.json /tmp/tele-apple-cpp-output.json
python3 apple/MakeFixture.py --cpp-output /tmp/tele-apple-cpp-output.json --vectors hpke-auth-vectors.json --output apple/PublicFixture.json --crypto-source <patched-tdesktop>/Telegram/SourceFiles/tele/tele_sealed_crypto.cpp
```

On Windows the harness executable is `sealed_crypto_test.exe`; use a temporary output path appropriate to that machine. `MakeFixture.py` adds public-vector provenance, canonical tokens and the known test backup password. The ciphertext fields come directly from the C++ harness. It never generates or uses real account keys. The provenance field records the C++ source hash when supplied, and continues to say Apple execution is unverified until a separate actual Mac run is reported.

The fixture's synthetic caller contexts are opaque protocol test bytes. They do not validate the desktop app's real account-ID construction or a future iOS client's account binding.

## Primary API references

- [Apple P521 key agreement private key](https://developer.apple.com/documentation/cryptokit/p521/keyagreement/privatekey.md), [public key and X9.63 representation](https://developer.apple.com/documentation/cryptokit/p521/keyagreement/publickey.md), and [shared secret bytes](https://developer.apple.com/documentation/cryptokit/sharedsecret.md).
- [Apple HMAC](https://developer.apple.com/documentation/cryptokit/hmac.md), [HKDF expansion](https://developer.apple.com/documentation/cryptokit/hkdf/expand(pseudorandomkey:info:outputbytecount:).md), [AES-GCM sealed box](https://developer.apple.com/documentation/cryptokit/aes/gcm/sealedbox.md), and [authenticated opening](https://developer.apple.com/documentation/cryptokit/aes/gcm/open(_:using:authenticating:).md).
- [Apple CommonCrypto PBKDF2 API header](https://github.com/apple-oss-distributions/CommonCrypto/blob/main/include/CommonKeyDerivation.h), [RFC 9180](https://www.rfc-editor.org/rfc/rfc9180.html), and [official public CFRG test vectors](https://github.com/cfrg/draft-irtf-cfrg-hpke/blob/master/test-vectors.json).
