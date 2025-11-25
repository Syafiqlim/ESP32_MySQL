/*
 * Minimal auth test against a user configured with mysql_native_password.
 */

#include "Credentials.h"

#define ESP32_MYSQL_DEBUG_PORT      Serial
#define _ESP32_MYSQL_LOGLEVEL_      1

#include <ESP32_MySQL.h>

#define USING_HOST_NAME true

#if USING_HOST_NAME
  char server[] = "your-db-host.example.com";   // change to your server's hostname/URL
#else
  IPAddress server(192, 168, 1, 10);            // change to your server's IP
#endif

uint16_t server_port = 3306;    // MySQL server port

ESP32_MySQL_Connection conn((Client *)&client);

const char *authPluginName(AuthPlugin plugin)
{
  switch (plugin)
  {
    case AUTH_MYSQL_NATIVE_PASSWORD: return "mysql_native_password";
    case AUTH_CACHING_SHA2_PASSWORD: return "caching_sha2_password";
    case AUTH_SHA256_PASSWORD:       return "sha256_password";
    default:                         return "unknown";
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  Serial.println("\nAuth test (mysql_native_password user)");

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  Serial.println("\nAttempting MySQL connection");

  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
  {
    Serial.print("Server auth plugin advertised: ");
    Serial.println(authPluginName(conn.get_auth_plugin()));
    Serial.println("Login succeeded with mysql_native_password user.");
    conn.close();
  }
  else
  {
    Serial.println("Connect failed.");
  }

  delay(10000);
}
