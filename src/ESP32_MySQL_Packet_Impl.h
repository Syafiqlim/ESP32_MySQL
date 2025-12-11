/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Packet_Impl.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once

#ifndef ESP32_MYSQL_PACKET_IMPL_H
#define ESP32_MYSQL_PACKET_IMPL_H

#include <Arduino.h>

#include <ESP32_MySQL_Encrypt_Sha1.h>
#include <ESP32_MySQL_Sha256.h>

#if ( USING_WIFI_ESP_AT )
  #define ESP32_MYSQL_DATA_TIMEOUT  10000   
#else
  #define ESP32_MYSQL_DATA_TIMEOUT  6000    // Client wait in milliseconds
#endif  
//////

#define ESP32_MYSQL_WAIT_INTERVAL 300    // WiFi client wait interval
#define ESP32_MYSQL_TLS_TIMEOUT_MS 10000

/*
  Constructor

  Initialize the buffer and store client instance.
*/
MySQL_Packet::MySQL_Packet(Client *client_instance)
{
	buffer = NULL;
	server_version = NULL;
	client = client_instance;
	memset(auth_plugin, 0, sizeof(auth_plugin));
	auth_plugin_type = AUTH_MYSQL_NATIVE_PASSWORD;
	auth_plugin_data_len = 0;
	server_capabilities = 0;
  memset(seed, 0, sizeof(seed));
#if defined(ESP32)
  mbedtls_ssl_init(&tls_ctx);
  mbedtls_ssl_config_init(&tls_conf);
  mbedtls_ctr_drbg_init(&tls_ctr_drbg);
  mbedtls_entropy_init(&tls_entropy);
#endif
  memset(tls_sni_host, 0, sizeof(tls_sni_host));
  tls_requested = false;
  tls_established = false;
  ssl_request_sent = false;
  next_sequence_id = 0x01;
}

uint32_t MySQL_Packet::build_client_flags(bool use_tls) const
{
  uint32_t flags = 0x0003A60D | CLIENT_PLUGIN_AUTH;

  if (use_tls)
    flags |= CLIENT_SSL;

  return flags;
}

bool MySQL_Packet::cleanup_tls()
{
#if defined(ESP32)
  mbedtls_ssl_free(&tls_ctx);
  mbedtls_ssl_config_free(&tls_conf);
  mbedtls_ctr_drbg_free(&tls_ctr_drbg);
  mbedtls_entropy_free(&tls_entropy);
  tls_established = false;
  return true;
#else
  return false;
#endif
}

int MySQL_Packet::tls_send_cb(void *ctx, const unsigned char *buf, size_t len)
{
#if defined(ESP32)
  MySQL_Packet *self = static_cast<MySQL_Packet *>(ctx);

  if (!self || !self->client)
    return MBEDTLS_ERR_NET_SEND_FAILED;

  size_t written = self->client->write(buf, len);

  return (written > 0) ? (int) written : MBEDTLS_ERR_NET_SEND_FAILED;
#else
  (void) ctx;
  (void) buf;
  (void) len;
  return -1;
#endif
}

int MySQL_Packet::tls_recv_cb(void *ctx, unsigned char *buf, size_t len)
{
#if defined(ESP32)
  MySQL_Packet *self = static_cast<MySQL_Packet *>(ctx);

  if (!self || !self->client)
    return MBEDTLS_ERR_NET_RECV_FAILED;

  unsigned long start = millis();

  while ((self->client->available() == 0) && ((millis() - start) < ESP32_MYSQL_DATA_TIMEOUT))
  {
    delay(1);
    yield();
  }

  if (self->client->available() == 0)
    return MBEDTLS_ERR_SSL_TIMEOUT;

  int received = self->client->read(buf, len);

  if (received <= 0)
    return MBEDTLS_ERR_NET_RECV_FAILED;

  return received;
#else
  (void) ctx;
  (void) buf;
  (void) len;
  return -1;
#endif
}

int MySQL_Packet::blocking_read(unsigned char *buf, size_t len)
{
  if (!client)
    return -1;

  size_t offset = 0;
  unsigned long start = millis();

  while ((offset < len) && ((millis() - start) < ESP32_MYSQL_DATA_TIMEOUT))
  {
    int avail = client->available();

    if (avail > 0)
    {
      int read_now = client->read(buf + offset, len - offset);

      if (read_now > 0)
        offset += read_now;
    }
    else
    {
      delay(1);
      yield();
    }
  }

  return offset;
}

