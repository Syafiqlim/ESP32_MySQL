/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/****************************************************** 
  ESP32_MySQL.h
  by Syafiqlim @ syafiqlimx

  modification :
  v1.0.0 -
  Add #include <ESP32_MySQL_Sha256.h>
  Add #include <ESP32_MySQL_Aes256_Impl.h>
*******************************************************/

#pragma once

#ifndef ESP32_MYSQL_H
#define ESP32_MYSQL_H

  #warning Using ESP32 built-in WiFi
  #include <WiFi.h>
  WiFiClient client;

#include <ESP32_MySQL.hpp>

#include <ESP32_MySQL_Connection_Impl.h>
#include <ESP32_MySQL_Query_Impl.h>
#include <ESP32_MySQL_Encrypt_Sha1_Impl.h>
#include <ESP32_MySQL_Packet_Impl.h>
#include <ESP32_MySQL_Sha256.h>
#include <ESP32_MySQL_Aes256_Impl.h>
 
#endif    //ESP32_MYSQL_H
