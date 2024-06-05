/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Query_Impl.h
  by Syafiqlim @ syafiqlimx
*****************************/

#pragma once

#ifndef ESP32_MySQL_Query_IMPL_H
#define ESP32_MySQL_Query_IMPL_H

#define COMMAND_HEADER_LEN      5

/*
  Constructor

  Requires an instance of the ESP32_MySQL_Connection class to communicate with a
  MySQL server.

  connection[in]  Connection to a MySQL server - must be connected.
*/
ESP32_MySQL_Query::ESP32_MySQL_Query(ESP32_MySQL_Connection *connection) 
{
  conn = connection;
  
#ifdef WITH_SELECT
  columns.num_fields = 0;
  
  for (int f = 0; f < MAX_FIELDS; f++) 
  {
    columns.fields[f] = NULL;
    row.values[f]     = NULL;
  }
  
  columns_read    = false;
  rows_affected   = -1;
  last_insert_id  = -1;
#endif
}

/*
  Destructor
*/
ESP32_MySQL_Query::~ESP32_MySQL_Query() 
{
#ifdef WITH_SELECT
  close();
#endif
}

/*
  execute - Execute a SQL statement

  This method executes the query specified as a character array. It copies
  the query to the local buffer then calls the execute_query() method to
  execute the query.

  If a result set is available after the query executes, the field
  packets and rows can be read separately using the get_field() and
  get_row() methods.

  query[in]       SQL statement (using normal memory access)
  progmem[in]     True if string is in program memory

  Returns bool - True = a result set is available for reading
*/

// TODO: Pass buffer pointer instead of using global buffer
bool ESP32_MySQL_Query::execute(const char *query, bool progmem)
{
  int query_len;   // length of query

  if (!conn->connected()) 
  {
    ESP32_MYSQL_LOGERROR(NOT_CONNECTED);
    
    return false;
  }

  if (progmem) 
  {
    query_len = (int) strlen_P(query);
  } 
  else 
  {
    query_len = (int) strlen(query);
  }
  
  if ( conn->largest_buffer_size < query_len + COMMAND_HEADER_LEN )
  {
    if ( conn->largest_buffer_size == 0 )
    {
      // Check if we need to allocate buffer the first time. Don't need, but to be safe
      conn->largest_buffer_size = query_len + COMMAND_HEADER_LEN;
      ESP32_MYSQL_LOGWARN1("ESP32_MySQL_Query::execute: First time allocate buffer, size = ", conn->largest_buffer_size);
      
      conn->buffer = (byte *) malloc(conn->largest_buffer_size);
    }
    else
    {
      // Check if we need to reallocate buffer
      conn->largest_buffer_size = query_len + COMMAND_HEADER_LEN;
      ESP32_MYSQL_LOGWARN1("ESP32_MySQL_Query::execute: Reallocate buffer, size = ", conn->largest_buffer_size);
      
      conn->buffer = (byte *) realloc(conn->buffer, conn->largest_buffer_size);    
    }
  }
  else
  {
    ESP32_MYSQL_LOGDEBUG3("ESP32_MySQL_Query::execute: Reuse allocated buffer, conn->largest_buffer_size = ", conn->largest_buffer_size, " > ", query_len + COMMAND_HEADER_LEN);
  }
   
  if (conn->buffer == NULL)
  {
    ESP32_MYSQL_LOGERROR("ESP32_MySQL_Query::execute: NULL buffer");
   
    return false;
  }
  else
  {
    memset(conn->buffer, 0, conn->largest_buffer_size);
  }
  //////
    
  // Write query to packet
  if (progmem) 
  {
    for (int c = 0; c < query_len; c++)
      conn->buffer[c + COMMAND_HEADER_LEN] = pgm_read_byte_near(query + c);
  } 
  else 
  {
    memcpy(&conn->buffer[COMMAND_HEADER_LEN], query, query_len);
  }

  ESP32_MYSQL_LOGDEBUG1("ESP32_MySQL_Query::execute: query = ", (char *) &conn->buffer[COMMAND_HEADER_LEN] );
  
  // Send the query
  return execute_query(query_len);
}


