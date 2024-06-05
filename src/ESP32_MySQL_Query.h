/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Query.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once

#ifndef ESP32_MySQL_Query_H
#define ESP32_MySQL_Query_H

#include "ESP32_MySQL_Debug.h"

#include <ESP32_MySQL_Connection.h>

// Comment this if you don't need SELECT queries to reduce memory footprint of the library.
#define WITH_SELECT          

#define MAX_FIELDS    0x20   // Maximum number of fields. Reduce to save memory. Default=32

#ifdef WITH_SELECT

// Structure for retrieving a field (minimal implementation).
typedef struct 
{
  char *db;
  char *table;
  char *name;
} field_struct;

// Structure for storing result set metadata.
typedef struct 
{
  int num_fields;     // actual number of fields
  field_struct *fields[MAX_FIELDS];
} column_names;

// Structure for storing row data.
typedef struct 
{
  char *values[MAX_FIELDS];
} row_values;

#endif  // WITH_SELECT

class ESP32_MySQL_Query 
{
  public:
    ESP32_MySQL_Query(ESP32_MySQL_Connection *connection);
    ~ESP32_MySQL_Query();
    bool execute(const char *query, bool progmem = false);

  private:
    bool execute_query(const int& query_len);
    
#ifdef WITH_SELECT

  public:
    void close();
    column_names  *get_columns();
    row_values    *get_next_row();
    void          show_results();
    
    int get_rows_affected() 
    {
      return rows_affected;
    }
    
    int get_last_insert_id() 
    {
      return last_insert_id;
    }

  private:
    void  free_columns_buffer();
    void  free_row_buffer();
    bool  clear_ok_packet();

    char  *read_string(int *offset);
    int   get_field(field_struct *fs);
    int   get_row();
    bool  get_fields();
    int   get_row_values();
    column_names *query_result();

    bool          columns_read;
    int           num_cols;
    
    column_names  columns;
    row_values    row;
    
    int           rows_affected;
    int           last_insert_id;
    
#endif

    ESP32_MySQL_Connection *conn;
};


#endif    // ESP32_MySQL_Query_H
