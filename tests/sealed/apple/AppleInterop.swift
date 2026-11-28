import Foundation
import CryptoKit
import Darwin
#if canImport(CommonCrypto)
import CommonCrypto
#endif

struct TestFailure: Error, CustomStringConvertible {
    let description: String
    init(_ description: String) { self.description = description }
}

final class Checks {
    private(set) var count = 0

    func require(_ condition: Bool, _ name: String) throws {
        count += 1
        if !condition { throw TestFailure(name) }
    }

    func rejects(_ name: String, _ operation: () throws -> Void) throws {
        var rejected = false
        do { try operation() } catch { rejected = true }
        try require(rejected, name)
    }
}

struct Encryption: Decodable {
    let aad: String
    let pt: String
    let ct: String
    let nonce: String
}

struct OfficialVector: Decodable {
    let mode: Int
    let kem_id: Int
    let kdf_id: Int
    let aead_id: Int
    let info: String
    let ikmR: String
    let ikmS: String
    let ikmE: String
    let skRm: String
    let skSm: String
    let skEm: String
    let pkRm: String
    let pkSm: String
    let pkEm: String
    let enc: String
    let shared_secret: String
    let key_schedule_context: String
    let secret: String
    let key: String
    let base_nonce: String
    let encryptions: [Encryption]
}

struct CppOutput: Decodable {
    let text: String
    let note: String
    let binary: String
    let backup: String
    let context: String
    let plaintext: String
}

struct Fixture: Decodable {
    let format: String
    let official_vector: OfficialVector
    let cpp_output: CppOutput
    let backup_passphrase: String
    let backup_context: String
    let text_token: String
    let public_token: String
    let fingerprint: String
}

func hex(_ text: String) throws -> Data {
    let bytes = Array(text.utf8)
    guard bytes.count <= 68 * 1024 * 1024, bytes.count % 2 == 0 else {
        throw TestFailure("Invalid fixture hex length")
    }
    func nibble(_ value: UInt8) throws -> UInt8 {
        if value >= 48 && value <= 57 { return value - 48 }
        if value >= 97 && value <= 102 { return value - 97 + 10 }
        throw TestFailure("Noncanonical fixture hex")
    }
    var output = Data()
    output.reserveCapacity(bytes.count / 2)
    for index in stride(from: 0, to: bytes.count, by: 2) {
        let high = try nibble(bytes[index])
        let low = try nibble(bytes[index + 1])
        output.append((high << 4) | low)
    }
    return output
}

func privateKey(_ bytes: Data) throws -> P521.KeyAgreement.PrivateKey {
    let order = try hex("01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409")
    guard bytes.count == 66, bytes.contains(where: { $0 != 0 }),
          bytes.lexicographicallyPrecedes(order) else {
        throw TestFailure("Invalid P521 private scalar")
    }
    let key = try P521.KeyAgreement.PrivateKey(rawRepresentation: bytes)
    guard key.rawRepresentation == bytes else { throw TestFailure("Noncanonical private key") }
    return key
}

func publicKey(_ bytes: Data) throws -> P521.KeyAgreement.PublicKey {
    let prime = Data([1] + Array(repeating: UInt8(255), count: 65))
    guard bytes.count == 133, bytes.first == 4,
          bytes.subdata(in: 1..<67).lexicographicallyPrecedes(prime),
          bytes.subdata(in: 67..<133).lexicographicallyPrecedes(prime) else {
        throw TestFailure("Invalid P521 point encoding")
    }
    let key = try P521.KeyAgreement.PublicKey(x963Representation: bytes)
    guard key.x963Representation == bytes else { throw TestFailure("Noncanonical public key") }
    return key
}

func dh(_ own: P521.KeyAgreement.PrivateKey, _ peer: Data) throws -> Data {
    let shared = try own.sharedSecretFromKeyAgreement(with: publicKey(peer))
    let bytes = shared.withUnsafeBytes { Data($0) }
    guard bytes.count == 66 else { throw TestFailure("ECDH must retain all 66 bytes") }
    return bytes
}

