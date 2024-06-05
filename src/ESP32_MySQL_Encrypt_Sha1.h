/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Encrypt_Sha1.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once

#ifndef ESP32_MYSQL_ENCRYPT_SHA1_H
#define ESP32_MYSQL_ENCRYPT_SHA1_H

#include <inttypes.h>
#include "Print.h"

#define HASH_LENGTH 20
#define BLOCK_LENGTH 64

union _buffer 
{
  uint8_t   b [BLOCK_LENGTH];
  uint32_t  w [BLOCK_LENGTH / 4];
};

union _state 
{
  uint8_t   b [HASH_LENGTH];
  uint32_t  w [HASH_LENGTH / 4];
};

class Encrypt_SHA1 : public Print
{
  public:
    void      init();
    void      initHmac(const uint8_t* secret, const int& secretLength);
    uint8_t*  result();
    virtual size_t write(uint8_t data);
    virtual size_t write(uint8_t* data, const int& length);
    using Print::write;
    
  private:
    void      pad();
    void      addUncounted(const uint8_t& data);
    void      hashBlock();
    uint32_t  rol32(const uint32_t& number, const uint8_t& bits);
    _buffer   buffer;
    uint8_t   bufferOffset;
    _state    state;
    uint32_t  byteCount;
    uint8_t   keyBuffer[BLOCK_LENGTH];
    uint8_t   innerHash[HASH_LENGTH];
};

//extern Encrypt_SHA1 Sha1;


#endif    // ESP32_MYSQL_ENCRYPT_SHA1_H
