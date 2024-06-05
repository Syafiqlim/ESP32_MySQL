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

#define ESP32_MYSQL_OK_PACKET         0x00
#define ESP32_MYSQL_EOF_PACKET        0xfe
#define ESP32_MYSQL_ERROR_PACKET      0xff

#define MAX_TRANSMISSION_UNIT   1500
//////

class MySQL_Packet 
{
  public:
    byte *buffer;           // buffer for reading packets
    
    uint16_t largest_buffer_size = 0;
    //////
    
    int packet_len;         // length of current packet
    Client *client;         // instance of client class (e.g. EthernetClient)
    char *server_version;   // save server version from handshake

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
    };
    
    bool    complete_handshake(char *user, char *password);
    void    send_authentication_packet(char *user, char *password, char *db = NULL);
    void    parse_handshake_packet();
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
};



#endif    // ESP32_MYSQL_PACKET_H