func kemSuite() -> Data { Data([0x4b, 0x45, 0x4d, 0, 0x12]) }
func hpkeSuite() -> Data { Data([0x48, 0x50, 0x4b, 0x45, 0, 0x12, 0, 3, 0, 2]) }

func extract(_ suite: Data, _ salt: Data, _ label: String, _ input: Data) -> Data {
    let labeled = Data("HPKE-v1".utf8) + suite + Data(label.utf8) + input
    let key = SymmetricKey(data: salt.isEmpty ? Data(repeating: 0, count: 64) : salt)
    return Data(HMAC<SHA512>.authenticationCode(for: labeled, using: key))
}

func expand(_ suite: Data, _ prk: Data, _ label: String, _ info: Data, _ count: Int) throws -> Data {
    guard count > 0 && count <= 255 * 64 else { throw TestFailure("Invalid HKDF output size") }
    let length = Data([UInt8((count >> 8) & 255), UInt8(count & 255)])
    let labeled = length + Data("HPKE-v1".utf8) + suite + Data(label.utf8) + info
    let key = HKDF<SHA512>.expand(pseudoRandomKey: prk, info: labeled, outputByteCount: count)
    return key.withUnsafeBytes { Data($0) }
}

func deriveKeyPair(_ ikm: Data) throws -> Data {
    let prk = extract(kemSuite(), Data(), "dkp_prk", ikm)
    for counter in 0..<256 {
        var candidate = try expand(kemSuite(), prk, "candidate", Data([UInt8(counter)]), 66)
        candidate[0] &= 1
        if (try? privateKey(candidate)) != nil { return candidate }
    }
    throw TestFailure("RFC DeriveKeyPair rejection limit")
}

func kemSecret(_ joinedDH: Data, _ enc: Data, _ recipient: Data, _ sender: Data) throws -> Data {
    let prk = extract(kemSuite(), Data(), "eae_prk", joinedDH)
    return try expand(kemSuite(), prk, "shared_secret", enc + recipient + sender, 64)
}

struct Schedule {
    let key: Data
    let nonce: Data
    let context: Data
    let secret: Data
}

func schedule(_ shared: Data, _ info: Data) throws -> Schedule {
    let context = Data([2]) + extract(hpkeSuite(), Data(), "psk_id_hash", Data())
        + extract(hpkeSuite(), Data(), "info_hash", info)
    let secret = extract(hpkeSuite(), shared, "secret", Data())
    return try Schedule(key: expand(hpkeSuite(), secret, "key", context, 32),
                        nonce: expand(hpkeSuite(), secret, "base_nonce", context, 12),
                        context: context, secret: secret)
}

func open(_ ciphertext: Data, _ key: Data, _ nonce: Data, _ aad: Data) throws -> Data {
    guard key.count == 32, nonce.count == 12, ciphertext.count >= 16 else {
        throw TestFailure("Invalid AES256GCM input")
    }
    let box = try AES.GCM.SealedBox(nonce: AES.GCM.Nonce(data: nonce),
                                  ciphertext: ciphertext.dropLast(16), tag: ciphertext.suffix(16))
    return try AES.GCM.open(box, using: SymmetricKey(data: key), authenticating: aad)
}

func hpkeOpen(_ own: P521.KeyAgreement.PrivateKey, _ sender: Data, _ enc: Data,
              _ info: Data, _ aad: Data, _ ciphertext: Data) throws -> Data {
    let joined = try dh(own, enc) + dh(own, sender)
    let shared = try kemSecret(joined, enc, own.publicKey.x963Representation, sender)
    let keys = try schedule(shared, info)
    return try open(ciphertext, keys.key, keys.nonce, aad)
}

func binding(_ label: String, _ role: UInt8, _ context: Data, _ header: Data) throws -> Data {
    guard context.count > 0 && context.count <= 256 else { throw TestFailure("Invalid caller context") }
    return Data(label.utf8) + Data([role, UInt8((context.count >> 8) & 255), UInt8(context.count & 255)])
        + context + header
}

