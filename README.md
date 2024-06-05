# ESP32_MySQL

[![arduino-library-badge](https://www.ardu-badge.com/badge/ESP32_MySQL.svg?)](https://www.ardu-badge.com/ESP32_MySQL)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/Syafiqlim/ESP32_MySQL/blob/main/LICENSE)
[![GitHub release](https://img.shields.io/github/release/Syafiqlim/ESP32_MySQL.svg)](https://GitHub.com/Syafiqlim/ESP32_MySQL/releases/)

Optimized library for ESP32 to directly connect and execute SQL to MySQL database WITHOUT any intermediary like HTTP server.

## Table of Contents

- [Overview](#overview)
  - [Background Of ESP32_MySQL Library](#background-of-esp32_mysql-library)
  - [Features](#features)
- [Installation](#installation)
  - [Using Arduino Library Manager](#using-arduino-library-manager)
  - [Manual Installation](#manual-installation)
- [Examples](#examples)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## Overview
### Background Of ESP32_MySQL Library

When I was doing my Final Year Project (FYP), which the title is "Cloud-based Hydroponic Plant Monitoring System",
I've been thinking on "How do I store my data to my Cloud MySQL Database without using any HTTP server such as PHP script?"
I tried search Google a bit, some articles wouldn't recommend to directly execute execute SQL as if more complex SQL may need
more hardware resources (CPU, RAM, Flash Memory).

Until I found some libraries online and tried some of them. Some got issues such as stack buffer overflow, memory leak and crash.
I found a library, which is MySQL_MariaDB_Generic library by Dr. Charles Bell and Khoi Hoang, which great to be used. So I modified and optimized it to suit my needs, and also add some new features to it.

![Overview Image](https://i.postimg.cc/JMYtqf4m/Cloud-Hydro-Plant.jpg)

### Features

##### Technically, you can execute any SQL query, but here are some features that might interest you

1. Variable INSERT data
  - Take my FYP for the example. I have various of data of sensors, which obviously they are not constant values, so I need to implement variables of int values to save into my database. So do you.

2. SELECT query
  - I mean, your ESP32 doesn't just INSERT (write) data to the database, your ESP32 also can read/fetch the data from the database. Here's an example, that print out the result in serial monitor (on left).

  <p align="center">
    <img src="https://i.postimg.cc/qqkPr1dS/SELECTquery-ESP32-My-SQL.png">
</p>

3. AES-256-ECB Encryption
  - Sometimes, we may need to encrypt our sensitive data before storing to the database. In real-world scenario for this library, a project that comes to my mind is "Cloud-based ESP32 Morse Code Device", which need to store sensitive messages interpreted from morse code to the database. So we can use this library to perform encryption then store the encrypted messages (ciphertexts) to the database. Here's an example of basic one, which just a constant string of plaintext. The second image is to decipher/decrypt the ciphertext from the ESP32.

  <p align="center">
    <img src="https://i.postimg.cc/V67RSgC3/ESP32-My-SQL-AES.png">
</p>

<p align="center">
    <img src="https://i.postimg.cc/sxvJ5WP1/ESP32-My-SQL-AES-decrypted.png">
</p>

4. SHA-256 Hash
  - People might confuse hash with encryption. The most basic difference between them is, encryption is two-way, you can encrypt and decrypt the message with corresponding key, while hash is one-way, irreversible, you can hash it but you cannot get the original input/message after you hash it. Most basic usage of hash to verify originality/authenticity of data, because even a bit (0 and 1) is modified, the entire output of hash will be very different, making us easier to check if the data is not original/authentic. In real-world scenario of this library, probably a "sign-up and login web page". We know that ESP32 can act as a web server, thus it is possible to create a web page for signing up and logging in, which need to use hash function to store and verify the passwords while maintaining the secure implementation of passwords storing in database. Here's an example of a constant string being hashed and stored in database.

  <p align="center">
    <img src="https://i.postimg.cc/W3KZLHrb/ESP32-My-SQL-SHA256.png">
</p>

## Installation

### Using Arduino Library Manager

Search my library "ESP32_MySQL" or my name "Syafiqlim" on library manager of your Arduino IDE. Install the latest release.

[![arduino-library-badge](https://www.ardu-badge.com/badge/ESP32_MySQL.svg?)](https://www.ardu-badge.com/ESP32_MySQL)

### Manual Installation

1. Download the latest release from the [releases page](https://github.com/Syafiqlim/ESP32_MySQL/releases).
2. Open your Arduino IDE.
3. Sketch -> Include Library -> Add .ZIP Library...
4. Select .zip file you downloaded.

## Examples

Here are the basic examples of coding you can use. Feel free to experiment, combine, mix and modify with your logical creativity!

1. [Basic INSERT](examples/Basic_Insert_ESP32MySQL)

2. [Variable INSERT](examples/Variable_Insert_ESP32MySQL)

3. [SELECT query](examples/SELECTquery_ESP32MySQL)

4. [AES-256-EBC Encryption](examples/AES256ECB_Encryption_ESP32MySQL)

5. [SHA-256 Hash](examples/SHA256_Hash_ESP32MySQL)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Acknowledgement

This library is modified and optimized (for ESP32) version of [MySQL_MariaDB_Generic](https://github.com/khoih-prog/MySQL_MariaDB_Generic) by [Khoi Hoang](https://github.com/khoih-prog) and [MySQL_Connector_Arduino](https://github.com/ChuckBell/MySQL_Connector_Arduino) by [Dr. Charles Bell](https://github.com/ChuckBell) 