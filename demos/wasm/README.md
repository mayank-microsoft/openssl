WebAssembly Demo
================

This directory contains example code demonstrating how to use OpenSSL in WebAssembly applications using the WASI (WebAssembly System Interface) platform.

Files
-----

- `crypto_example.c` - A simple example demonstrating SHA-256 hashing and AES-256-CBC encryption/decryption

Prerequisites
-------------

1. **WASI SDK**: Download from https://github.com/WebAssembly/wasi-sdk/releases
2. **OpenSSL built for wasm32-wasip1**: Use the provided build script in the root directory
3. **WASM runtime** (one of the following):
   - Wasmtime: https://wasmtime.dev/
   - Wasmer: https://wasmer.io/
   - Node.js with WASI support (v12+)

Building OpenSSL for WASM
-------------------------

First, build OpenSSL for the wasm32-wasip1 target:

```bash
cd ../..  # Go to OpenSSL root directory
./build-wasm32-wasip1.sh /tmp/openssl-wasm
```

This will build and install OpenSSL to `/tmp/openssl-wasm`.

Building the Demo
-----------------

Once OpenSSL is built, compile the demo:

```bash
wasm32-wasip1-clang -O3 \
  -I/tmp/openssl-wasm/include \
  -L/tmp/openssl-wasm/lib \
  -o crypto_example.wasm \
  crypto_example.c \
  -lssl -lcrypto
```

Or if using the alternative compiler name:

```bash
wasm32-wasi-clang -O3 \
  -I/tmp/openssl-wasm/include \
  -L/tmp/openssl-wasm/lib \
  -o crypto_example.wasm \
  crypto_example.c \
  -lssl -lcrypto
```

Running the Demo
----------------

### Using Wasmtime

```bash
wasmtime crypto_example.wasm
```

### Using Wasmer

```bash
wasmer run crypto_example.wasm
```

### Using Node.js

Create a file `run.js`:

```javascript
const { readFileSync } = require('fs');
const { WASI } = require('wasi');

const wasi = new WASI({
  args: process.argv,
  env: process.env,
  preopens: {
    '/': '/'
  }
});

const importObject = { wasi_snapshot_preview1: wasi.wasiImport };

(async () => {
  const wasm = await WebAssembly.compile(
    readFileSync('./crypto_example.wasm')
  );
  const instance = await WebAssembly.instantiate(wasm, importObject);
  wasi.start(instance);
})();
```

Then run:

```bash
node run.js
```

Expected Output
---------------

The demo should output something like:

```
OpenSSL WebAssembly Demo
========================
OpenSSL version: OpenSSL 4.0.0-dev

=== SHA-256 Example ===
Message: Hello, WebAssembly!
SHA-256 Hash: 4f3d5f6a8b9c1e2d3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f

=== AES-256-CBC Example ===
Plaintext: This is a secret message for WASM!
Key: 1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b
IV: 9a8b7c6d5e4f3a2b1c0d9e8f7a6b5c4d
Ciphertext: a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2
Decrypted: This is a secret message for WASM!
Encryption/Decryption successful!

All examples completed successfully!
```

Notes
-----

- The actual hash values and ciphertext will vary since random keys and IVs are generated
- This example uses only basic cryptographic operations that don't require threading or system-specific features
- For production use, ensure proper key management and security practices

Troubleshooting
---------------

### "wasm32-wasip1-clang: command not found"

Make sure the WASI SDK bin directory is in your PATH:

```bash
export WASI_SDK_PATH=/path/to/wasi-sdk
export PATH=$WASI_SDK_PATH/bin:$PATH
```

### "cannot find -lssl" or "cannot find -lcrypto"

Ensure OpenSSL is built and installed, and the `-L` flag points to the correct library directory.

### Runtime errors

Some WASM runtimes have different capabilities. If you encounter errors:
1. Try a different runtime (Wasmtime is generally most compatible)
2. Check that the runtime supports the WASI features required
3. Ensure sufficient memory is allocated

Further Information
-------------------

- See [NOTES-WASM.md](../../NOTES-WASM.md) for detailed information about building OpenSSL for WebAssembly
- For more OpenSSL examples, see the other demo directories
- For WASI documentation, visit https://wasi.dev/
