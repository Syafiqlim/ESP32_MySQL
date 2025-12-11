/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Connection_Impl.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once
 
#ifndef ESP32_MySQL_Connection_IMPL_H
#define ESP32_MySQL_Connection_IMPL_H

#include <Arduino.h>

#include <ESP32_MySQL_Encrypt_Sha1.h>

#define MAX_CONNECT_ATTEMPTS      10
#define CONNECT_DELAY_MS          1000
#define SUCCESS                   1

/*
  connect - Connect to a MySQL server.

  This method is used to connect to a MySQL server. It will attempt to
  connect to the server as a client retrying up to MAX_CONNECT_ATTEMPTS.
  This permits the possibility of longer than normal network lag times
  for wireless networks. You can adjust MAX_CONNECT_ATTEMPTS to suit
  your environment.

  server[in]      IP address of the server as IPAddress type
  port[in]        port number of the server
  user[in]        user name
  password[in]    (optional) user password
  db[in]          (optional) default database

  Returns bool - True = connection succeeded
*/


String SQL_IPAddressToString(const IPAddress& _address)
{
  String str = String(_address[0]);
  str += ".";
  str += String(_address[1]);
  str += ".";
  str += String(_address[2]);
  str += ".";
  str += String(_address[3]);
  
  return str;
}

//////////////////////////////////////////////////////////////

bool ESP32_MySQL_Connection::connect(const char *hostname, const uint16_t& port, char *user, char *password, char *db)
{
  int  connected 	= 0;
  int  retries 		= 0;
  bool returnVal 	= false;
  
  ESP32_MYSQL_LOGWARN3("Connecting to Server:", hostname, ", Port = ", port);
  
  if (db)
    ESP32_MYSQL_LOGWARN1("Using Database:", db);

  reset_for_connect();
  if (wants_tls())
    enable_tls(true, hostname);
  cache_password(password);
  cache_password(password);
  if (wants_tls())
    enable_tls(true, hostname);

  reset_for_connect();

  // Retry up to MAX_CONNECT_ATTEMPTS times.
  while (retries++ < MAX_CONNECT_ATTEMPTS)
  {
    connected = client->connect(hostname, port);
    
    ESP32_MYSQL_LOGDEBUG1("connected =", connected);

    if (connected != SUCCESS)
    {
      ESP32_MYSQL_LOGDEBUG1("Can't connect. Retry #", retries);
      delay(CONNECT_DELAY_MS);
    }
    else
    {
      break;
    }
  }

  if (connected != SUCCESS)
    return false;

  ESP32_MYSQL_LOGINFO("Connect OK. Try reading packets");

  if ( !read_packet() )
  {
    ESP32_MYSQL_LOGERROR("Can't connect. Error reading packets");
    return false;
  }

  ESP32_MYSQL_LOGINFO("Try parsing packets");

  parse_handshake_packet();

  ESP32_MYSQL_LOGINFO("Try send_authentication packets");

  const bool tls_possible = wants_tls() && (server_capabilities & CLIENT_SSL);
  uint32_t client_flags = build_client_flags(tls_possible);
  uint8_t auth_sequence_id = 0x01;

  if (tls_possible)
  {
    if (!send_ssl_request(client_flags, auth_sequence_id))
    {
      ESP32_MYSQL_LOGERROR("Failed to send SSL Request packet");
      return false;
    }

    auth_sequence_id = get_next_sequence_id();

    if (!start_tls_handshake())
    {
      ESP32_MYSQL_LOGERROR("TLS handshake failed");
      return false;
    }
  }
  else if (wants_tls() && !tls_possible)
  {
    ESP32_MYSQL_LOGWARN("Server does not advertise SSL support, continuing without TLS");
  }

  send_authentication_packet(user, password, db, client_flags, auth_sequence_id);
   
  if ( !read_packet() )
  {
    ESP32_MYSQL_LOGERROR("Can't connect. Error reading auth packets");
  }
  else if (handle_authentication_result())
  {
    ESP32_MYSQL_LOGWARN1("Connected. Server Version =", server_version);
    returnVal = true;
  }

	if (server_version)
	{
		free(server_version); // don't need it anymore
		server_version = NULL;
	}

  return returnVal;
}

//////////////////////////////////////////////////////////////

