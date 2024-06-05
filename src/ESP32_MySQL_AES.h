#ifndef ESP32_MYSQL_AES_H
#define ESP32_MYSQL_AES_H

#include <Arduino.h>
#include "mbedtls/aes.h"

class ESP32_MySQL_AES {
public:
    ESP32_MySQL_AES();
    ~ESP32_MySQL_AES();

    void init(const byte* key);
    String encrypt(const byte* plaintext, size_t length);

private:
    mbedtls_aes_context aes;
    byte key[32];

    void pad(const byte* input, size_t length, byte* output, size_t &outputLength);
    void bytesToHex(const byte* input, size_t length, String &output);
};

#endif // ESP32_MYSQL_AES_H
