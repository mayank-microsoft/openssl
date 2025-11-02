NOTES FOR THE WEBASSEMBLY (WASM) PLATFORMS
===========================================

WebAssembly (WASM) is a binary instruction format for a stack-based virtual
machine. WASI (WebAssembly System Interface) is a system interface for
WebAssembly that provides a standardized API for accessing operating system
features.

Targets
-------

OpenSSL supports the following WebAssembly targets:

 - `wasm32-wasip1` - WebAssembly 32-bit target using WASI Preview 1
 - `wasm32-wasi` - Alternative name for wasm32-wasip1

Prerequisites
-------------

To build OpenSSL for WebAssembly, you need the WASI SDK:

1. Download and install WASI SDK from:
   https://github.com/WebAssembly/wasi-sdk/releases

2. Add the WASI SDK bin directory to your PATH:
   ```
   export WASI_SDK_PATH=/path/to/wasi-sdk
   export PATH=$WASI_SDK_PATH/bin:$PATH
   ```

3. Verify the compiler is available:
   ```
   wasm32-wasip1-clang --version
   ```
   or
   ```
   wasm32-wasi-clang --version
   ```

Building
--------

Basic build configuration for wasm32-wasip1:

    $ ./Configure wasm32-wasip1 \
        no-threads \
        no-shared \
        no-asm \
        --prefix=/path/to/install \
        --openssldir=/path/to/openssldir

Then build with:

    $ make
    $ make install

Notes:

 - `no-threads`: WASM environments typically don't support threading
 - `no-shared`: WASM doesn't support shared libraries
 - `no-asm`: Assembly optimizations are not available for WASM
 - `no-dso`: Dynamic loading is not supported (automatically set)
 - `no-engine`: Engine support requires dynamic loading (automatically set)

Configuration Options
---------------------

You may want to disable additional features that are not needed for your
WASM application to reduce the size of the resulting library:

    $ ./Configure wasm32-wasip1 \
        no-threads no-shared no-asm \
        no-sock no-dgram no-ui-console \
        --prefix=/path/to/install

Common additional options:
 - `no-sock`: Disable socket support
 - `no-dgram`: Disable datagram support  
 - `no-ui-console`: Disable UI console functions
 - `no-stdio`: Disable stdio functions
 - `no-deprecated`: Disable deprecated APIs

For a minimal build focused on cryptographic operations:

    $ ./Configure wasm32-wasip1 \
        no-threads no-shared no-asm \
        no-sock no-dgram no-ui-console \
        no-engine no-hw no-apps \
        --prefix=/path/to/install

Using the WASM Library
----------------------

After building, you'll have static libraries (libcrypto.a and libssl.a) that
can be linked with your WASM application.

To compile and link your WASM application:

    $ wasm32-wasip1-clang -O3 \
        -I/path/to/install/include \
        -L/path/to/install/lib \
        -o myapp.wasm \
        myapp.c \
        -lssl -lcrypto

Running WASM Applications
--------------------------

WASM applications can be run using various runtimes:

1. Wasmtime:
   ```
   wasmtime myapp.wasm
   ```

2. Wasmer:
   ```
   wasmer run myapp.wasm
   ```

3. Node.js with WASI support:
   ```javascript
   const { WASI } = require('wasi');
   const fs = require('fs');
   
   const wasi = new WASI({
     args: process.argv,
     env: process.env,
   });
   
   const importObject = { wasi_snapshot_preview1: wasi.wasiImport };
   
   WebAssembly.instantiate(
     fs.readFileSync('./myapp.wasm'),
     importObject
   ).then((obj) => {
     wasi.start(obj.instance);
   });
   ```

Troubleshooting
---------------

### Compiler not found

If you get an error about `wasm32-wasip1-clang` not being found:

1. Verify WASI SDK is installed
2. Check that the bin directory is in your PATH
3. Try using `wasm32-wasi-clang` instead (use the `wasm32-wasi` target)

### Linking errors

If you encounter linking errors:

1. Ensure you're using the matching WASI SDK version for all components
2. Check that all required libraries are linked in the correct order
3. Verify that no forbidden features (threads, shared libs) are enabled

### Size concerns

If the resulting WASM binary is too large:

1. Use release build configuration (`-O3` optimization)
2. Disable unneeded OpenSSL features during configuration
3. Use `wasm-opt` from the Binaryen toolkit to optimize the WASM binary:
   ```
   wasm-opt -O3 myapp.wasm -o myapp-optimized.wasm
   ```
4. Strip debug information:
   ```
   wasm32-wasip1-strip myapp.wasm
   ```

### Runtime errors

If you encounter errors when running the WASM application:

1. Ensure the WASM runtime supports the required WASI features
2. Check that sufficient memory is allocated for cryptographic operations
3. Verify that any required files/certificates are accessible to the WASM
   environment

Limitations
-----------

When using OpenSSL in WASM environments, be aware of the following limitations:

1. No hardware acceleration: Assembly optimizations are disabled
2. No threading: Multi-threaded operations are not supported
3. No dynamic loading: Engines and dynamic libraries cannot be loaded
4. Limited I/O: Some file system and network operations may be restricted
   depending on the WASM runtime environment
5. Memory constraints: WASM environments may have memory limitations

For more information about WASI and WebAssembly, see:
 - https://wasi.dev/
 - https://webassembly.org/
 - https://github.com/WebAssembly/wasi-sdk