int MySQL_Packet::blocking_read_tls(unsigned char *buf, size_t len)
{
#if defined(ESP32)
  size_t offset = 0;
  unsigned long start = millis();

  while ((offset < len) && ((millis() - start) < ESP32_MYSQL_DATA_TIMEOUT))
  {
    int ret = mbedtls_ssl_read(&tls_ctx, buf + offset, len - offset);

    if (ret > 0)
    {
      offset += ret;
    }
    else if ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE))
    {
      delay(1);
      yield();
      continue;
    }
    else
    {
      return ret;
    }
  }

  return offset;
#else
  (void) buf;
  (void) len;
  return -1;
#endif
}

int MySQL_Packet::blocking_write_tls(const unsigned char *buf, size_t len)
{
#if defined(ESP32)
  size_t offset = 0;
  unsigned long start = millis();

  while ((offset < len) && ((millis() - start) < ESP32_MYSQL_DATA_TIMEOUT))
  {
    int ret = mbedtls_ssl_write(&tls_ctx, buf + offset, len - offset);

    if (ret > 0)
    {
      offset += ret;
    }
    else if ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE))
    {
      delay(1);
      yield();
      continue;
    }
    else
    {
      return ret;
    }
  }

  return offset;
#else
  (void) buf;
  (void) len;
  return -1;
#endif
}

bool MySQL_Packet::write_bytes(const uint8_t *data, size_t len)
{
  if (!client || (data == NULL))
    return false;

  if (tls_established)
  {
    int ret = blocking_write_tls(data, len);
    return ret == (int) len;
  }

  size_t written = client->write(data, len);

  return written == len;
}

bool MySQL_Packet::read_bytes(uint8_t *out, size_t len)
{
  if (!client || (out == NULL))
    return false;

  int ret = tls_established ? blocking_read_tls(out, len) : blocking_read(out, len);

  return ret == (int) len;
}

bool MySQL_Packet::send_ssl_request(uint32_t client_flags, uint8_t sequence_id)
{
  // SSL Request packet: header (4 bytes) + payload (32 bytes)
  uint8_t packet[4 + 32];
  size_t offset = 0;

  // Packet length (payload only)
  store_int(packet, 32, 3);
  packet[3] = sequence_id;
  offset = 4;

  store_int(&packet[offset], client_flags | CLIENT_SSL, 4);
  offset += 4;

  // max_allowed_packet (little endian). Keep minimal 1MB default style (0x01000000).
  packet[offset + 0] = 0x00;
  packet[offset + 1] = 0x00;
  packet[offset + 2] = 0x00;
  packet[offset + 3] = 0x01;
  offset += 4;

  // charset - default 8 (latin1)
  packet[offset++] = byte(0x08);

  // filler
  for (int i = 0; i < 23; i++)
    packet[offset + i] = 0x00;

  offset += 23;

  ssl_request_sent = write_bytes(packet, offset);

  if (ssl_request_sent)
    next_sequence_id = sequence_id + 1;

  return ssl_request_sent;
}