func unpad(_ padded: Data, _ version: UInt8) throws -> Data {
    guard padded.count >= 4, version == 1 || version == 2 else { throw TestFailure("Invalid padded payload") }
    let bytes = Array(padded)
    let length = bytes.prefix(4).reduce(UInt32(0)) { ($0 << 8) | UInt32($1) }
    let limit = version == 1 ? 1024 : 16 * 1024 * 1024
    guard length > 0, Int(length) <= limit, Int(length) + 4 <= bytes.count else {
        throw TestFailure("Invalid authenticated inner length")
    }
    let expected = version == 1 ? 1028 : ((Int(length) + 4 + 65535) / 65536) * 65536
    guard padded.count == expected, bytes.dropFirst(Int(length) + 4).allSatisfy({ $0 == 0 }) else {
        throw TestFailure("Noncanonical authenticated padding")
    }
    return padded.subdata(in: 4..<(Int(length) + 4))
}

func validEnvelope(_ envelope: Data, _ version: UInt8) throws {
    guard version == 1 || version == 2, envelope.first == version,
          envelope.count >= 267, (envelope.count - 267) % 2 == 0 else {
        throw TestFailure("Invalid envelope header")
    }
    let padded = (envelope.count - 267) / 2 - 16
    if version == 1 {
        guard padded == 1028 else { throw TestFailure("Invalid fixed chat size") }
    } else {
        guard padded >= 65536, padded <= 257 * 65536, padded % 65536 == 0 else {
            throw TestFailure("Invalid file bucket size")
        }
    }
}

func unseal(_ envelope: Data, _ own: P521.KeyAgreement.PrivateKey,
            _ sender: Data, _ context: Data, _ version: UInt8) throws -> Data {
    try validEnvelope(envelope, version)
    _ = try publicKey(sender)
    let role: UInt8 = own.publicKey.x963Representation == sender ? 1 : 0
    let header = envelope.subdata(in: 0..<267)
    let offset = 1 + Int(role) * 133
    let enc = header.subdata(in: offset..<(offset + 133))
    let recordSize = (envelope.count - 267) / 2
    let start = 267 + Int(role) * recordSize
    let plain = try hpkeOpen(own, sender, enc,
        binding("tele sealed hpke v1", role, context, header),
        binding("tele sealed aad v1", role, context, header),
        envelope.subdata(in: start..<(start + recordSize)))
    return try unpad(plain, version)
}

func token(_ prefix: String, _ bytes: Data) -> String {
    prefix + bytes.base64EncodedString().replacingOccurrences(of: "+", with: "-")
        .replacingOccurrences(of: "/", with: "_").replacingOccurrences(of: "=", with: "")
}

func parseToken(_ prefix: String, _ text: String) throws -> Data {
    guard text.utf8.count <= 4096, text.hasPrefix(prefix) else { throw TestFailure("Invalid token prefix or size") }
    let encoded = String(text.dropFirst(prefix.count))
    guard !encoded.isEmpty, encoded.utf8.allSatisfy({
        ($0 >= 65 && $0 <= 90) || ($0 >= 97 && $0 <= 122)
            || ($0 >= 48 && $0 <= 57) || $0 == 45 || $0 == 95
    }) else { throw TestFailure("Invalid token alphabet") }
    var standard = encoded.replacingOccurrences(of: "-", with: "+").replacingOccurrences(of: "_", with: "/")
    standard += String(repeating: "=", count: (4 - standard.utf8.count % 4) % 4)
    guard let bytes = Data(base64Encoded: standard), token(prefix, bytes) == text else {
        throw TestFailure("Noncanonical base64url token")
    }
    return bytes
}