Connection_Result ESP32_MySQL_Connection::connectNonBlocking(const char *hostname, const uint16_t& port, char *user, char *password, char *db)
{
  int  connected 	= 0;
  int  retries 		= 0;
  
  Connection_Result returnVal = RESULT_FAIL;
  
  long now = 0;
  
  ESP32_MYSQL_LOGWARN3("Connecting to Server:", hostname, ", Port = ", port);
  
  if (db)
    ESP32_MYSQL_LOGWARN1("Using Database:", db);
  
  while (retries < MAX_CONNECT_ATTEMPTS)
  {  
    if ( (now == 0) || ( millis() - now ) > CONNECT_DELAY_MS )
    {
      now = millis();
      
      connected = client->connect(hostname, port);
      
      retries++;
      
      ESP32_MYSQL_LOGDEBUG1("connected =", connected);

      if (connected == SUCCESS)
      {
        break;
      }
      else
      {
        ESP32_MYSQL_LOGDEBUG1("Can't connect. Retry #", retries);
      }     
    }
    else
    {
      //delay(CONNECT_DELAY_MS);
      yield();
    }
  }

  if (connected != SUCCESS)
    return RESULT_FAIL;

  ESP32_MYSQL_LOGINFO("Connect OK. Try reading packets");

  if ( !read_packet() )
  {
    ESP32_MYSQL_LOGERROR("Can't connect. Error reading packets");
    return RESULT_FAIL;
  }

  ESP32_MYSQL_LOGINFO("Try parsing packets");

  parse_handshake_packet();

  ESP32_MYSQL_LOGINFO("Try send_authentication packets");

  const bool tls_possible = wants_tls() && (server_capabilities & CLIENT_SSL);
  uint32_t client_flags = build_client_flags(tls_possible);
  uint8_t auth_sequence_id = 0x01;

  if (tls_possible)
  {
    if (!send_ssl_request(client_flags, auth_sequence_id))
    {
      ESP32_MYSQL_LOGERROR("Failed to send SSL Request packet");
      return RESULT_FAIL;
    }

    auth_sequence_id = get_next_sequence_id();

    if (!start_tls_handshake())
    {
      ESP32_MYSQL_LOGERROR("TLS handshake failed");
      return RESULT_FAIL;
    }
  }
  else if (wants_tls() && !tls_possible)
  {
    ESP32_MYSQL_LOGWARN("Server does not advertise SSL support, continuing without TLS");
  }

  send_authentication_packet(user, password, db, client_flags, auth_sequence_id);
   
  if ( !read_packet() )
  {
    ESP32_MYSQL_LOGERROR("Can't connect. Error reading auth packets");
  }
  else if (handle_authentication_result())
  {
    ESP32_MYSQL_LOGWARN1("Connected. Server Version =", server_version);
    returnVal = RESULT_OK;
  }

	if (server_version)
	{
		free(server_version); // don't need it anymore
		server_version = NULL;
	}

  return returnVal;
}

//////////////////////////////////////////////////////////////

bool ESP32_MySQL_Connection::connect(const IPAddress& server, const uint16_t& port, char *user, char *password, char *db)
{
	return connect(SQL_IPAddressToString(server).c_str(), port, user, password, db);
}

//////////////////////////////////////////////////////////////

Connection_Result ESP32_MySQL_Connection::connectNonBlocking(const IPAddress& server, const uint16_t& port, char *user, char *password, char *db)
{
	return connectNonBlocking(SQL_IPAddressToString(server).c_str(), port, user, password, db);
}

//////////////////////////////////////////////////////////////

