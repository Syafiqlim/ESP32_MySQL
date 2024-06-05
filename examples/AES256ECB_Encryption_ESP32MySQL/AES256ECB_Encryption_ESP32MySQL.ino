/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/*********************************************************************************************************************************
  AES256ECB_Encryption_ESP32MySQL.ino
  by Syafiqlim @ syafiqlimx

 **********************************************************************************************************************************/
/*
  INSTRUCTIONS FOR USE

  1) Change the address of the server to the IP address of the MySQL server in Credentials.h
  2) Change the user and password to a valid MySQL user and password in Credentials.h
  3) Change the SSID and pass to match your WiFi network in Credentials.h
  4) Change the default DB, table, columns and value according to your DB schema
  5) Make sure you have AES-256 key in hex, or you can just generate random 32 bytes (256-bit) of hex
     using any tool, for example openssl
  6) Change with your key, also your desired plaintext/message
  7) Connect a USB cable to your ESP32
  8) Select the correct board and port
  9) Compile and upload the sketch to your ESP32
  10) Once uploaded, open Serial Monitor (use 115200 baudrate) and observe

*/

#include <ESP32_MySQL.h>
#include <ESP32_MySQL_Aes256_Impl.h>

#include <Arduino.h>

#include "Credentials.h"

#define ESP32_MYSQL_DEBUG_PORT      Serial

// Debug Level from 0 to 4
#define _ESP32_MYSQL_LOGLEVEL_      1

#define USING_HOST_NAME     true

#if USING_HOST_NAME
  // hostname or IPv4 address of MySQL server
  char server[] = "xxxxx.com"; // change to your server's hostname/URL
#else
  IPAddress server(128, 100, 001, 010);
#endif

uint16_t server_port = 14014;    // default MySQL port = 3306

char default_database[] = "DB0";           //insert default DB
char default_table[]    = "TEST0x00";          //insert default table

ESP32_MySQL_Connection conn((Client *)&client);

ESP32_MySQL_Query *query_mem;

ESP32_MySQL_AES aes;

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

void encryptAndInsert() {
    // AES key (32 bytes for AES-256), this is just example. You MUST seperate hex to 8-bit (1 Byte) with comma
    byte key[32] = {
        0xdc, 0x01, 0x6f, 0x5c, 0xce, 0x95, 0xfc, 0x3b, 
        0x5f, 0x8c, 0xbd, 0x30, 0xfc, 0xc2, 0xde, 0x88,
        0xf2, 0xe5, 0x03, 0x50, 0xf0, 0x67, 0x43, 0x39, 
        0x57, 0x6b, 0xca, 0x5a, 0x3f, 0xba, 0x7a, 0x31
    };

    aes.init(key);

    // Insert your constat plaintext
    const char* plaintext = "sayangAin";
    String encryptedText = aes.encrypt((const byte*)plaintext, strlen(plaintext));

    // Print the ciphertext in hex
    Serial.println(encryptedText);

    // INSERT the encryptedText into DB
    String INSERT_SQL = "INSERT INTO " + String(default_database) + "." + String(default_table) +
                        " (data0) VALUES ('" + encryptedText + "');";

    ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&conn);

    if (conn.connected()) {
        ESP32_MYSQL_DISPLAY(INSERT_SQL);

        // Execute the query
        if (!query_mem.execute(INSERT_SQL.c_str())) {
            ESP32_MYSQL_DISPLAY("Insert error");
        } else {
            ESP32_MYSQL_DISPLAY("Data Inserted.");
        }
    } else {
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
    encryptAndInsert();
    conn.close();                     // close the connection
  } 
  else 
  {
    ESP32_MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
  }

  ESP32_MYSQL_DISPLAY("\nSleeping...");
  ESP32_MYSQL_DISPLAY("================================================");

  delay(10000);
}