/*
  execute_query - execute a query

  This method sends the query string to the server and waits for a
  response. If the result is a result set, it returns true, if it is
  an error, it processes the error packet and prints the error via
  Serial.print(). If it is an Ok packet, it parses the packet and
  returns false.

  query_len[in]   Number of bytes in the query string

  Returns bool - true = result set available,
                    false = no result set returned.
*/
// TODO: Pass buffer pointer instead of using global buffer
bool ESP32_MySQL_Query::execute_query(const int& query_len)
{
  if (!conn->buffer)
  {
    ESP32_MYSQL_LOGERROR("ESP32_MySQL_Query::execute_query: NULL buffer");
    return false;
  }

  // Reset the rows affected and last insert id before query.
  rows_affected  = -1;
  last_insert_id = -1;

  conn->store_int(&conn->buffer[0], query_len + 1, 3);
  conn->buffer[3] = byte(0x00);
  conn->buffer[4] = byte(0x03);  // command packet

  // Send the query
  ESP32_MYSQL_LOGDEBUG1("ESP32_MySQL_Query::execute_query: query = ", (char *) &conn->buffer[COMMAND_HEADER_LEN] );
  
  conn->client->write((uint8_t*)conn->buffer, query_len + COMMAND_HEADER_LEN);
  conn->client->flush();

  // Read a response packet and check it for Ok or Error.
  if ( !conn->read_packet() || ( conn->packet_len <= 0 ) || ( conn->packet_len > MAX_TRANSMISSION_UNIT ) )
    return false;
  //////
  
  int res = conn->get_packet_type();
  
  if (res == ESP32_MYSQL_ERROR_PACKET) 
  {
    conn->parse_error_packet();
    return false;
  } 
  else if (res == ESP32_MYSQL_OK_PACKET || res == ESP32_MYSQL_EOF_PACKET) 
  {
    // Read the rows affected and last insert id.
    int loc1 = conn->buffer[5];  // Location of rows affected
    int loc2 = 5;
    
    if (loc1 < 252) 
    {
      loc2++;
    } 
    else if (loc1 == 252) 
    {
      loc2 += 2;
    } 
    else if (loc1 == 253) 
    {
      loc2 += 3;
    } 
    else 
    {
      loc2 += 8;
    }
    
    rows_affected = conn->read_lcb_int(5);
    
    if (rows_affected > 0) 
    {
      last_insert_id = conn->read_lcb_int(loc2);
    }
    
    return true;
  }

  // Not an Ok packet, so we now have the result set to process.
#ifdef WITH_SELECT
  columns_read = false;
#endif
  return true;
}

#ifdef WITH_SELECT
/*
  Close

  Takes care of removing allocated memory.
*/
void ESP32_MySQL_Query::close() 
{
  free_columns_buffer();
  free_row_buffer();
}


/*
  get_columns - Get a list of the columns (fields)

  This method returns an instance of the column_names structure
  that contains an array of fields.

  Note: you should call free_columns_buffer() after consuming
        the field data to free memory.
*/
column_names *ESP32_MySQL_Query::get_columns() 
{
  free_columns_buffer();
  free_row_buffer();
  num_cols = 0;
  
  if (get_fields()) 
  {
    columns_read = true;
    return &columns;
  }

  return NULL;
}


/*
  get_next_row - Iterator for reading rows from a result set

  This method returns an instance of a structure (row_values)
  that contains an array of strings representing the row
  values returned from the server.

  The caller can use the values however needed - by first
  converting them to a specific type or as a string.
*/
row_values *ESP32_MySQL_Query::get_next_row() 
{
  int res = 0;

  free_row_buffer();

  // Read the rows
  ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_next_row: get_row_values");
  
  res = get_row_values();
  
  if (res != ESP32_MYSQL_EOF_PACKET) 
  {
    return &row;
  }
  
  return NULL;
}

