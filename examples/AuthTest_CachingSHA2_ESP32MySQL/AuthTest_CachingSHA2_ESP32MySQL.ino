/*
 * Minimal auth test against a user configured with caching_sha2_password.
 * Uses TLS so both fast-auth and full-auth (password in TLS tunnel) work.
 */

#include "Credentials.h"

#define ESP32_MYSQL_DEBUG_PORT      Serial
#define _ESP32_MYSQL_LOGLEVEL_      1

#include <ESP32_MySQL.h>

#define USING_HOST_NAME true
 //true pour TLS, false pour fallback RSA
#define USE_TLS         true
 //true pour authentifier le serveur (recommande) : certificat en TLS, cle RSA epinglee sans TLS
 //false = chiffre mais sans authentification (interception possible)
#define VERIFY_SERVER   true
 //false si le certificat ne contient pas le nom/IP du serveur (ex: certificats auto-generes par MySQL)
#define VERIFY_HOSTNAME true

#if USE_TLS && VERIFY_SERVER
// CA that signed the MySQL server certificate (for a default install: ca.pem in the MySQL data dir)
const char ca_cert[] = R"EOF(
-----BEGIN CERTIFICATE-----
...paste your CA certificate here...
-----END CERTIFICATE-----
)EOF";
#elif VERIFY_SERVER
// Server RSA public key for caching_sha2_password (public_key.pem in the MySQL data dir, or
// SHOW STATUS LIKE 'Caching_sha2_password_rsa_public_key';)
const char server_pubkey[] = R"EOF(
-----BEGIN PUBLIC KEY-----
...paste the server public key here...
-----END PUBLIC KEY-----
)EOF";
#endif

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

#if USE_TLS
  #if USING_HOST_NAME
    conn.enable_tls(true, server);   // Request TLS and pass hostname for SNI
  #else
    conn.enable_tls(true);
  #endif
  #if VERIFY_SERVER
    conn.set_ca_cert(ca_cert);       // Verify the server certificate (no plaintext fallback)
    conn.set_tls_verify_hostname(VERIFY_HOSTNAME);
  #endif
#elif VERIFY_SERVER
  conn.set_server_public_key(server_pubkey);   // RSA key never requested from the server
#endif

  Serial.println("\nAuth test (caching_sha2_password user)");

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected. IP: ");
  Serial.println(WiFi.localIP());

#if USE_TLS && VERIFY_SERVER
  // Certificate validity dates are checked: the clock must be set first
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time");
  while (time(nullptr) < 1700000000)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
#endif
}

void loop()
{
  Serial.println("\nAttempting MySQL connection");

  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
  {
    Serial.print("Server auth plugin advertised: ");
    Serial.println(authPluginName(conn.get_auth_plugin()));
    Serial.println("Login succeeded on caching_sha2_password fast path.");
    conn.close();
  }
  else
  {
    Serial.println("Connect failed. Check TLS/RSA availability on server and credentials.");
  }

  delay(10000);
}