bool ESP32_MySQL_Connection::handle_authentication_result()
{
  const int type = get_packet_type();

  if (type == ESP32_MYSQL_OK_PACKET)
    return true;

  if (type == ESP32_MYSQL_ERROR_PACKET)
  {
    parse_error_packet();
    return false;
  }

  if ((auth_plugin_type == AUTH_CACHING_SHA2_PASSWORD) && buffer && (packet_len >= 2))
  {
    // caching_sha2_password returns small packets with auth stage markers
    if (buffer[4] == 0x01)
    {
      const uint8_t auth_step = buffer[5];

      if (auth_step == 0x03)
      {
        ESP32_MYSQL_LOGINFO("caching_sha2 fast auth accepted, waiting for final OK");

        if (!read_packet())
        {
          ESP32_MYSQL_LOGERROR("Failed reading final OK packet after fast auth");
          return false;
        }

        if (get_packet_type() == ESP32_MYSQL_OK_PACKET)
          return true;

        parse_error_packet();
        return false;
      }
      else if (auth_step == 0x04)
      {
        const char *pwd = get_cached_password();

        if (!pwd)
        {
          ESP32_MYSQL_LOGERROR("No cached password available for full authentication");
          return false;
        }

        if (tls_active())
        {
          const size_t pwd_len = strlen(pwd);
          const size_t payload_len = pwd_len + 1; // null-terminated password
          const size_t packet_len = payload_len + 4;
          uint8_t *packet = (uint8_t *) malloc(packet_len);

          if (!packet)
          {
            ESP32_MYSQL_LOGERROR("Failed to allocate packet for full authentication");
            return false;
          }

          const uint8_t response_seq = buffer ? (uint8_t) (buffer[3] + 1) : get_next_sequence_id();

          store_int(packet, payload_len, 3);
          packet[3] = response_seq;
          memcpy(packet + 4, pwd, pwd_len);
          packet[4 + pwd_len] = 0x00;

          bool wrote = write_bytes(packet, packet_len);
          set_next_sequence_id(response_seq + 1);
          free(packet);

          if (!wrote)
          {
            ESP32_MYSQL_LOGERROR("Failed to send full authentication response over TLS");
            return false;
          }

          if (!read_packet())
          {
            ESP32_MYSQL_LOGERROR("Failed reading final OK packet after full auth");
            return false;
          }

          if (get_packet_type() == ESP32_MYSQL_OK_PACKET)
            return true;

          parse_error_packet();
          return false;
        }
        else
        {
          // Fallback RSA path (no TLS available)
          const uint8_t request_seq = buffer ? (uint8_t) (buffer[3] + 1) : get_next_sequence_id();
          uint8_t request[5];
          store_int(request, 1, 3);
          request[3] = request_seq;
          request[4] = 0x02; // request public key

          if (!write_bytes(request, sizeof(request)))
          {
            ESP32_MYSQL_LOGERROR("Failed to request RSA public key");
            return false;
          }

          set_next_sequence_id(request_seq + 1);

          if (!read_packet())
          {
            ESP32_MYSQL_LOGERROR("Failed reading RSA public key packet");
            return false;
          }

          if ((packet_len <= 0) || !buffer || (packet_len > MAX_TRANSMISSION_UNIT))
          {
            ESP32_MYSQL_LOGERROR("Invalid RSA public key packet");
            return false;
          }

          const uint8_t *pubkey = buffer + 4;
          size_t pubkey_len = packet_len;

          uint8_t encrypted[512];
          size_t encrypted_len = sizeof(encrypted);

          if (!encrypt_password_rsa(pubkey, pubkey_len, pwd, encrypted, &encrypted_len))
          {
            ESP32_MYSQL_LOGERROR("RSA encryption failed");
            return false;
          }

          const uint8_t response_seq = buffer ? (uint8_t) (buffer[3] + 1) : get_next_sequence_id();
          const size_t payload_len = encrypted_len;
          const size_t packet_len_out = payload_len + 4;
          uint8_t *packet = (uint8_t *) malloc(packet_len_out);

          if (!packet)
          {
            ESP32_MYSQL_LOGERROR("Failed to allocate RSA auth packet");
            return false;
          }

          store_int(packet, payload_len, 3);
          packet[3] = response_seq;
          memcpy(packet + 4, encrypted, encrypted_len);

          bool wrote = write_bytes(packet, packet_len_out);
          set_next_sequence_id(response_seq + 1);
          free(packet);

          if (!wrote)
          {
            ESP32_MYSQL_LOGERROR("Failed to send RSA full authentication response");
            return false;
          }

          if (!read_packet())
          {
            ESP32_MYSQL_LOGERROR("Failed reading final OK packet after RSA full auth");
            return false;
          }

          if (get_packet_type() == ESP32_MYSQL_OK_PACKET)
            return true;

          parse_error_packet();
          return false;
        }
      }
    }
  }

  ESP32_MYSQL_LOGERROR1("Unexpected auth response, packet type =", type);
  return false;
}

//////////////////////////////////////////////////////////////

/*
  close - cancel the connection

  This method closes the connection to the server and frees up any memory
  used in the buffer.
*/
void ESP32_MySQL_Connection::close()
{
  if (connected())
  {
    client->flush();
    client->stop();
    
    reset_for_connect();
    ESP32_MYSQL_LOGERROR("Disconnected");
  }
}

#endif    // ESP32_MySQL_Connection_IMPL_H