/*
  show_results - Show a result set from the server via Serial.print

  This method reads a result from the server and displays it via the
  via the Serial.print methods. It can be used in cases where
  you may want to issue a SELECT or SHOW and see the results on your
  computer from the Arduino.

  It is also a good example of how to read a result set from the
  because it uses the public methods designed to return result
  sets from the server.
*/
void ESP32_MySQL_Query::show_results() 
{
  column_names *cols;
  int rows = 0;

  // Get the columns
  cols = get_columns();
  
  if (cols == NULL) 
  {
    return;
  }

  for (int f = 0; f < columns.num_fields; f++) 
  {
    ESP32_MYSQL_LOGERROR0(columns.fields[f]->name);
    
    if (f < columns.num_fields - 1)
      ESP32_MYSQL_LOGERROR0(',');
  }
  
  ESP32_MYSQL_LOGERROR0LN("");

  // Read the rows
  while (get_next_row()) 
  {
    rows++;
    
    for (int f = 0; f < columns.num_fields; f++) 
    {
      ESP32_MYSQL_LOGERROR0(row.values[f]);
      
      if (f < columns.num_fields - 1)
        ESP32_MYSQL_LOGERROR0(',');
    }
    
    free_row_buffer();
    ESP32_MYSQL_LOGERROR0LN("");
  }

  // Report how many rows were read
  ESP32_MYSQL_LOGERROR0(rows);
  ESP32_MYSQL_LOGERROR0LN(ROWS);
  
  free_columns_buffer();

  // Free any post-query messages in queue for stored procedures
  clear_ok_packet();
}


/*
  clear_ok_packet - clear last Ok packet (if present)

  This method reads the header and status to see if this is an Ok packet.
  If it is, it reads the packet and discards it. This is useful for
  processing result sets from stored procedures.

  Returns False if the packet was not an Ok packet.
*/
bool ESP32_MySQL_Query::clear_ok_packet() 
{
  int num = 0;

  do 
  {
    num = conn->client->available();
    
    if (num > 0) 
    {
      if ( !conn->read_packet() || ( conn->packet_len <= 0 ) || ( conn->packet_len > MAX_TRANSMISSION_UNIT ) )
        return false;
      //////
      
      if (conn->get_packet_type() != ESP32_MYSQL_OK_PACKET) 
      {
        conn->parse_error_packet();
        return false;
      }
    }
  } while (num > 0);
  
  rows_affected = -1;
  last_insert_id = -1;
  
  return true;
}


/*
  free_columns_buffer - Free memory allocated for column names

  This method frees the memory allocated during the get_columns()
  method.

  NOTICE: Failing to call this method after calling get_columns()
          and consuming the column names, types, etc. will result
          in a memory leak. The size of the leak will depend on
          the size of the combined column names (bytes).
*/
void ESP32_MySQL_Query::free_columns_buffer() 
{
  // clear the columns
  for (int f = 0; f < MAX_FIELDS; f++) 
  {
    if (columns.fields[f] != NULL) 
    {
      free(columns.fields[f]->db);
      free(columns.fields[f]->table);
      free(columns.fields[f]->name);
      free(columns.fields[f]);
    }
    
    columns.fields[f] = NULL;
  }
  
  num_cols = 0;
  columns_read = false;
}


/*
  free_row_buffer - Free memory allocated for row values

  This method frees the memory allocated during the get_next_row()
  method.

  NOTICE: You must call this method at least once after you
          have consumed the values you wish to process. Failing
          to do will result in a memory leak equal to the sum
          of the length of values and one byte for each max cols.
*/
void ESP32_MySQL_Query::free_row_buffer() 
{
  // clear the row
  for (int f = 0; f < MAX_FIELDS; f++) 
  {
    if (row.values[f] != NULL) 
    {
      free(row.values[f]);
    }
    
    row.values[f] = NULL;
  }
}


