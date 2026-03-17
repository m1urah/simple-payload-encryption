#include <Windows.h>
#include <stdio.h>
#include <string.h>

#include "aes.h"


// =======  Constants  ===================================================== //

#define SIZE_KEY        32
#define SIZE_IV         16
#define PKCS7_PAD_SIZE  16

unsigned char pKey[] = {
    0x54, 0x68, 0x69, 0x73, 0x69, 0x73, 0x61, 0x73,
    0x69, 0x6D, 0x70, 0x6C, 0x65, 0x74, 0x6F, 0x65,
    0x6E, 0x63, 0x72, 0x79, 0x70, 0x74, 0x73, 0x6F,
    0x6D, 0x65, 0x73, 0x74, 0x75, 0x66, 0x66, 0x21
};
unsigned char pIv[] = {
    0x41, 0x6E, 0x64, 0x74, 0x68, 0x69, 0x73, 0x69,
    0x73, 0x74, 0x68, 0x65, 0x69, 0x76, 0x21, 0x21
};


// =======  Helpers  ======================================================= //

static void SafeFree(PVOID *ptr) {
    if (ptr && *ptr) {
        HeapFree(GetProcessHeap(), 0, *ptr);
        *ptr = NULL;
    }
}

static void PrintHexData(IN unsigned char *data, IN SIZE_T dataLen) {
    SIZE_T strLen = strlen(data); // jic it's a str
    while (dataLen-- && strLen--) {
        // We print it cleanly
        if (dataLen == 0 || strLen == 0) {
            printf("0x%02X\n", *data);
        } else {
            printf("0x%02X ", *data++);
        }
    }
}

static BOOL UnpadBuffer(
    IN PVOID data,
    IN SIZE_T dataSize,
    OUT PVOID *dataOut,
    OUT PSIZE_T dataOutSize
) {
    int padding = ((unsigned char *)data)[dataSize - 1];
    *dataOutSize = dataSize - padding;

    *dataOut = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *dataOutSize);
    if (*dataOut == NULL) {
        printf("[-] Error allocating output buffer: %d\n", GetLastError());
        return FALSE;
    }

    memcpy(*dataOut, data, *dataOutSize);
    return TRUE;
}

// Simple implementation of PKCS#7 (we always pad, even on dataLength % 16 = 0)
static BOOL PadBuffer(
    IN PVOID data,
    IN SIZE_T dataSize,
    OUT PVOID *dataOut,
    OUT PSIZE_T dataOutSize
) {
    int neededPadding = PKCS7_PAD_SIZE - (dataSize % PKCS7_PAD_SIZE);
    *dataOutSize = dataSize + neededPadding;
    *dataOut = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *dataOutSize);
    if (*dataOut == NULL) {
        printf("[-] Error allocating output buffer: %d\n", GetLastError());
        return FALSE;
    }

    errno_t err = memcpy_s(*dataOut, *dataOutSize, data, dataSize);
    if (err != 0) {
        printf(
            "[-] Error copying existing data into padded buffer: %d\n",
            GetLastError()
        );

        SafeFree(dataOut);
        return FALSE;
    }

    memset((unsigned char *)*dataOut + dataSize, neededPadding, neededPadding);
    return TRUE;
}


// =======  Cipher  ======================================================== //

static BOOL DecryptWithAES(
    IN PVOID data,
    IN SIZE_T dataSize,
    OUT PVOID *dataOut,
    OUT PSIZE_T dataOutSize
) {
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, pKey, pIv);

    *dataOutSize = dataSize;
    *dataOut = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *dataOutSize);
    if (*dataOut == NULL) {
        printf(
            "[-] Error while allocating plaintext buffer: %d", GetLastError()
        );
        return FALSE;
    }

    memcpy(*dataOut, data, *dataOutSize); // Remove padding
    AES_CBC_decrypt_buffer(&ctx, *dataOut, *dataOutSize);

    return TRUE;
}

static BOOL EncryptWithAES(
    IN PVOID data,
    IN SIZE_T dataSize,
    OUT PVOID *dataOut,
    OUT PSIZE_T dataOutSize
) {
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, pKey, pIv);

    *dataOutSize = dataSize;
    *dataOut = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *dataOutSize);
    if (*dataOut == NULL) {
        printf(
            "[-] Error while allocating ciphertext buffer: %d", GetLastError()
        );
        return FALSE;
    }

    memcpy(*dataOut, data, *dataOutSize);

    AES_CBC_encrypt_buffer(&ctx, *dataOut, *dataOutSize);
    return TRUE;
}


// =======  Data Handling  ================================================= //

