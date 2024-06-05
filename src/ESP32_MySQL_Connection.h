/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Connection.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once
 
#ifndef ESP32_MySQL_Connection_H
#define ESP32_MySQL_Connection_H

#include "ESP32_MySQL_Debug.h"

#include <ESP32_MySQL_Packet.h>

typedef enum 
{
  RESULT_OK     = 0, 
  RESULT_FAIL,
  RESULT_PENDING
} Connection_Result;


class ESP32_MySQL_Connection : public MySQL_Packet 
{
  public:
    ESP32_MySQL_Connection(Client *client_instance) : MySQL_Packet(client_instance) {}
    
    virtual ~ESP32_MySQL_Connection()
    {
	    this->close();
    };
    
    bool connect(const IPAddress& server, const uint16_t& port, char *user, char *password, char *db = NULL);
    
    Connection_Result connectNonBlocking(const IPAddress& server, const uint16_t& port, char *user, char *password, char *db = NULL);
    
    bool connect(const char *hostname, const uint16_t& port, char *user, char *password, char *db = NULL);
    
    Connection_Result connectNonBlocking(const char *hostname, const uint16_t& port, char *user, char *password, char *db = NULL);
    ////////
    
    int connected() 
    {
      return client->connected();
    }
    
    const char *version() 
    {
      return ESP32_MYSQL_GENERIC_VERSION;
    }
    
    void close();
};

//#include <MySQL_Generic_Connection_Impl.h>

#endif    // ESP32_MySQL_Connection_H