/*
  read_string - Retrieve a string from the buffer

  This reads a string from the buffer. It reads the length of the string
  as the first byte.

  offset[in]      offset from start of buffer

  Returns string - String from the buffer
*/
char *ESP32_MySQL_Query::read_string(int *offset) 
{
  char *str;
  
  ESP32_MYSQL_LOGLEVEL5("ESP32_MySQL_Query::read_string: step 1");
  
  int len_bytes = conn->get_lcb_len(conn->buffer[*offset]);
  int len = conn->read_int(*offset, len_bytes);
  
  ESP32_MYSQL_LOGINFO1("ESP32_MySQL_Query::read_string: offset = ", *offset);
  ESP32_MYSQL_LOGINFO3("ESP32_MySQL_Query::read_string: len = ", len, "len_bytes =", len_bytes);
  
  if (len == 251) 
  {
    // This is a null field.
    str = (char *) malloc(5);
    strcpy(str, "NULL");
    str[4] = 0x00;
    *offset += len_bytes;
  }

  if ( (len < 251) && (len > 0) )
  {
    str = (char *) malloc(len + 1);
    
    ESP32_MYSQL_LOGINFO3("ESP32_MySQL_Query::read_string: len = ", len, "conn->buffer size =", conn->largest_buffer_size);
    
    strncpy(str, (char *)&conn->buffer[*offset + len_bytes], len);
    str[len] = 0x00;
    *offset += len_bytes + len;
    
    ESP32_MYSQL_LOGDEBUG1("ESP32_MySQL_Query::read_string: str = ", str);
    
    return str;
  }
  
  ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::read_string: return NULL");
  
  return NULL;
  ////// 
}


/*
  get_field - Read a field from the server

  This method reads a field packet from the server. Field packets are
  defined as:

  Bytes                      Name
  -----                      ----
  n (Length Coded String)    catalog
  n (Length Coded String)    db
  n (Length Coded String)    table
  n (Length Coded String)    org_table
  n (Length Coded String)    name
  n (Length Coded String)    org_name
  1                          (filler)
  2                          charsetnr
  4                          length
  1                          type
  2                          flags
  1                          decimals
  2                          (filler), always 0x00
  n (Length Coded Binary)    default

  Note: the sum of all db, column, and field names must be < 255 in length
*/
int ESP32_MySQL_Query::get_field(field_struct *fs) 
{
  int len_bytes;
  int len;
  int offset;

  if (conn->buffer == NULL) 
  {
    ESP32_MYSQL_LOGERROR("ESP32_MySQL_Query::get_field: NULL buffer");
    return ESP32_MYSQL_ERROR_PACKET;
  }
  //////
  
  // Read field packets until EOF
  ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_field: read_packet");

  if ( !conn->read_packet() || ( conn->packet_len <= 0 ) || ( conn->packet_len > MAX_TRANSMISSION_UNIT ) )
    return ESP32_MYSQL_ERROR_PACKET;
  //////

  if (conn->buffer && conn->buffer[4] != ESP32_MYSQL_EOF_PACKET)
  {
    // calculate location of db
    len_bytes = conn->get_lcb_len(4);
    len = conn->read_int(4, len_bytes);
    offset = 4 + len_bytes + len;
    
    ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_field: read_string to fs->db");
    fs->db = read_string(&offset);
    ESP32_MYSQL_LOGDEBUG1("ESP32_MySQL_Query::get_field: fs->db = ", fs->db);
    
    // get table
    ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_field: read_string to fs->table");
    fs->table = read_string(&offset);
    ESP32_MYSQL_LOGDEBUG1("ESP32_MySQL_Query::get_field: fs->table = ", fs->table);
    
    // calculate location of name
    len_bytes = conn->get_lcb_len(offset);
    len = conn->read_int(offset, len_bytes);
    offset += len_bytes + len;
    
    // get name
    ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_field: read_string to fs->name");
    fs->name = read_string(&offset);
    ESP32_MYSQL_LOGDEBUG1("ESP32_MySQL_Query::get_field: fs->name = ", fs->name);
    
    //return 0;
    return ESP32_MYSQL_OK_PACKET;
  }
  else if (conn->buffer && conn->buffer[4] == ESP32_MYSQL_EOF_PACKET)
    return ESP32_MYSQL_EOF_PACKET;
  else
    return ESP32_MYSQL_ERROR_PACKET;
  //////
}