bool MySQL_Packet::start_tls_handshake()
{
#if defined(ESP32)
  cleanup_tls();

  mbedtls_ssl_init(&tls_ctx);
  mbedtls_ssl_config_init(&tls_conf);
  mbedtls_ctr_drbg_init(&tls_ctr_drbg);
  mbedtls_entropy_init(&tls_entropy);

  const char *pers = "esp32_mysql_tls";
  int ret = mbedtls_ctr_drbg_seed(&tls_ctr_drbg, mbedtls_entropy_func, &tls_entropy, (const unsigned char *) pers, strlen(pers));

  if (ret != 0)
  {
    ESP32_MYSQL_LOGERROR1("TLS seed failed, code =", ret);
    return false;
  }

  ret = mbedtls_ssl_config_defaults(&tls_conf,
                                    MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);

  if (ret != 0)
  {
    ESP32_MYSQL_LOGERROR1("TLS config defaults failed, code =", ret);
    return false;
  }

  mbedtls_ssl_conf_authmode(&tls_conf, MBEDTLS_SSL_VERIFY_NONE);
  mbedtls_ssl_conf_rng(&tls_conf, mbedtls_ctr_drbg_random, &tls_ctr_drbg);

  ret = mbedtls_ssl_setup(&tls_ctx, &tls_conf);

  if (ret != 0)
  {
    ESP32_MYSQL_LOGERROR1("TLS setup failed, code =", ret);
    return false;
  }

  if (tls_sni_host[0] != 0)
    mbedtls_ssl_set_hostname(&tls_ctx, tls_sni_host);

  mbedtls_ssl_set_bio(&tls_ctx, this, tls_send_cb, tls_recv_cb, NULL);

  unsigned long start = millis();

  while ((ret = mbedtls_ssl_handshake(&tls_ctx)) != 0)
  {
    if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
    {
      ESP32_MYSQL_LOGERROR1("TLS handshake failed, code =", ret);
      cleanup_tls();
      return false;
    }

    if ((millis() - start) > ESP32_MYSQL_TLS_TIMEOUT_MS)
    {
      ESP32_MYSQL_LOGERROR("TLS handshake timeout");
      cleanup_tls();
      return false;
    }

    delay(1);
    yield();
  }

  tls_established = true;
  return true;
#else
  ESP32_MYSQL_LOGERROR("TLS not supported on this platform");
  return false;
#endif
}

/*
  send_authentication_packet

  This method builds a response packet used to respond to the server's
  challenge packet (called the handshake packet). It includes the user
  name and password scrambled using the SHA1 seed from the handshake
  packet. It also sets the character set (default is 8 which you can
  change to meet your needs).

  Note: you can also set the default database in this packet. See
        the code before for a comment on where this happens.

  The authentication packet is defined as follows.

  Bytes                        Name
  -----                        ----
  4                            client_flags
  4                            max_packet_size
  1                            charset_number
  23                           (filler) always 0x00...
  n (Null-Terminated String)   user
  n (Length Coded Binary)      scramble_buff (1 + x bytes)
  n (Null-Terminated String)   databasename (optional)

  user[in]        User name
  password[in]    password
  db[in]          default database
*/

void MySQL_Packet::send_authentication_packet(char *user, char *password, char *db, uint32_t client_flags, uint8_t sequence_id)
{ 
  byte this_buffer[256];
  byte scramble[SHA256_HASH_SIZE];

  int size_send = 4;

  if (client_flags == 0)
  {
    const bool use_tls = tls_established || ssl_request_sent || tls_requested;
    client_flags = build_client_flags(use_tls);
  }

  store_int(&this_buffer[size_send], client_flags, 4);
  size_send += 4;

  // max_allowed_packet
  this_buffer[size_send] = 0;
  this_buffer[size_send + 1] = 0;
  this_buffer[size_send + 2] = 0;
  this_buffer[size_send + 3] = 1;
  size_send += 4;

  // charset - default is 8
  this_buffer[size_send] = byte(0x08);
  size_send += 1;

  for (int i = 0; i < 23; i++)
    this_buffer[size_send + i] = 0x00;

  size_send += 23;

  // user name
  memcpy((char *) &this_buffer[size_send], user, strlen(user));
  size_send += strlen(user) + 1;
  this_buffer[size_send - 1] = 0x00;

  AuthPlugin plugin = (auth_plugin_type == AUTH_UNKNOWN) ? AUTH_MYSQL_NATIVE_PASSWORD : auth_plugin_type;
  bool has_scramble = false;
  uint8_t scramble_len = 0;

  if (plugin == AUTH_CACHING_SHA2_PASSWORD)
  {
    has_scramble = scramble_password_caching_sha2(password, scramble);
    scramble_len = SHA256_HASH_SIZE;
  }
  else if (plugin == AUTH_SHA256_PASSWORD)
  {
    has_scramble = scramble_password_sha256(password, scramble);
    scramble_len = SHA256_HASH_SIZE;
  }
  else
  {
    has_scramble = scramble_password(password, scramble);
    scramble_len = 20;
  }

  if (has_scramble)
  {
    this_buffer[size_send] = scramble_len;
    size_send += 1;

    for (uint8_t i = 0; i < scramble_len; i++)
      this_buffer[i + size_send] = scramble[i];

    size_send += scramble_len;
  }
  else
  {
    this_buffer[size_send] = 0x00;
    size_send += 1;
  }

  if (db)
  {
    memcpy((char *) &this_buffer[size_send], db, strlen(db));
    size_send += strlen(db) + 1;
    this_buffer[size_send - 1] = 0x00;
  }
  else
  {
    this_buffer[size_send] = 0x00;
    size_send += 1;
  }

  // Authentication plugin name (makes the server honor our scramble choice)
  const char *plugin_name = (auth_plugin[0] != 0) ? auth_plugin : "mysql_native_password";
  const size_t plugin_len = strlen(plugin_name);

  memcpy(&this_buffer[size_send], plugin_name, plugin_len + 1);
  size_send += plugin_len + 1;

  // Write packet size
  int p_size = size_send - 4;
  store_int(&this_buffer[0], p_size, 3);
  this_buffer[3] = sequence_id;

  next_sequence_id = sequence_id + 1;

  // Write the packet
  ESP32_MYSQL_LOGINFO1("Writing authentication packet, size =", size_send);

  write_bytes((uint8_t*)this_buffer, size_send);
}