func backupKey(_ password: String, _ salt: Data) throws -> Data {
    #if canImport(CommonCrypto)
    let bytes = Data(password.utf8)
    guard password.unicodeScalars.count >= 16, bytes.count <= 1024, salt.count == 16 else {
        throw TestFailure("Invalid backup password or salt")
    }
    var output = Data(repeating: 0, count: 32)
    let result = bytes.withUnsafeBytes { (passwordBytes: UnsafeRawBufferPointer) in
        salt.withUnsafeBytes { (saltBytes: UnsafeRawBufferPointer) in
            output.withUnsafeMutableBytes { (keyBytes: UnsafeMutableRawBufferPointer) in
                CCKeyDerivationPBKDF(CCPBKDFAlgorithm(kCCPBKDF2),
                    passwordBytes.baseAddress?.assumingMemoryBound(to: CChar.self), bytes.count,
                    saltBytes.baseAddress?.assumingMemoryBound(to: UInt8.self), salt.count,
                    CCPseudoRandomAlgorithm(kCCPRFHmacAlgSHA256), 600000,
                    keyBytes.baseAddress?.assumingMemoryBound(to: UInt8.self), 32)
            }
        }
    }
    guard result == 0 else { throw TestFailure("CommonCrypto PBKDF2 failed") }
    return output
    #else
    throw TestFailure("CommonCrypto is unavailable; backup interoperability remains unverified")
    #endif
}

func restoreBackup(_ backup: Data, _ password: String, _ context: Data) throws -> Data {
    let magic = Data("tele-sealed-key-backup:v1".utf8)
    guard backup.count == 135, backup.prefix(magic.count) == magic else {
        throw TestFailure("Invalid backup format")
    }
    let salt = backup.subdata(in: 25..<41)
    let nonce = backup.subdata(in: 41..<53)
    let header = backup.subdata(in: 0..<53)
    let key = try backupKey(password, salt)
    let secret = try open(backup.subdata(in: 53..<135), key, nonce,
                          binding("tele sealed backup aad v1", 0, context, header))
    _ = try privateKey(secret)
    return secret
}

func officialTests(_ vector: OfficialVector, _ checks: Checks) throws {
    try checks.require(vector.mode == 2 && vector.kem_id == 18 && vector.kdf_id == 3 && vector.aead_id == 2,
                       "Exact P521/SHA512/AES256GCM Auth suite")
    let recipient = try privateKey(hex(vector.skRm))
    let sender = try privateKey(hex(vector.skSm))
    let ephemeral = try privateKey(hex(vector.skEm))
    try checks.require(recipient.publicKey.x963Representation == hex(vector.pkRm), "RFC recipient public derivation")
    try checks.require(sender.publicKey.x963Representation == hex(vector.pkSm), "RFC sender public derivation")
    try checks.require(ephemeral.publicKey.x963Representation == hex(vector.enc), "RFC ephemeral public derivation")
    try checks.require(hex(vector.pkEm) == hex(vector.enc), "RFC serialized ephemeral key")
    try checks.require(deriveKeyPair(hex(vector.ikmR)) == hex(vector.skRm), "RFC recipient DeriveKeyPair")
    try checks.require(deriveKeyPair(hex(vector.ikmS)) == hex(vector.skSm), "RFC sender DeriveKeyPair")
    try checks.require(deriveKeyPair(hex(vector.ikmE)) == hex(vector.skEm), "RFC ephemeral DeriveKeyPair")
    let enc = try hex(vector.enc)
    let recipientPublic = try hex(vector.pkRm)
    let senderPublic = try hex(vector.pkSm)
    let joined = try dh(ephemeral, recipientPublic) + dh(sender, recipientPublic)
    let shared = try kemSecret(joined, enc, recipientPublic, senderPublic)
    try checks.require(shared == hex(vector.shared_secret), "RFC Auth KEM secret")
    let keys = try schedule(shared, hex(vector.info))
    try checks.require(keys.key == hex(vector.key), "RFC AES key")
    try checks.require(keys.nonce == hex(vector.base_nonce), "RFC base nonce")
    try checks.require(keys.context == hex(vector.key_schedule_context), "RFC key schedule context")
    try checks.require(keys.secret == hex(vector.secret), "RFC schedule secret")
    try checks.require(vector.encryptions.count == 257, "Full official encryption sequence fixture")
    for (sequence, encryption) in vector.encryptions.enumerated() {
        var nonce = Array(keys.nonce)
        for byte in 0..<8 { nonce[11 - byte] ^= UInt8(truncatingIfNeeded: UInt64(sequence) >> (8 * byte)) }
        let nonceData = Data(nonce)
        let ciphertext = try hex(encryption.ct)
        let aad = try hex(encryption.aad)
        let plain = try hex(encryption.pt)
        try checks.require(nonceData == hex(encryption.nonce), "RFC sequence nonce")
        try checks.require(open(ciphertext, keys.key, nonceData, aad) == plain, "RFC CryptoKit AES256GCM open")
        let sealed = try AES.GCM.seal(plain, using: SymmetricKey(data: keys.key),
                                     nonce: AES.GCM.Nonce(data: nonceData), authenticating: aad)
        try checks.require(sealed.ciphertext + sealed.tag == ciphertext, "RFC CryptoKit AES256GCM seal")
        if sequence == 0 {
            try checks.require(hpkeOpen(recipient, senderPublic, enc, hex(vector.info), aad, ciphertext) == plain,
                               "RFC authenticated HPKE open")
        }
    }
}

