/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Debug.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once

#ifndef ESP32_MYSQL_DEBUG_H
#define ESP32_MYSQL_DEBUG_H

#include <stdio.h>

const char CONNECTED[]      /*PROGMEM*/ = "Connected";
const char DISCONNECTED[]   /*PROGMEM*/ = "Disconnected.";

const char MEMORY_ERROR[]   /*PROGMEM*/ = "Memory error.";
const char PACKET_ERROR[]   /*PROGMEM*/ = "Packet error.";
const char READ_TIMEOUT[]   /*PROGMEM*/ = "ERROR: Timeout waiting for client.";

const char BAD_MOJO[]       /*PROGMEM*/ = "Bad mojo. EOF found reading column header.";
const char ROWS[]           /*PROGMEM*/ = " rows in result.";
const char READ_COLS[]      /*PROGMEM*/ = "ERROR: You must read the columns first!";
const char NOT_CONNECTED[]  /*PROGMEM*/ = "ERROR: Class requires connected server.";


#ifdef ESP32_MYSQL_DEBUG_PORT
  #define ESP32_MYSQL_DEBUG_OUTPUT ESP32_MYSQL_DEBUG_PORT
#else
  #define ESP32_MYSQL_DEBUG_OUTPUT Serial
#endif

// Change _ESP32_MYSQL_LOGLEVEL_ to set tracing and logging verbosity
// 0: DISABLED: no logging
// 1: ERROR: errors
// 2: WARN: errors and warnings
// 3: INFO: errors, warnings and informational (default)
// 4: DEBUG: errors, warnings, informational and debug

#ifndef _ESP32_MYSQL_LOGLEVEL_
#define _ESP32_MYSQL_LOGLEVEL_       0
#endif

//////////////////////////////////////////

const char ESP32_MYSQL_MARK[] = "[SQL] ";
const char ESP32_MYSQL_SP[]   = " ";

#define ESP32_MYSQL_PRINT         ESP32_MYSQL_DEBUG_OUTPUT.print
#define ESP32_MYSQL_PRINTLN       ESP32_MYSQL_DEBUG_OUTPUT.println

#define ESP32_MYSQL_PRINT_MARK    ESP32_MYSQL_PRINT(ESP32_MYSQL_MARK)
#define ESP32_MYSQL_PRINT_SP      ESP32_MYSQL_PRINT(ESP32_MYSQL_SP)

///////////////////////////////////////////////////

#define ESP32_MYSQL_DISPLAY(x)        { ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_DISPLAY0(x)       { ESP32_MYSQL_PRINT(x); }
#define ESP32_MYSQL_DISPLAY1(x,y)     { ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(y); }
#define ESP32_MYSQL_DISPLAY2(x,y,z)   { ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(z); }
#define ESP32_MYSQL_DISPLAY3(x,y,z,w) { ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(z); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(w); }
#define ESP32_MYSQL_DISPLAY5(x,y,z,w,xx,yy) { ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(z); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(w); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(xx); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(yy); }

///////////////////////////////////////////////////

#define ESP32_MYSQL_LOGERROR(x)        if(_ESP32_MYSQL_LOGLEVEL_>0) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGERROR0(x)       if(_ESP32_MYSQL_LOGLEVEL_>0) { ESP32_MYSQL_PRINT(x); }
#define ESP32_MYSQL_LOGERROR0LN(x)     if(_ESP32_MYSQL_LOGLEVEL_>0) { ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGERROR1(x,y)     if(_ESP32_MYSQL_LOGLEVEL_>0) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(y); }
#define ESP32_MYSQL_LOGERROR2(x,y,z)   if(_ESP32_MYSQL_LOGLEVEL_>0) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(z); }
#define ESP32_MYSQL_LOGERROR3(x,y,z,w) if(_ESP32_MYSQL_LOGLEVEL_>0) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(z); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(w); }

///////////////////////////////////////////////////