/*
  scramble_password - Build a SHA1 scramble of the user password

  This method uses the password hash seed sent from the server to
  form a SHA1 hash of the password. This is used to send back to
  the server to complete the challenge and response step in the
  authentication handshake.

  password[in]    User's password in clear text
  pwd_hash[in]    Seed from the server

  Returns bool - True = scramble succeeded
*/
bool MySQL_Packet::scramble_password(char *password, byte *pwd_hash) 
{
  byte *digest;
  byte hash1[20];
  byte hash2[20];
  byte hash3[20];
  byte pwd_buffer[40];

  if (strlen(password) == 0)
    return false;

  // hash1
  Sha1.init();
  Sha1.print(password);
  digest = Sha1.result();
  memcpy(hash1, digest, 20);

  // hash2
  Sha1.init();
  Sha1.write(hash1, 20);
  digest = Sha1.result();
  memcpy(hash2, digest, 20);

  // hash3 of seed + hash2
  Sha1.init();
  memcpy(pwd_buffer, &seed, 20);
  memcpy(pwd_buffer + 20, hash2, 20);
  Sha1.write(pwd_buffer, 40);
  digest = Sha1.result();
  memcpy(hash3, digest, 20);

  // XOR for hash4
  for (int i = 0; i < 20; i++)
    pwd_hash[i] = hash1[i] ^ hash3[i];

  return true;
}

bool MySQL_Packet::scramble_password_caching_sha2(char *password, byte *pwd_hash)
{
  if (!password || (strlen(password) == 0))
    return false;

  uint8_t hash1[SHA256_HASH_SIZE];
  uint8_t hash2[SHA256_HASH_SIZE];
  uint8_t hash3[SHA256_HASH_SIZE];

  ESP32_MySQL_SHA256 sha256;
  sha256.update((uint8_t *) password, strlen(password));
  sha256.final(hash1);

  ESP32_MySQL_SHA256 sha256_second;
  sha256_second.update(hash1, SHA256_HASH_SIZE);
  sha256_second.final(hash2);

  ESP32_MySQL_SHA256 sha256_third;
  sha256_third.update(hash2, SHA256_HASH_SIZE);
  sha256_third.update(seed, 20);
  sha256_third.final(hash3);

  for (int i = 0; i < SHA256_HASH_SIZE; i++)
    pwd_hash[i] = hash1[i] ^ hash3[i];

  return true;
}

bool MySQL_Packet::scramble_password_sha256(char *password, byte *pwd_hash)
{
  // sha256_password requires the same scramble as caching_sha2_password for the fast auth path.
  return scramble_password_caching_sha2(password, pwd_hash);
}

