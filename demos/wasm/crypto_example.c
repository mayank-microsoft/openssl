/*
 * Simple OpenSSL crypto demo for WebAssembly (WASM32-wasip1)
 * 
 * This example demonstrates basic cryptographic operations using OpenSSL
 * in a WebAssembly environment.
 * 
 * To build this example:
 *   1. First build OpenSSL for wasm32-wasip1 using the build script:
 *      ./build-wasm32-wasip1.sh /tmp/openssl-wasm
 * 
 *   2. Compile this example:
 *      wasm32-wasip1-clang -O3 \
 *        -I/tmp/openssl-wasm/include \
 *        -L/tmp/openssl-wasm/lib \
 *        -o crypto_example.wasm \
 *        crypto_example.c \
 *        -lssl -lcrypto
 * 
 *   3. Run with a WASM runtime:
 *      wasmtime crypto_example.wasm
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

/* Print hex dump of binary data */
static void print_hex(const char *label, const unsigned char *data, size_t len)
{
    size_t i;
    printf("%s: ", label);
    for (i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/* Example 1: SHA-256 hash */
static int example_sha256(void)
{
    const char *message = "Hello, WebAssembly!";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX *mdctx;
    unsigned int hash_len;
    
    printf("\n=== SHA-256 Example ===\n");
    printf("Message: %s\n", message);
    
    /* Create and initialize digest context */
    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        fprintf(stderr, "Failed to create EVP_MD_CTX\n");
        return 0;
    }
    
    /* Initialize SHA-256 */
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        fprintf(stderr, "Failed to initialize SHA-256\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }
    
    /* Update with message */
    if (EVP_DigestUpdate(mdctx, message, strlen(message)) != 1) {
        fprintf(stderr, "Failed to update digest\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }
    
    /* Finalize */
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        fprintf(stderr, "Failed to finalize digest\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }
    
    print_hex("SHA-256 Hash", hash, hash_len);
    
    EVP_MD_CTX_free(mdctx);
    return 1;
}

/* Example 2: AES-256-CBC encryption */
static int example_aes_encrypt(void)
{
    /* Key and IV for AES-256-CBC */
    unsigned char key[32]; /* 256 bits */
    unsigned char iv[16];  /* 128 bits */
    
    const char *plaintext = "This is a secret message for WASM!";
    unsigned char ciphertext[128];
    unsigned char decrypted[128];
    
    int ciphertext_len, decrypted_len;
    EVP_CIPHER_CTX *ctx;
    int len;
    
    printf("\n=== AES-256-CBC Example ===\n");
    printf("Plaintext: %s\n", plaintext);
    
    /* Generate random key and IV */
    if (RAND_bytes(key, sizeof(key)) != 1) {
        fprintf(stderr, "Failed to generate random key\n");
        return 0;
    }
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        fprintf(stderr, "Failed to generate random IV\n");
        return 0;
    }
    
    print_hex("Key", key, sizeof(key));
    print_hex("IV", iv, sizeof(iv));
    
    /* Encrypt */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Failed to create cipher context\n");
        return 0;
    }
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Failed to initialize encryption\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, 
                          (unsigned char *)plaintext, strlen(plaintext)) != 1) {
        fprintf(stderr, "Failed to encrypt\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "Failed to finalize encryption\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    ciphertext_len += len;
    
    print_hex("Ciphertext", ciphertext, ciphertext_len);
    
    EVP_CIPHER_CTX_free(ctx);
    
    /* Decrypt */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Failed to create cipher context for decryption\n");
        return 0;
    }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Failed to initialize decryption\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    if (EVP_DecryptUpdate(ctx, decrypted, &len, ciphertext, ciphertext_len) != 1) {
        fprintf(stderr, "Failed to decrypt\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    decrypted_len = len;
    
    if (EVP_DecryptFinal_ex(ctx, decrypted + len, &len) != 1) {
        fprintf(stderr, "Failed to finalize decryption\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    decrypted_len += len;
    
    decrypted[decrypted_len] = '\0';
    printf("Decrypted: %s\n", decrypted);
    
    EVP_CIPHER_CTX_free(ctx);
    
    /* Verify */
    if (strcmp((char *)decrypted, plaintext) == 0) {
        printf("Encryption/Decryption successful!\n");
        return 1;
    } else {
        fprintf(stderr, "Decryption failed - plaintext doesn't match\n");
        return 0;
    }
}

int main(void)
{
    printf("OpenSSL WebAssembly Demo\n");
    printf("========================\n");
    printf("OpenSSL version: %s\n", OpenSSL_version(OPENSSL_VERSION));
    
    /* Run examples */
    if (!example_sha256()) {
        fprintf(stderr, "SHA-256 example failed\n");
        return 1;
    }
    
    if (!example_aes_encrypt()) {
        fprintf(stderr, "AES encryption example failed\n");
        return 1;
    }
    
    printf("\nAll examples completed successfully!\n");
    return 0;
}