#define ESP32_MYSQL_LOGWARN(x)         if(_ESP32_MYSQL_LOGLEVEL_>1) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGWARN0(x)        if(_ESP32_MYSQL_LOGLEVEL_>1) { ESP32_MYSQL_PRINT(x); }
#define ESP32_MYSQL_LOGWARN0LN(x)      if(_ESP32_MYSQL_LOGLEVEL_>1) { ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGWARN1(x,y)      if(_ESP32_MYSQL_LOGLEVEL_>1) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(y); }
#define ESP32_MYSQL_LOGWARN2(x,y,z)    if(_ESP32_MYSQL_LOGLEVEL_>1) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(z); }
#define ESP32_MYSQL_LOGWARN3(x,y,z,w)  if(_ESP32_MYSQL_LOGLEVEL_>1) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(z); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(w); }

///////////////////////////////////////////////////

#define ESP32_MYSQL_LOGINFO(x)         if(_ESP32_MYSQL_LOGLEVEL_>2) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGINFO0(x)        if(_ESP32_MYSQL_LOGLEVEL_>2) { ESP32_MYSQL_PRINT(x); }
#define ESP32_MYSQL_LOGINFO0LN(x)      if(_ESP32_MYSQL_LOGLEVEL_>2) { ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGINFO1(x,y)      if(_ESP32_MYSQL_LOGLEVEL_>2) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(y); }
#define ESP32_MYSQL_LOGINFO2(x,y,z)    if(_ESP32_MYSQL_LOGLEVEL_>2) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(z); }
#define ESP32_MYSQL_LOGINFO3(x,y,z,w)  if(_ESP32_MYSQL_LOGLEVEL_>2) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(z); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(w); }

///////////////////////////////////////////////////

#define ESP32_MYSQL_LOGDEBUG(x)        if(_ESP32_MYSQL_LOGLEVEL_>3) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGDEBUG0(x)       if(_ESP32_MYSQL_LOGLEVEL_>3) { ESP32_MYSQL_PRINT(x); }
#define ESP32_MYSQL_LOGDEBUG0LN(x)     if(_ESP32_MYSQL_LOGLEVEL_>3) { ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGDEBUG1(x,y)     if(_ESP32_MYSQL_LOGLEVEL_>3) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(y); }
#define ESP32_MYSQL_LOGDEBUG2(x,y,z)   if(_ESP32_MYSQL_LOGLEVEL_>3) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(z); }
#define ESP32_MYSQL_LOGDEBUG3(x,y,z,w) if(_ESP32_MYSQL_LOGLEVEL_>3) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(z); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(w); }

///////////////////////////////////////////////////

#define ESP32_MYSQL_LOGLEVEL5(x)        if(_ESP32_MYSQL_LOGLEVEL_>4) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGLEVEL5_0(x)      if(_ESP32_MYSQL_LOGLEVEL_>4) { ESP32_MYSQL_PRINT(x); }
#define ESP32_MYSQL_LOGLEVEL5_0LN(x)    if(_ESP32_MYSQL_LOGLEVEL_>4) { ESP32_MYSQL_PRINTLN(x); }
#define ESP32_MYSQL_LOGLEVEL5_1(x,y)    if(_ESP32_MYSQL_LOGLEVEL_>4) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(y); }
#define ESP32_MYSQL_LOGLEVEL5_2(x,y,z)  if(_ESP32_MYSQL_LOGLEVEL_>4) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(z); }
#define ESP32_MYSQL_LOGLEVEL5_3(x,y,z,w) if(_ESP32_MYSQL_LOGLEVEL_>4) { ESP32_MYSQL_PRINT_MARK; ESP32_MYSQL_PRINT(x); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(y); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINT(z); ESP32_MYSQL_PRINT_SP; ESP32_MYSQL_PRINTLN(w); }

///////////////////////////////////////////////////

#endif    // ESP32_MYSQL_DEBUG_H
