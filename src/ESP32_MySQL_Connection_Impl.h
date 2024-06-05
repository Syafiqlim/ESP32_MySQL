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

  send_authentication_packet(user, password, db);
   
  if ( !read_packet() )
  {
    ESP32_MYSQL_LOGERROR("Can't connect. Error reading auth packets");
  }
	else if (get_packet_type() != ESP32_MYSQL_OK_PACKET)
  {
    parse_error_packet();
  }
  else
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

  send_authentication_packet(user, password, db);
   
  if ( !read_packet() )
  {
    ESP32_MYSQL_LOGERROR("Can't connect. Error reading auth packets");
  }
	else if (get_packet_type() != ESP32_MYSQL_OK_PACKET)
  {
    parse_error_packet();
  }
  else
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
    
    ESP32_MYSQL_LOGERROR("Disconnected");
  }
}

#endif    // ESP32_MySQL_Connection_IMPL_H
