/**************************** 
  ESP32_MySQL.h
  by Syafiqlim @ syafiqlimx
*****************************/

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