func applicationTests(_ fixture: Fixture, _ checks: Checks) throws {
    let vector = fixture.official_vector
    let output = fixture.cpp_output
    let sender = try privateKey(hex(vector.skSm))
    let recipient = try privateKey(hex(vector.skRm))
    let stranger = try privateKey(hex(vector.skEm))
    let senderPublic = try hex(vector.pkSm)
    let recipientPublic = try hex(vector.pkRm)
    let context = try hex(output.context)
    let plaintext = try hex(output.plaintext)
    let text = try hex(output.text)
    let file = try hex(output.binary)
    let note = try hex(output.note)
    try checks.require(text.count == 2355 && fixture.text_token.utf8.count == 3155, "Fixed P521 text wire size")
    for (envelope, version) in [(text, UInt8(1)), (file, UInt8(2))] {
        try checks.require(unseal(envelope, recipient, senderPublic, context, version) == plaintext, "C++ recipient copy opens in CryptoKit")
        try checks.require(unseal(envelope, sender, senderPublic, context, version) == plaintext, "C++ sender copy opens in CryptoKit")
        try checks.rejects("Wrong caller context rejects") {
            _ = try unseal(envelope, recipient, senderPublic, context + Data([0]), version)
        }
        try checks.rejects("Untrusted sender rejects") {
            _ = try unseal(envelope, recipient, recipientPublic, context, version)
        }
        try checks.rejects("Unrelated private key rejects") {
            _ = try unseal(envelope, stranger, senderPublic, context, version)
        }
    }
    try checks.require(unseal(note, sender, senderPublic, context, 1) == plaintext, "C++ Saved Messages self note opens")
    try checks.rejects("File cannot be read as text") { _ = try unseal(file, recipient, senderPublic, context, 1) }
    try checks.rejects("Text cannot be read as file") { _ = try unseal(text, recipient, senderPublic, context, 2) }
    for offset in [0, 1, 133, 134, 266, 267, 1310] {
        var modified = text
        modified[offset] ^= 1
        try checks.rejects("Selected peer record or common header mutation rejects") {
            _ = try unseal(modified, recipient, senderPublic, context, 1)
        }
    }
    var modifiedSelf = text
    modifiedSelf[1311] ^= 1
    try checks.rejects("Selected self record mutation rejects") {
        _ = try unseal(modifiedSelf, sender, senderPublic, context, 1)
    }
    try checks.require(parseToken("tele-sealed:v1:", fixture.text_token) == text, "Canonical text token decodes")
    try checks.require(parseToken("tele-key:v1:", fixture.public_token) == senderPublic, "Canonical public token decodes")
    _ = try publicKey(parseToken("tele-key:v1:", fixture.public_token))
    for suffix in ["=", "\n", "!", " "] {
        try checks.rejects("Noncanonical base64url suffix rejects") {
            _ = try parseToken("tele-key:v1:", fixture.public_token + suffix)
        }
    }
    try checks.rejects("Noncanonical unused base64 bits reject") { _ = try parseToken("x:", "x:AB") }
    let digest = Data(SHA512.hash(data: Data("tele sealed fingerprint v1".utf8) + senderPublic))
    let hexDigest = digest.map { String(format: "%02X", $0) }.joined()
    let fingerprint = stride(from: 0, to: hexDigest.count, by: 4).map { offset -> String in
        let start = hexDigest.index(hexDigest.startIndex, offsetBy: offset)
        return String(hexDigest[start..<hexDigest.index(start, offsetBy: 4)])
    }.joined(separator: " ")
    try checks.require(fingerprint == fixture.fingerprint, "Full SHA512 fingerprint agrees")
    let backup = try hex(output.backup)
    let backupContext = try hex(fixture.backup_context)
    try checks.require(restoreBackup(backup, fixture.backup_passphrase, backupContext) == hex(vector.skSm),
                       "C++ encrypted backup restores the same portable P521 key")
    try checks.rejects("Wrong backup password rejects") {
        _ = try restoreBackup(backup, fixture.backup_passphrase + "wrong", backupContext)
    }
    try checks.rejects("Wrong backup account/network context rejects") {
        _ = try restoreBackup(backup, fixture.backup_passphrase, backupContext + Data([0]))
    }
    var brokenBackup = backup
    brokenBackup[134] ^= 1
    try checks.rejects("Backup authentication tag mutation rejects") {
        _ = try restoreBackup(brokenBackup, fixture.backup_passphrase, backupContext)
    }
    try checks.rejects("Backup trailing bytes reject") {
        _ = try restoreBackup(backup + Data([0]), fixture.backup_passphrase, backupContext)
    }
    try checks.rejects("Zero scalar rejects") { _ = try privateKey(Data(repeating: 0, count: 66)) }
    try checks.rejects("Overlong scalar rejects") { _ = try privateKey(Data([0]) + sender.rawRepresentation) }
    try checks.rejects("High scalar rejects") { _ = try privateKey(Data(repeating: 255, count: 66)) }
    try checks.rejects("Curve-order scalar rejects") {
        _ = try privateKey(hex("01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409"))
    }
    try checks.rejects("Short scalar rejects") { _ = try privateKey(Data(sender.rawRepresentation.dropFirst())) }
    try checks.rejects("Infinity or short point rejects") { _ = try publicKey(Data([0])) }
    var compressed = Data(senderPublic.prefix(67))
    compressed[0] = 2
    try checks.rejects("Compressed point rejects") { _ = try publicKey(compressed) }
    var hybrid = senderPublic
    hybrid[0] = 6
    try checks.rejects("Hybrid point rejects") { _ = try publicKey(hybrid) }
    var offCurve = senderPublic
    offCurve[132] ^= 1
    try checks.rejects("Off-curve point rejects") { _ = try publicKey(offCurve) }
    var outOfRange = senderPublic
    outOfRange[1] = 2
    try checks.rejects("Out-of-range coordinate rejects") { _ = try publicKey(outOfRange) }
    var badPadding = Data([0, 0, 0, 1, 42]) + Data(repeating: 0, count: 1023)
    try checks.require(unpad(badPadding, 1) == Data([42]), "Canonical padding decodes")
    badPadding[1027] = 1
    try checks.rejects("Nonzero authenticated padding rejects") { _ = try unpad(badPadding, 1) }
}

do {
    guard CommandLine.arguments.count == 2 else { throw TestFailure("Usage: swift AppleInterop.swift PublicFixture.json") }
    let url = URL(fileURLWithPath: CommandLine.arguments[1])
    let attributes = try FileManager.default.attributesOfItem(atPath: url.path)
    guard let size = attributes[.size] as? NSNumber, size.intValue > 0, size.intValue <= 2 * 1024 * 1024 else {
        throw TestFailure("Fixture must be at most 2 MiB")
    }
    let fixture = try JSONDecoder().decode(Fixture.self, from: Data(contentsOf: url))
    guard fixture.format == "tele-sealed-apple-interop:v1" else { throw TestFailure("Unknown fixture format") }
    let checks = Checks()
    try officialTests(fixture.official_vector, checks)
    try applicationTests(fixture, checks)
    print("Apple CryptoKit interoperability passed: \(checks.count) checks. Public fixture only; no account or network access.")
} catch {
    FileHandle.standardError.write(Data("FAILED: \(error)\n".utf8))
    exit(1)
}
