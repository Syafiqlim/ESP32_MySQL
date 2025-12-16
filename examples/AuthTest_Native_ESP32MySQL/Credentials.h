/*
 * Provide your WiFi and MySQL credentials here.
 * Use a MySQL user created with:
 *   CREATE USER 'user'@'%' IDENTIFIED WITH mysql_native_password BY 'password';
 */

#ifndef Credentials_h
#define Credentials_h

char ssid[] = "xxxxx";       // your network SSID (name)
char pass[] = "xxxxx";       // your network password

char user[]     = "native_user";   // MySQL username (mysql_native_password)
char password[] = "native_pass";   // MySQL password

#endif    // Credentials_h