static BOOL ReadStdinAndFlush(IN SIZE_T dataSize, OUT PVOID *data) {
    *data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dataSize);
    if (*data == NULL) {
        printf("[-] Error allocating read input buffer: %d", GetLastError());
        return FALSE;
    }

    if (fgets((char *)*data, dataSize, stdin) == NULL) {
        printf("[-] Error reading input from stdin: %d", GetLastError());
        SafeFree(data);
        return FALSE;
    }

    SIZE_T len = strlen((char *)*data);
    if (len > 0 && ((char *)*data)[len - 1] == '\n') {
        ((char *)*data)[len - 1] = '\0';
    } else if (getchar() != '\n') {
        // If no new line, input was longer than buffer
        printf(
            "[-] Invalid input, it must be of less or equal to %zu "
            "characters.\n",
            dataSize - 1
        );
        SafeFree(data);
        return FALSE;
    }

    return TRUE;
}

static BOOL GetInputData(OUT PVOID *data, OUT PSIZE_T dataSize) {
    // --- DATA LENGTH --- //
    printf("[?] Enter data length (0 - 99): ");

    unsigned char *dataLengthIn = NULL;
    if (!ReadStdinAndFlush(3, (PVOID *)&dataLengthIn)) { // 2 + \0
        return -1;
    }

    if (dataLengthIn[0] < '0') {
        printf(
            "[-] Negative data length received, it must be a value 0-99: %s",
            dataLengthIn
        );
        return FALSE;
    }
    BOOL twoDigits = dataLengthIn[1] != '\n';
    if (dataLengthIn[0] > '9' &&
        (twoDigits && (dataLengthIn[1] < '0' || dataLengthIn[1] > '9'))) {
        printf(
            "[-] An invalid number was received, it must be a value 0-99: %s",
            dataLengthIn
        );
        return FALSE;
    }

    *dataSize = (SIZE_T)atoi((char *)dataLengthIn);
    HeapFree(GetProcessHeap(), 0, dataLengthIn);

    // --- DATA --- //
    printf("[?] Enter plaintext data (max %zu chars): ", (SIZE_T)*dataSize);
    if (!ReadStdinAndFlush(*dataSize + 1, data)) {
        return FALSE;
    }

    printf("[#] Data received: %s\n", (char *)*data);

    // --- ADJUST DATA SIZE --- //
    *dataSize = strnlen_s(*data, *dataSize);
    return TRUE;
}


// =======  Entrypoint  ==================================================== //

int main(void) {
    printf("[*] Using 256-bit key: ");
    PrintHexData(pKey, SIZE_KEY);

    printf("[*] Using 128-bit IV: ");
    PrintHexData(pIv, SIZE_IV);

    unsigned char *data = NULL;
    SIZE_T dataSize = 0;
    if (!GetInputData((PVOID *)&data, &dataSize)) {
        return -1;
    }

    unsigned char *dataPadded = NULL;
    SIZE_T dataPaddedSize = 0;
    if (!PadBuffer(data, dataSize, (PVOID *)&dataPadded, &dataPaddedSize)) {
        SafeFree((PVOID *)&data);
        return -1;
    }

    printf("[*] Unpadded data in hex (size = %zu): ", dataSize);
    PrintHexData(data, dataSize);
    SafeFree((PVOID *)&data);

    printf("[*] Padded data in hex (size = %zu): ", dataPaddedSize);
    PrintHexData(dataPadded, dataPaddedSize);


    // --- ENCRYPT --- //

    unsigned char *dataEncrypted = NULL;
    SIZE_T dataEncryptedSize = 0;
    if (!EncryptWithAES(
            dataPadded,
            dataPaddedSize,
            (PVOID *)&dataEncrypted,
            &dataEncryptedSize
        )) {
        
        SafeFree((PVOID *)&dataPadded);
        return -1;
    }

    SafeFree((PVOID *)&dataPadded);
    printf("[+] Encrypted data: ");
    PrintHexData(dataEncrypted, dataEncryptedSize);

    
    // --- DECRYPT --- //

    unsigned char *dataDecrypted = NULL;
    SIZE_T dataDecryptedSize = 0;
    if (!DecryptWithAES(
            dataEncrypted,
            dataEncryptedSize,
            (PVOID *)&dataDecrypted,
            &dataDecryptedSize
        )) {

        SafeFree((PVOID *)&dataEncrypted);
        return -1;
    }

    SafeFree((PVOID *)&dataEncrypted);
    printf("[+] Decrypted data: ");
    PrintHexData(dataDecrypted, dataDecryptedSize);


    // --- SHOW PLAINTEXT --- //

    unsigned char *plaintext = NULL;
    SIZE_T plaintextSize = 0;
    if (!UnpadBuffer(
            dataDecrypted,
            dataDecryptedSize,
            (PVOID *)&plaintext,
            &plaintextSize
        )) {

        SafeFree((PVOID *)&dataEncrypted);
        return -1;
    }

    SafeFree((PVOID *)&dataEncrypted);
    printf("[+] Plaintext decrypted data: %s", plaintext);
    SafeFree((PVOID *)&plaintext);

    return 0;
}