/*
  wait_for_bytes - Wait until data is available for reading

  This method is used to permit the connector to respond to servers
  that have high latency or execute long queries. The timeout is
  set by ESP32_MYSQL_DATA_TIMEOUT. Adjust this value to match the performance of
  your server and network.

  It is also used to read how many bytes in total are available from the
  server. Thus, it can be used to know how large a data burst is from
  the server.

  bytes_need[in]    Bytes count to wait for

  Returns integer - Number of bytes available to read.
*/
int MySQL_Packet::wait_for_bytes(const int& bytes_need)
{
  const long wait_till = millis() + ESP32_MYSQL_DATA_TIMEOUT;
  int num = 0;

  long now = 0;

  do
  {
    if ( (now == 0) || ( millis() - now ) > ESP32_MYSQL_WAIT_INTERVAL )
    {
      now = millis();
      num = client->available();

      ESP32_MYSQL_LOGLEVEL5_3("MySQL_Packet::wait_for_bytes: Num bytes= ", num, ", need bytes= ", bytes_need);

      if (num >= bytes_need)
        break;
    }
    
    yield();
    //delay(0);
  } while (now < wait_till);

  if (num == 0 && now >= wait_till)
  {
    ESP32_MYSQL_LOGDEBUG("MySQL_Packet::wait_for_bytes: client->stop");

    //client->stop();
  }

  ESP32_MYSQL_LOGDEBUG1("MySQL_Packet::wait_for_bytes: OK, Num bytes= ", num);
  //////

  return num;
}

/*
  read_packet - Read a packet from the server and store it in the buffer

  This method reads the bytes sent by the server as a packet. All packets
  have a packet header defined as follows.

  Bytes                 Name
  -----                 ----
  3                     Packet Length
  1                     Packet Number

  Thus, the length of the packet (not including the packet header) can
  be found by reading the first 4 bytes from the server then reading
  N bytes for the packet payload.
*/

// TODO: Pass buffer pointer instead of using global buffer

bool MySQL_Packet::read_packet()
{
  #define PACKET_HEADER_SZ      4
  
  byte local[PACKET_HEADER_SZ];
  
  ESP32_MYSQL_LOGLEVEL5("MySQL_Packet::read_packet: step 1");
  
  if ( largest_buffer_size > 0 )
    memset(buffer, 0, largest_buffer_size);

  // Read packet header
  if (!read_bytes(local, PACKET_HEADER_SZ))
  {
    packet_len = 0;
    ESP32_MYSQL_LOGINFO1("MySQL_Packet::read_packet: ", READ_TIMEOUT);
    
    return false;
  }

  ESP32_MYSQL_LOGLEVEL5("MySQL_Packet::read_packet: step 2");

  packet_len = 0;

  // Get packet length
  packet_len = local[0];
  packet_len += (local[1] << 8);
  packet_len += ((uint32_t)local[2] << 16);

  // We must wait for slow arriving packets for Ethernet shields only.
  /*
    if (wait_for_bytes(packet_len) < packet_len) 
    {
      ESP32_MYSQL_LOGERROR(READ_TIMEOUT);
      return false;
    }
  */

  ESP32_MYSQL_LOGINFO1("MySQL_Packet::read_packet: packet_len= ", packet_len);

  // Check for valid packet.
  if ( (packet_len < 0) || ( packet_len > MAX_TRANSMISSION_UNIT ) )
  {
    ESP32_MYSQL_LOGERROR(PACKET_ERROR);
    packet_len = 0;
    
    return false;
  }

  if ( largest_buffer_size < packet_len + PACKET_HEADER_SZ )
  {
    if (largest_buffer_size == 0 )
    {
      // Check if we need to allocate buffer the first time
      largest_buffer_size = packet_len + PACKET_HEADER_SZ;
      ESP32_MYSQL_LOGINFO1("MySQL_Packet::read_packet: First time allocate buffer, size = ", largest_buffer_size);
      
      buffer = (byte *) malloc(largest_buffer_size);
    }
    else
    {
      // Check if we need to reallocate buffer
      largest_buffer_size = packet_len + PACKET_HEADER_SZ;
      ESP32_MYSQL_LOGINFO1("MySQL_Packet::read_packet: Reallocate buffer, size = ", largest_buffer_size);
      
      buffer = (byte *) realloc(buffer, largest_buffer_size);
    }
  }
  
  if (buffer == NULL)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::read_packet: NULL buffer");
    largest_buffer_size = 0;
    
    return false;
  }
  else
  {
    memset(buffer, 0, largest_buffer_size);
  }

  memcpy(buffer, local, PACKET_HEADER_SZ);

  if (packet_len > 0)
  {
    if (!read_bytes(buffer + PACKET_HEADER_SZ, packet_len))
    {
      ESP32_MYSQL_LOGERROR("MySQL_Packet::read_packet: failed reading payload");
      return false;
    }
  }

  ESP32_MYSQL_LOGDEBUG("MySQL_Packet::read_packet: exit");
  
  return true;
}