/*
  get_row - Read a row from the server and store it in the buffer

  This reads a single row and stores it in the buffer. If there are
  no more rows, it returns ESP32_MYSQL_EOF_PACKET. A row packet is defined as
  follows.

  Bytes                   Name
  -----                   ----
  n (Length Coded String) (column value)
  ...

  Note: each column is store as a length coded string concatenated
        as a single stream

  Returns integer - ESP32_MYSQL_EOF_PACKET if no more rows, 0 if more rows available
*/
int ESP32_MySQL_Query::get_row() 
{
  // Read row packets
  ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_row: read_packet");

  if ( !conn->read_packet() || ( conn->packet_len <= 0 ) || ( conn->packet_len > MAX_TRANSMISSION_UNIT ) )
    return ESP32_MYSQL_EOF_PACKET;    //ESP32_MYSQL_ERROR_PACKET;
  //////

  if (conn->buffer && conn->buffer[4] != ESP32_MYSQL_EOF_PACKET)
    return ESP32_MYSQL_OK_PACKET;
    
  return ESP32_MYSQL_EOF_PACKET;
}


/*
  get_fields - reads the fields from the read buffer

  This method is used to read the field names, types, etc.
  from the read buffer and store them in the columns structure
  in the class.
*/
bool ESP32_MySQL_Query::get_fields()
{
  int num_fields = 0;
  int res = 0;

  if (conn->buffer == NULL) 
  {
    ESP32_MYSQL_LOGERROR("ESP32_MySQL_Query::get_fields: NULL buffer");
    return false;
  }
  
  num_fields = conn->buffer[4]; // From result header packet
  columns.num_fields = num_fields;
  num_cols = num_fields; // Save this for later use
  
  for (int f = 0; f < num_fields; f++) 
  {
    field_struct *field = (field_struct *) malloc(sizeof(field_struct));
    res = get_field(field);
    
    //if (res == ESP32_MYSQL_EOF_PACKET) 
    if ( (res == ESP32_MYSQL_EOF_PACKET) || (res == ESP32_MYSQL_ERROR_PACKET) )
    {
      ESP32_MYSQL_LOGERROR(BAD_MOJO);
      
      return false;
    }
    
    columns.fields[f] = field;
  }

  // EOF packet
  ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_fields: read_packet");
  
  if ( !conn->read_packet() || ( conn->packet_len <= 0 ) || ( conn->packet_len > MAX_TRANSMISSION_UNIT ) )
    return false;
  //////
  
  return true;
}

/*
  get_row_values - reads the row values from the read buffer

  This method is used to read the row column values
  from the read buffer and store them in the row structure
  in the class.
*/
int ESP32_MySQL_Query::get_row_values() 
{
  int res = 0;
  int offset = 0;

  // It is an error to try to read rows before columns
  // are read.
  if (!columns_read) 
  {
    ESP32_MYSQL_LOGERROR(READ_COLS);
    
    return ESP32_MYSQL_EOF_PACKET;
  }
  
  // Drop any row data already read
  free_row_buffer();

  // Read a row
  ESP32_MYSQL_LOGDEBUG("ESP32_MySQL_Query::get_row_values: get_row");
  
  res = get_row();
  
  if ( (res != ESP32_MYSQL_EOF_PACKET) && (res != ESP32_MYSQL_ERROR_PACKET) )
  {
    offset = 4;
    
    for (int f = 0; f < num_cols; f++) 
    {
      row.values[f] = read_string(&offset);
    }
  }
  
  return res;
}

#endif    // WITH_SELECT

#endif    // ESP32_MySQL_Query_IMPL_H
