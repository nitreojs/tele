# Outgoing history after identity-only recovery

Status: the native key-selection change has been reviewed, but this GUI flow has not yet been exercised in a running Tele build. The standalone crypto and storage tests do not execute `SealedKeys` or the native message/file actions. A substitute UI implementation would not verify this regression.

Use isolated development profiles and accounts you control. Do not delete or replace a real key store to perform this check.

1. Enable encrypted messages and a local app passcode. In a private test conversation, verify the other account's public key, then send an encrypted text and file. Also create an encrypted Saved Messages note and file.
2. Export the sender's encrypted identity backup, keep it off this device, and record the sender's public-key fingerprint.
3. Sign into the same sender account in an empty test profile, set its local passcode, and restore only that encrypted identity backup. Confirm the fingerprint matches. Leave the peer-key list empty.
4. Open the earlier outgoing text and file. Both should decrypt using the restored sender self-copy without asking for the peer's key. The Saved Messages note and file should also open.
5. An incoming text/file from the other account must still require that account's separately verified public key. A new send to that account must also remain blocked until its key is verified.
6. In another empty test profile with no restored identity, reading an old message/file must ask for recovery without opening the new-key creation dialog or creating keys. Explicit sending may open the dialog, but creation still requires its button.
7. Confirm forwarded-message rejection, wrong sender/account/network context rejection, duress quarantine, account switching and app locking still block access as before. With encrypted messages off, the encrypted actions should disappear.

Record the development-build revision and actual results before marking this native recovery flow tested. Do not count the existing sender-copy cryptography roundtrip as a native key-selection test.