/*
  parse_handshake_packet - Decipher the server's challenge data

  This method reads the server version string and the seed from the
  server. The handshake packet is defined as follows.

   Bytes                        Name
   -----                        ----
   1                            protocol_version
   n (Null-Terminated String)   server_version
   4                            thread_id
   8                            scramble_buff
   1                            (filler) always 0x00
   2                            server_capabilities
   1                            server_language
   2                            server_status
   2                            server capabilities (two upper bytes)
   1                            length of the scramble seed
  10                            (filler)  always 0
   n                            rest of the plugin provided data
                                (at least 12 bytes)
   1                            \0 byte, terminating the second part of
                                 a scramble seed
*/

void MySQL_Packet::parse_handshake_packet()
{
  if (!buffer)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::parse_handshake_packet: NULL buffer");
    return;
  }

  // Reset state from any previous handshake
  memset(seed, 0, sizeof(seed));
  memset(auth_plugin, 0, sizeof(auth_plugin));
  auth_plugin_type = AUTH_MYSQL_NATIVE_PASSWORD;
  auth_plugin_data_len = 0;
  server_capabilities = 0;

  // Payload starts after the 4-byte packet header
  size_t offset = 4;
  const size_t end = packet_len + 4;

  if (offset >= end)
    return;

  // Skip protocol version
  offset++;

  // Read server version string (null-terminated)
  size_t version_start = offset;

  while ((offset < end) && (buffer[offset] != 0x00))
    offset++;

  if (offset > version_start)
  {
    size_t ver_len = offset - version_start;

    if (server_version)
    {
      free(server_version);
      server_version = NULL;
    }

    server_version = (char *) malloc(ver_len + 1);

    if (server_version)
    {
      memcpy(server_version, &buffer[version_start], ver_len);
      server_version[ver_len] = 0;
    }
  }

  // Skip the null terminator
  offset++;

  if (offset + 4 > end)
    return;

  // Thread id
  offset += 4;

  // Scramble part 1 (8 bytes)
  for (int j = 0; (j < 8) && (offset + j < end); j++)
  {
    seed[j] = buffer[offset + j];
  }

  offset += 8;

  // Filler
  offset++;

  if (offset + 2 > end)
    return;

  server_capabilities = buffer[offset] | (buffer[offset + 1] << 8);
  offset += 2;

  // Charset
  offset++;

  // Status flags
  offset += 2;

  if (offset + 2 > end)
    return;

  server_capabilities |= ((uint32_t) (buffer[offset] | (buffer[offset + 1] << 8))) << 16;
  offset += 2;

  if (offset < end)
  {
    auth_plugin_data_len = buffer[offset];
    offset++;
  }

  // Reserved bytes
  offset += 10;

  size_t second_seed_len = (auth_plugin_data_len > 0) ? max((int) auth_plugin_data_len - 8, 12) : 12;
  const size_t available_seed_bytes = (offset < end) ? min(second_seed_len, end - offset) : 0;

  for (size_t j = 0; (j < 12) && (j < available_seed_bytes); j++)
  {
    seed[j + 8] = buffer[offset + j];
  }

  offset += available_seed_bytes;

  if (offset < end)
  {
    size_t plugin_start = offset;

    while ((offset < end) && (buffer[offset] != 0x00))
      offset++;

    size_t plugin_len = (offset > plugin_start) ? min((size_t) (sizeof(auth_plugin) - 1), offset - plugin_start) : 0;

    if (plugin_len > 0)
    {
      memcpy(auth_plugin, &buffer[plugin_start], plugin_len);
      auth_plugin[plugin_len] = 0;
    }
  }

  if (auth_plugin[0] == 0)
  {
    strncpy(auth_plugin, "mysql_native_password", sizeof(auth_plugin) - 1);
    auth_plugin[sizeof(auth_plugin) - 1] = 0;
  }

  auth_plugin_type = plugin_from_name(auth_plugin);
  ESP32_MYSQL_LOGINFO1("Auth plugin from server:", auth_plugin);
}

