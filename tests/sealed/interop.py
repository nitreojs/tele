from pathlib import Path
import argparse
import hashlib
import hmac
import json
from cryptography.exceptions import InvalidTag
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.kdf.hkdf import HKDFExpand
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.ciphers.aead import AESGCM

parser = argparse.ArgumentParser()
parser.add_argument('--vectors', type=Path, required=True)
parser.add_argument('--fixture', type=Path, required=True)
args = parser.parse_args()
vector = json.loads(args.vectors.read_text(encoding='utf-8'))['vectors'][0]
fixture = json.loads(args.fixture.read_text(encoding='utf-8'))
kem_suite = b'KEM' + bytes.fromhex('0012')
hpke_suite = b'HPKE' + bytes.fromhex('001200030002')

def extract(suite, salt, label, ikm):
    return hmac.new(salt, b'HPKE-v1' + suite + label + ikm, hashlib.sha512).digest()

def expand(suite, prk, label, info, length):
    label_info = length.to_bytes(2, 'big') + b'HPKE-v1' + suite + label + info
    return HKDFExpand(algorithm=hashes.SHA512(), length=length, info=label_info).derive(prk)

def public(private):
    return private.public_key().public_bytes(serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)

def hpke_open(private, sender_public, enc, info, aad, ciphertext):
    dh = private.exchange(ec.ECDH(), ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP521R1(), enc))
    dh += private.exchange(ec.ECDH(), ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP521R1(), sender_public))
    prk = extract(kem_suite, b'', b'eae_prk', dh)
    shared = expand(kem_suite, prk, b'shared_secret', enc + public(private) + sender_public, 64)
    context = b'\x02' + extract(hpke_suite, b'', b'psk_id_hash', b'')
    context += extract(hpke_suite, b'', b'info_hash', info)
    secret = extract(hpke_suite, shared, b'secret', b'')
    key = expand(hpke_suite, secret, b'key', context, 32)
    nonce = expand(hpke_suite, secret, b'base_nonce', context, 12)
    return AESGCM(key).decrypt(nonce, ciphertext, aad)

def binding(label, role, context, header):
    return label + bytes([role]) + len(context).to_bytes(2, 'big') + context + header

def unseal(envelope, private, expected_sender, context, version):
    assert envelope[0] == version
    header = envelope[:267]
    role = int(public(private) == expected_sender)
    enc = header[1 + role * 133 : 134 + role * 133]
    record_size = (len(envelope) - 267) // 2
    ciphertext = envelope[267 + role * record_size : 267 + (role + 1) * record_size]
    padded = hpke_open(private, expected_sender, enc,
                       binding(b'tele sealed hpke v1', role, context, header),
                       binding(b'tele sealed aad v1', role, context, header), ciphertext)
    length = int.from_bytes(padded[:4], 'big')
    assert length > 0
    assert not any(padded[4 + length:])
    if version == 1:
        assert len(padded) == 1028 and length <= 1024
    else:
        assert len(padded) % 65536 == 0 and length <= 16777216
    return padded[4:4 + length]

sender = ec.derive_private_key(int(vector['skSm'], 16), ec.SECP521R1())
recipient = ec.derive_private_key(int(vector['skRm'], 16), ec.SECP521R1())
sender_public = bytes.fromhex(vector['pkSm'])
context = bytes.fromhex(fixture['context'])
expected = bytes.fromhex(fixture['plaintext'])

for field, version in [('text', 1), ('binary', 2)]:
    envelope = bytes.fromhex(fixture[field])
    assert unseal(envelope, recipient, sender_public, context, version) == expected
    assert unseal(envelope, sender, sender_public, context, version) == expected
    try:
        unseal(envelope, recipient, sender_public, context + b'wrong', version)
        raise AssertionError('Unexpected successful decryption with wrong context')
    except InvalidTag:
        pass
assert unseal(bytes.fromhex(fixture['note']), sender, sender_public, context, 1) == expected

backup = bytes.fromhex(fixture['backup'])
magic = b'tele-sealed-key-backup:v1'
assert backup.startswith(magic) and len(backup) == len(magic) + 16 + 12 + 66 + 16
salt = backup[len(magic):len(magic)+16]
nonce = backup[len(magic)+16:len(magic)+28]
header = backup[:len(magic)+28]
key = PBKDF2HMAC(algorithm=hashes.SHA256(), length=32, salt=salt, iterations=600000).derive(
    b'This is a test-only backup passphrase 12345')
aad = binding(b'tele sealed backup aad v1', 0, b'prod:1234567', header)
assert AESGCM(key).decrypt(nonce, backup[len(header):], aad) == bytes.fromhex(vector['skSm'])
print('Independent Python interoperability passed: recipient/sender text, files, self notes, wrong-context rejection and account-bound encrypted backup.')
