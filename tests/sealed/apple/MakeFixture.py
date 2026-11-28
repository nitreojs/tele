import argparse
import base64
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--cpp-output', type=Path, required=True)
parser.add_argument('--vectors', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--crypto-source', type=Path)
args = parser.parse_args()

cpp = json.loads(args.cpp_output.read_text(encoding='utf-8'))
vectors = json.loads(args.vectors.read_text(encoding='utf-8'))
matching = [item for item in vectors['vectors'] if
            (item['mode'], item['kem_id'], item['kdf_id'], item['aead_id']) == (2, 18, 3, 2)]
if len(matching) != 1 or len(matching[0]['encryptions']) != 257:
    raise ValueError('Expected one complete official P521 Auth suite vector')
vector = matching[0]
required = {'text', 'note', 'binary', 'backup', 'context', 'plaintext'}
if set(cpp) != required:
    raise ValueError('Unexpected C++ fixture fields')
decoded = {name: bytes.fromhex(value) for name, value in cpp.items()}
if any(decoded[name].hex() != cpp[name] for name in cpp):
    raise ValueError('C++ fixture hex is not canonical')
if len(decoded['text']) != 2355 or len(decoded['note']) != 2355 or len(decoded['backup']) != 135:
    raise ValueError('C++ fixture does not use the expected P521 protocol sizes')
if decoded['text'][0] != 1 or decoded['note'][0] != 1 or decoded['binary'][0] != 2:
    raise ValueError('Unexpected C++ payload versions')

def token(prefix, value):
    return prefix + base64.urlsafe_b64encode(value).decode('ascii').rstrip('=')

def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=True).encode('utf-8')

public = bytes.fromhex(vector['pkSm'])
digest = hashlib.sha512(b'tele sealed fingerprint v1' + public).hexdigest().upper()
provenance = {
    'generated_utc': datetime.now(timezone.utc).isoformat(),
    'official_vector_source': vectors['source'],
    'official_vector_sha256': hashlib.sha256(canonical(vector)).hexdigest(),
    'cpp_output_sha256': hashlib.sha256(canonical(cpp)).hexdigest(),
    'account_access': False,
    'network_access': False,
    'apple_execution': 'not run; requires user Mac with Xcode',
    'identities': 'Public RFC fixture identities only; never use for real messages or backups.'
}
if args.crypto_source:
    provenance['cpp_crypto_source_sha256'] = hashlib.sha256(args.crypto_source.read_bytes()).hexdigest()

result = {
    'format': 'tele-sealed-apple-interop:v1',
    'provenance': provenance,
    'official_vector': vector,
    'cpp_output': cpp,
    'backup_passphrase': 'This is a test-only backup passphrase 12345',
    'backup_context': b'prod:1234567'.hex(),
    'text_token': token('tele-sealed:v1:', decoded['text']),
    'public_token': token('tele-key:v1:', public),
    'fingerprint': ' '.join(digest[index:index + 4] for index in range(0, len(digest), 4))
}
args.output.write_bytes((json.dumps(result, indent=2, ensure_ascii=True) + '\n').replace('\n', '\r\n').encode('utf-8'))
print(f'Wrote public Apple fixture: {args.output.name}; Apple execution remains unverified.')