/*
  parse_error_packet - Display the error returned from the server

  This method parses an error packet from the server and displays the
  error code and text via Serial.print. The error packet is defined
  as follows.

  Note: the error packet is already stored in the buffer since this
        packet is not an expected response.

  Bytes                       Name
  -----                       ----
  1                           field_count, always = 0xff
  2                           errno
  1                           (sqlstate marker), always '#'
  5                           sqlstate (5 characters)
  n                           message
*/

void MySQL_Packet::parse_error_packet() 
{
  ESP32_MYSQL_LOGDEBUG2("Error: ", read_int(5, 2), " = ");

  if (!buffer)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::parse_error_packet: NULL buffer");
    return;
  }

  for (int i = 0; i < packet_len - 9; i++)
  {
    ESP32_MYSQL_LOGDEBUG0((char)buffer[i + 13]);
  }
    
  ESP32_MYSQL_LOGDEBUG0LN(".");
}

AuthPlugin MySQL_Packet::plugin_from_name(const char *name) const
{
  if (!name || (strlen(name) == 0))
    return AUTH_UNKNOWN;

  if (strcmp(name, "mysql_native_password") == 0)
    return AUTH_MYSQL_NATIVE_PASSWORD;

  if (strcmp(name, "caching_sha2_password") == 0)
    return AUTH_CACHING_SHA2_PASSWORD;

  if (strcmp(name, "sha256_password") == 0)
    return AUTH_SHA256_PASSWORD;

  return AUTH_UNKNOWN;
}


/*
  get_packet_type - Returns the packet type received from the server.

   Bytes                       Name
   -----                       ----
   1   (Length Coded Binary)   field_count, always = 0
   1-9 (Length Coded Binary)   affected_rows
   1-9 (Length Coded Binary)   insert_id
   2                           server_status
   2                           warning_count
   n   (until end of packet)   message

  Returns integer - 0 = successful parse, packet type if not an Ok packet
*/

int MySQL_Packet::get_packet_type() 
{
  if (!buffer)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::get_packet_type: NULL buffer");
    return -1;
  }

  int type = buffer[4];
  
  ESP32_MYSQL_LOGDEBUG1("MySQL_Packet::get_packet_type: packet type= ", type);

  if (type == ESP32_MYSQL_OK_PACKET)
  {
    ESP32_MYSQL_LOGDEBUG("MySQL_Packet::get_packet_type: packet type= ESP32_MYSQL_OK_PACKET");
  }
  else if (type == ESP32_MYSQL_EOF_PACKET)
  {
    ESP32_MYSQL_LOGDEBUG("MySQL_Packet::get_packet_type: packet type= ESP32_MYSQL_EOF_PACKET");
  }
  else if (type == ESP32_MYSQL_ERROR_PACKET)
  {
    ESP32_MYSQL_LOGDEBUG("MySQL_Packet::get_packet_type: packet type= ESP32_MYSQL_ERROR_PACKET");
  }
  else
  {
    ESP32_MYSQL_LOGDEBUG("MySQL_Packet::get_packet_type: Packet Type Error");
  }

  return type;
}


/*
  get_lcb_len - Retrieves the length of a length coded binary value

  This reads the first byte from the offset into the buffer and returns
  the number of bytes (size) that the integer consumes. It is used in
  conjunction with read_int() to read length coded binary integers
  from the buffer.

  Returns integer - number of bytes integer consumes
*/

int MySQL_Packet::get_lcb_len(const int& offset) 
{
  if (!buffer)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::get_lcb_len: NULL buffer");
    return 0;
  }

  int read_len = buffer[offset];
  
  if (read_len > 250) 
  {
    // read type:
    byte type = buffer[offset + 1];
    
    if (type == 0xfc)
      read_len = 2;
    else if (type == 0xfd)
      read_len = 3;
    else if (type == 0xfe)
      read_len = 8;
  } 
  else 
  {
    read_len = 1;
  }

  ESP32_MYSQL_LOGDEBUG1("MySQL_Packet::get_lcb_len: read_len= ", read_len);

  return read_len;
}

