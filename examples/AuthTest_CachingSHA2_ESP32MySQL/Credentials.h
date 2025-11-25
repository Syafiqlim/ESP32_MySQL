/*
 * Provide your WiFi and MySQL credentials here.
 * Use a MySQL user created with:
 *   CREATE USER 'user'@'%' IDENTIFIED WITH caching_sha2_password BY 'password';
 *   GRANT ALL PRIVILEGES ON *.* TO 'user'@'%' WITH GRANT OPTION;
 * Increase the authentication cache by connecting once from a desktop client to warm it.
 */

#ifndef Credentials_h
#define Credentials_h

char ssid[] = "xxxxx";       // your network SSID (name)
char pass[] = "xxxxx";       // your network password

char user[]     = "caching_user";   // MySQL username (caching_sha2_password)
char password[] = "caching_pass";   // MySQL password

#endif    // Credentials_h
