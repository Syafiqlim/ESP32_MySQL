/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/*********************************************************************************************************************************
  Basic_Insert_ESP32MySQL.ino
  by Syafiqlim @ syafiqlimx

 **********************************************************************************************************************************/
/*
  INSTRUCTIONS FOR USE

  1) Change the address of the server to the IP address of the MySQL server in Credentials.h
  2) Change the user and password to a valid MySQL user and password in Credentials.h
  3) Change the SSID and pass to match your WiFi network in Credentials.h
  4) Change the default DB, default table, default column and default value according to your DB schema
  5) Connect a USB cable to your ESP32
  6) Select the correct board and port
  7) Compile and upload the sketch to your ESP32
  8) Once uploaded, open Serial Monitor (use 115200 speed) and observe

*/

#include "Credentials.h"

#define ESP32_MYSQL_DEBUG_PORT      Serial

// Debug Level from 0 to 4
#define _ESP32_MYSQL_LOGLEVEL_      1

#include <ESP32_MySQL.h>

#define USING_HOST_NAME     true

#if USING_HOST_NAME
  // Optional using hostname
  char server[] = "xxxxxx.com"; // change to your server's hostname/URL
#else
  IPAddress server(128, 100, 001, 010);
#endif

uint16_t server_port = 14014;    // MySQL server port (default : 3306)

char default_database[] = "DB0";           //default DB
char default_table[]    = "TEST0x00";          //default table

char default_column[] = "data0";   //default column

String default_value    = "sayangAin";    //default value (make sure datatype same with column of DB table, like you can use String for VARCHAR)

// Sample query
String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table 
                 + " (" + default_column + ")" + " VALUES ('" + default_value + "')";

ESP32_MySQL_Connection conn((Client *)&client);

ESP32_MySQL_Query *query_mem;

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 5000); // wait for serial port to connect

  ESP32_MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);

  // Begin WiFi section
  ESP32_MYSQL_DISPLAY1("Connecting to", ssid);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    ESP32_MYSQL_DISPLAY0(".");
  }

  // print out info about the connection:
  ESP32_MYSQL_DISPLAY1("Connected to network. My IP address is:", WiFi.localIP());

  ESP32_MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
  ESP32_MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);
}

void runInsert()
{
  // Initiate the query class instance
  ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&conn);

  if (conn.connected())
  {
    ESP32_MYSQL_DISPLAY(INSERT_SQL);
    
    // Execute the query
    if ( !query_mem.execute(INSERT_SQL.c_str()) )
    {
      ESP32_MYSQL_DISPLAY("Insert error");
    }
    else
    {
      ESP32_MYSQL_DISPLAY("Data Inserted.");
    }
  }
  else
  {
    ESP32_MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }
}

void loop()
{
  ESP32_MYSQL_DISPLAY("Connecting...");
  
  //if (conn.connect(server, server_port, user, password))
  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
  {
    delay(500);
    runInsert();
    conn.close();                     // close the connection
  } 
  else 
  {
    ESP32_MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
  }

  ESP32_MYSQL_DISPLAY("\nSleeping...");
  ESP32_MYSQL_DISPLAY("================================================");
 
  delay(10000); // every 10 seconds
}