/*
  read_int - Retrieve an integer from the buffer in size bytes.

  This reads an integer from the buffer at offset position indicated for
  the number of bytes specified (size).

  offset[in]      offset from start of buffer
  size[in]        number of bytes to use to store the integer

  Returns integer - integer from the buffer
*/

int MySQL_Packet::read_int(const int& offset, const int& size) 
{
  int value = 0;
  int new_size = 0;
  
  if (!buffer)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::read_int: NULL buffer");
    return -1;
  }
    
  if (size == 0)
    new_size = get_lcb_len(offset);
    
  if (size == 1)
    return buffer[offset];
    
  new_size = size;
  int shifter = (new_size - 1) * 8;
  
  for (int i = new_size; i > 0; i--) 
  {
    value += (buffer[i - 1] << shifter);
    shifter -= 8;
  }
  
  return value;
}


/*
  store_int - Store an integer value into a byte array of size bytes.

  This writes an integer into the buffer at the current position of the
  buffer. It will transform an integer of size to a length coded binary
  form where 1-3 bytes are used to store the value (set by size).

  buff[in]        pointer to location in internal buffer where the
                  integer will be stored
  value[in]       integer value to be stored
  size[in]        number of bytes to use to store the integer
*/
void MySQL_Packet::store_int(byte *buff, const long& value, const int& size) 
{
  if (!buff)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::store_int: NULL buffer");
    return;
  }

  memset(buff, 0, size);
  
  if (value <= 0xff)
    buff[0] = (byte)value;
  else if (value <= 0xffff) 
  {
    buff[0] = (byte)value;
    buff[1] = (byte)(value >> 8);
  } 
  else if (value <= 0xffffff) 
  {
    buff[0] = (byte)value;
    buff[1] = (byte)(value >> 8);
    buff[2] = (byte)(value >> 16);
  } 
  else if (value > 0xffffff) 
  {
    buff[0] = (byte)value;
    buff[1] = (byte)(value >> 8);
    buff[2] = (byte)(value >> 16);
    buff[3] = (byte)(value >> 24);
  }
}

/*
  read_lcb_int - Read an integer with len encoded byte

  This reads an integer from the buffer looking at the first byte in the offset
  as the encoded length of the integer.

  offset[in]      offset from start of buffer

  Returns integer - integer from the buffer
*/

int MySQL_Packet::read_lcb_int(const int& offset) 
{
  int len_size = 0;
  int value = 0;
  
  if (!buffer)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::read_lcb_int: NULL buffer");
    return -1;
  }
    
  len_size = buffer[offset];
  
  if (len_size < 252) 
  {
    return buffer[offset];
  } 
  else if (len_size == 252) 
  {
    len_size = 2;
  } 
  else if (len_size == 253) 
  {
    len_size = 3;
  } 
  else 
  {
    len_size = 8;
  }
  
  int shifter = (len_size - 1) * 8;
  
  for (int i = len_size; i > 0; i--) 
  {
    value += (buffer[offset + i] << shifter);
    shifter -= 8;
  }
  
  return value;
}

/*
  print_packet - Print the contents of a packet via Serial.print

  This method is a diagnostic method. It is best used to decipher a
  packet from the server (or before being sent to the server). If you
  are looking for additional program memory space, you can safely
  delete this method.
*/

void MySQL_Packet::print_packet() 
{
  if (!buffer)
  {
    ESP32_MYSQL_LOGERROR("MySQL_Packet::print_packet: NULL buffer");
    return;
  }

  ESP32_MYSQL_LOGDEBUG3("Packet: ", buffer[3], " contains no. bytes = ", packet_len + 3);

  ESP32_MYSQL_LOGDEBUG0("  HEX: ");
  
  for (int i = 0; i < packet_len + 3; i++) 
  {
    ESP32_MYSQL_LOGDEBUG0(String(buffer[i], HEX));
    ESP32_MYSQL_LOGDEBUG0(" ");
  }
  
  ESP32_MYSQL_LOGDEBUG0LN("");
  
  ESP32_MYSQL_LOGDEBUG0("ASCII: ");
  
  for (int i = 0; i < packet_len + 3; i++)
    ESP32_MYSQL_LOGDEBUG0((char)buffer[i]);
    
  ESP32_MYSQL_LOGDEBUG0LN("");
}

#endif    // ESP32_MYSQL_PACKET_IMPL_H
