/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Packet.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once

#ifndef ESP32_MYSQL_PACKET_H
#define ESP32_MYSQL_PACKET_H

#include <Arduino.h>
#include <Client.h>
#if defined(ESP32)
  #include "mbedtls/ctr_drbg.h"
  #include "mbedtls/entropy.h"
  #include "mbedtls/md.h"
  #include "mbedtls/pk.h"
  #include "mbedtls/rsa.h"
  #include "mbedtls/ssl.h"
  #include "mbedtls/net_sockets.h"
#endif

#define ESP32_MYSQL_OK_PACKET         0x00
#define ESP32_MYSQL_EOF_PACKET        0xfe
#define ESP32_MYSQL_ERROR_PACKET      0xff

#define MAX_TRANSMISSION_UNIT   1500

// Minimal subset of capability bits we need when crafting the handshake response
#define CLIENT_LONG_PASSWORD                   0x00000001UL
#define CLIENT_FOUND_ROWS                      0x00000002UL
#define CLIENT_LONG_FLAG                       0x00000004UL
#define CLIENT_CONNECT_WITH_DB                 0x00000008UL
#define CLIENT_PROTOCOL_41                     0x00000200UL
#define CLIENT_INTERACTIVE                     0x00000400UL
#define CLIENT_SSL                             0x00000800UL
#define CLIENT_TRANSACTIONS                    0x00002000UL
#define CLIENT_SECURE_CONNECTION               0x00008000UL
#define CLIENT_MULTI_STATEMENTS                0x00010000UL
#define CLIENT_MULTI_RESULTS                   0x00020000UL
#define CLIENT_PLUGIN_AUTH                     0x00080000UL
///////

enum AuthPlugin
{
  AUTH_MYSQL_NATIVE_PASSWORD = 0,
  AUTH_CACHING_SHA2_PASSWORD,
  AUTH_SHA256_PASSWORD,
  AUTH_UNKNOWN
};

class MySQL_Packet 
{
  public:
    byte *buffer;           // buffer for reading packets
    
    uint16_t largest_buffer_size = 0;
    //////
    
    int packet_len;         // length of current packet
    Client *client;         // instance of client class (e.g. EthernetClient)
    char *server_version;   // save server version from handshake
    char  auth_plugin[32];  // authentication plugin name advertised by server
    uint8_t auth_plugin_data_len = 0;
    uint32_t server_capabilities = 0;
    AuthPlugin auth_plugin_type = AUTH_MYSQL_NATIVE_PASSWORD;

    MySQL_Packet(Client *client_instance);
    virtual ~MySQL_Packet()
    {
			if (buffer)
			{
				ESP32_MYSQL_LOGDEBUG("Free buffer");

				free(buffer);
			}
			if (server_version)
			{
				ESP32_MYSQL_LOGDEBUG("Free server_version");

				free(server_version);
			}
      if (cached_password)
      {
        free(cached_password);
        cached_password = NULL;
      }

      cleanup_tls();
    };
    
    bool    complete_handshake(char *user, char *password);
    void    send_authentication_packet(char *user, char *password, char *db = NULL, uint32_t client_flags = 0, uint8_t sequence_id = 0x01);
    void    parse_handshake_packet();
    AuthPlugin get_auth_plugin() const
    {
      return auth_plugin_type;
    }
    void    reset_for_connect()
    {
      cache_password(NULL);
      ssl_request_sent = false;
      next_sequence_id = 0x01;
      tls_established = false;
      cleanup_tls();
    }
    void    enable_tls(bool enable = true, const char *sni_host = NULL)
    {
      tls_requested = enable;

      if (sni_host)
      {
        strncpy(tls_sni_host, sni_host, sizeof(tls_sni_host) - 1);
        tls_sni_host[sizeof(tls_sni_host) - 1] = 0;
      }
    }
    bool    tls_active() const
    {
      return tls_established;
    }
    bool    wants_tls() const
    {
      return tls_requested;
    }
    bool    write_bytes(const uint8_t *data, size_t len);
    bool    read_bytes(uint8_t *out, size_t len);
    uint32_t build_client_flags(bool use_tls) const;
    bool    send_ssl_request(uint32_t client_flags, uint8_t sequence_id = 0x01);
    bool    start_tls_handshake();
    uint8_t get_next_sequence_id() const
    {
      return next_sequence_id;
    }
    void    set_next_sequence_id(uint8_t seq)
    {
      next_sequence_id = seq;
    }
    bool    encrypt_password_rsa(const uint8_t *pubkey, size_t pubkey_len, const char *password,
                                 uint8_t *encrypted, size_t *encrypted_len);
    void    cache_password(const char *password)
    {
      if (cached_password)
      {
        free(cached_password);
        cached_password = NULL;
      }

      if (password)
      {
        size_t len = strlen(password);
        cached_password = (char *) malloc(len + 1);

        if (cached_password)
        {
          memcpy(cached_password, password, len + 1);
        }
      }
    }
    const char *get_cached_password() const
    {
      return cached_password;
    }
    bool    scramble_password(char *password, byte *pwd_hash);

    bool    read_packet();
    
    int     get_packet_type();
    void    parse_error_packet();
    int     get_lcb_len(const int& offset);
    int     read_int(const int& offset, const int& size = 0);
    void    store_int(byte *buff, const long& value, const int& size);
    int     read_lcb_int(const int& offset);
    int     wait_for_bytes(const int& bytes_need);

    void    print_packet();

  private:
    byte seed[20];
    bool tls_requested = false;
    bool tls_established = false;
    char tls_sni_host[64] = { 0 };
    bool ssl_request_sent = false;
    uint8_t next_sequence_id = 0x01;
    char *cached_password = NULL;
    AuthPlugin plugin_from_name(const char *name) const;
    bool cleanup_tls();
    static int tls_send_cb(void *ctx, const unsigned char *buf, size_t len);
    static int tls_recv_cb(void *ctx, unsigned char *buf, size_t len);
    int blocking_read(unsigned char *buf, size_t len);
    int blocking_read_tls(unsigned char *buf, size_t len);
    int blocking_write_tls(const unsigned char *buf, size_t len);
#if defined(ESP32)
    mbedtls_ssl_context tls_ctx;
    mbedtls_ssl_config tls_conf;
    mbedtls_ctr_drbg_context tls_ctr_drbg;
    mbedtls_entropy_context tls_entropy;
#endif
    bool scramble_password_caching_sha2(char *password, byte *pwd_hash);
    bool scramble_password_sha256(char *password, byte *pwd_hash);
};



#endif    // ESP32_MYSQL_PACKET_H
