# sealed-message regression tests

These tests compile the actual product sources in a patched tdesktop checkout. They do not log in, send messages or open Tele. The storage facade deliberately substitutes Telegram's application and file-encryption APIs; it checks sealed-store parsing, persistence failures and identity isolation, not the real encryption backend or native application lifecycle. See [test-scope.json](test-scope.json).

## build and run

Use CMake 3.25+, a C++20 compiler, Qt 6 Core/Gui with PNG and JPEG support, and OpenSSL 3 development files. Set `TDESKTOP_SOURCE_DIR` to the checkout where the patch queue and encrypted-message feature have been applied. Point `CMAKE_PREFIX_PATH` at your Qt SDK if it is not already discoverable.

```
cmake -S tests/sealed -B /tmp/tele-sealed-tests -DTDESKTOP_SOURCE_DIR=/path/to/tdesktop -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build /tmp/tele-sealed-tests
ctest --test-dir /tmp/tele-sealed-tests --output-on-failure
```

Use a writable build path appropriate for your platform. With a multi-configuration generator, pass `--config Release` to the build and `-C Release` to CTest. On Windows, use an initialized MSVC terminal. `OPENSSL_RUNTIME_DLL` can identify the matching crypto DLL to copy beside the test binaries. `SEALED_QT_PLUGIN_DIR` sets the Qt plugin path for image tests if JPEG is not built into your Qt distribution. Standard CMake OpenSSL discovery variables apply.

If Python's `cryptography` package is installed at configure time, CMake also adds an independent interoperability test. Without that package, configuration prints that interoperability is skipped; the C++ tests still run. Install optional dependencies in your own environment, then reconfigure to include those checks.

## coverage

- `sealed_crypto`: selected official CFRG authenticated HPKE vectors plus real application envelopes, fixed and bucket padding, self notes, canonical parsing, malformed/tampered input, wrong keys/context and account-bound password-protected backups.
- `sealed_rng_failure`: a separate process deliberately configures an unavailable random generator, verifies randomness actually fails, then checks that identity generation, message/file encryption and backup export all refuse to continue.
- `sealed_files`: actual payload packing and bounded parsing, exact file roundtrips, filename handling, EXIF/GPS/orientation, PNG metadata removal, legacy document compatibility and authenticated image-presentation framing with strict PNG/chunk/CRC/decode bounds, normal-photo resizing and unchanged full-size document behavior. The JPEG fixture is embedded in the test.
- `sealed_store_facade`: actual sealed-key-store and directory-registry code with small fake storage/application objects. Includes corrupt/unavailable files, registration before private writes, write failures, passcode/lock gates, account ownership and contact-key replacement races.
- `sealed_registry`: actual bounded directory registry and shared exact-filename cleanup in temporary folders. Covers atomic registration before private writes, failed and partial writers, writer exclusion during native unprotection preflight, persistent cleanup retries, unrelated-file preservation, corrupt/busy state, default unused-profile behavior and unsafe links. Windows uses a temporary junction if symbolic-link creation is unavailable. It does not call the native logout or passcode APIs.
- `sealed_duress_store`: actual verifier and profile-state helper in temporary folders, including path bounds, mutation generations, deny-before-delete, failed cleanup, activation persistence, per-account recovery, and first-use permission for unused/new slots while previously wiped slots remain denied. It does not exercise the native passcode screen.
- `sealed_passcode_gate`: actual pure native-key observation policy, including normal-verdict precedence, absent/failed evidence, sticky failure in either observation order, unchanged inactive behavior and nested scope restoration. Real Argon2/scrypt failure injection, worker/main integration and warm/cold lock-screen testing remain pending.
- `sealed_signout_plan`: actual pure planner with fake orchestration, exact selected hash/date matching and local deadline decisions. No real RPC or session revocation is performed.
- `sealed_auto_mode`: actual automatic-mode parser and fingerprint-binding helper, including account/network/peer isolation, malformed-settings blocking, input bounds, changed or missing pins and identity-only recovery of own-sent copies. It does not exercise native key-cache lifetime, dialogs or Telegram transport.
- `sealed_send_plan`: actual send-proof helper, including exact account/chat/key/epoch binding, expiry and clock bounds, text/reply matching, content digests, uploaded file identifiers and canonical transport filename normalization after proof matching. Native RPC parsing, proof consumption, compose routing and lifecycle remain application checks.
- `sealed_rich_send_plan`: actual once-only multipart-release cursor and queue-capacity arithmetic. Covers 1 to 64 parts, cancellation, reentrant callbacks, all tail indices, group/part/byte budgets and integer extremes. Native sending and lifecycle remain application checks.
- `sealed_text` / `sealed_text_rng_failure`: actual rich-text/multipart codec and real HPKE engine. Covers canonical native tag serialization, Unicode boundaries, legacy short text,64-part groups, sender/recipient/Saved copies, private hash/length metadata, mutation/context/duplicate/incomplete rejection and isolated randomness failure. Native rendering, grouping and send integration need application testing.
- `sealed_view_cache`: actual bounded plaintext cache and publication-stamp helper, including independent byte ownership, count/byte limits, LRU removal, explicit clearing and stale cipher/context/key/epoch rejection. Native visibility, clearing of rendered text, selection and clipboard routes remain application checks.
- `sealed_codes_filter`: actual display filter, including stable replacements, Unicode surrounding text, matching limits, unchanged unrelated text and same-code handling across translations. This does not prove that every native view or copy route uses the filter.
- `sealed_code_mode_facade`: actual code-mode implementation with synthetic session/history/timer/UI facades, including policy persistence, model/translation mapping, expiry restoration, notifications and write failures. It does not validate native rendering, copy/search/accessibility coverage or the repair dialog; see [facade scope](code_mode_facade/README.md).
- `sealed_interop_fixture` and `sealed_python_interop`: C++ emits fresh envelopes/backups for publicly known fixture identities; an independent Python implementation decrypts them and checks the padding, role/context binding and payload.

The public [HPKE vectors](hpke-auth-vectors.json) come from the [CFRG HPKE test vectors](https://github.com/cfrg/draft-irtf-cfrg-hpke/blob/master/test-vectors.json), filtered to mode 2, KEM 18, KDF 3 and AEAD 2. The test identities are public fixtures and must never be used for real messages. [protocol-v1.json](protocol-v1.json) describes the application wire format.

The native Tele build and manual validation in [encrypted messages](../../docs/encrypted-messages.md#validation) are required separately. These tests do not validate a GUI, Telegram authorization, live transport, real local encrypted storage, platform passcode behavior, or an iOS app.

`sealed_jobs_plan` tests the actual six-slot CPU admission budget: three normal-only slots and three shared with search, reserved-first normal admission, search isolation, pool-specific release and invalid-state rejection. Native worker leases, main-thread release and wakeup delivery remain integration checks.

`sealed_search_plan` tests actual local-search policy and matching helpers: selected/private/account/network isolation, malformed-policy failure, candidate/byte bounds and case-folded NFKC substring matching including Russian. Native request parsing, query storage/scripts and search lifecycle need application validation.
