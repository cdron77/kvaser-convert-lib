/*
**             Copyright 2017 by Kvaser AB, Molndal, Sweden
**                         http://www.kvaser.com
**
** This software is dual licensed under the following two licenses:
** BSD-new and GPLv2. You may use either one. See the included
** COPYING file for details.
**
** License: BSD-new
** ==============================================================================
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the <organization> nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
** BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
** IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
**
** License: GPLv2
** ==============================================================================
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
**
**
** IMPORTANT NOTICE:
** ==============================================================================
** This source code is made available for free, as an open license, by Kvaser AB,
** for use with its applications. Kvaser AB does not accept any liability
** whatsoever for any third party patent or other immaterial property rights
** violations that may result from any usage of this source code, regardless of
** the combination of source code and various applications that it can be used
** in, or with.
**
** -----------------------------------------------------------------------------
*/

// CANdbLdf.h: Read & write Lin Ldf (.ldf) files into/from  CANdb classes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>


#include "CANdb.h"
#include "CANdbLdf.h"

// ****************************************************************************

static CANdbFileFormat candb_reader ("LDF",
                                     "LIN Description File",
                                     "Description of LIN network traffic in LDF format",
                                     "ldf",
                                     CANDB_FILE_FORMAT_FLAG_READ,
                                     CANdbLDF::build);

// ****************************************************************************

#ifndef MAX_PATH
#  define MAX_PATH      1024
#endif

#define DUMMY_RECEIVER_NODE     "Vector__XXX"

#define DUMMY_MSG_FOR_INDEPENDENT_SIGNALS       "VECTOR__INDEPENDENT_SIG_MSG"
#define DUMMY_MSGID_FOR_INDEPENDENT_SIGNALS     0xc0000000

// Tokens:
#define T_EOF               0
#define T_UNKNOWN           998
#define T_NEWLINE           999
#define T_NODES             1000
#define T_FRAMES            1001
#define T_SIGNALS           1003
#define T_SIG_ENC_TYPES     1004
#define T_SIG_REPS          1005
#define T_SCHEDULE_TABS     1006
#define T_DIAG_FRAMES       1007
#define T_EVENT_TRIG_FRAMES 1008
#define T_CHANNEL_NAME      1009
#define T_IDENT             1021
#define T_INT_CONST         1022
#define T_DOUBLE_CONST      1023
#define T_STRING_CONST      1024

#define MAX_ERROR_STRING_LEN    256
// ****************************************************************************

struct s_keyword {
         const char *name;
         int  token;
       };

struct s_hash_entry {
         struct s_keyword *keyword;
         unsigned int     hash_val;
       };


// ****************************************************************************


static struct s_hash_entry *keyword_hash_table = NULL;
static unsigned int keyword_hash_table_size = 0;

static struct s_keyword keywords[] = {
         { "Nodes",                 T_NODES             },
         { "Signals",               T_SIGNALS           },
         { "Frames",                T_FRAMES            },
         { "Signal_encoding_types", T_SIG_ENC_TYPES     },
         { "Signal_representation", T_SIG_REPS          },
         { "Schedule_tables",       T_SCHEDULE_TABS     },
         { "Diagnostic_frames",     T_DIAG_FRAMES       },
         { "Event_triggered_frames",T_EVENT_TRIG_FRAMES },
         { "Channel_name",          T_CHANNEL_NAME      },
         { NULL,                    0                   },
       };

static unsigned long hash_value (const char *s)
{
  unsigned long h = 0;
  if (!s) return h;
  while (*s) {
    h = h * 2 + *((unsigned char*) s);
    h = (h & 0x00ffffff) + ((h >> 24) & 0x000000ff);
    ++s;
  }
  return h;
} // hash_value


static void build_keyword_hash_table (void)
{
  unsigned int i = 0;

  if (!keyword_hash_table) {
    unsigned int keyword_count = 0;
    while (keywords [keyword_count].name) ++keyword_count;

    keyword_hash_table_size = keyword_count * 2 + 1;
    keyword_hash_table = new struct s_hash_entry [keyword_hash_table_size];
  }

  if (keyword_hash_table) {
    for (i = 0; i < keyword_hash_table_size; ++i) {
      keyword_hash_table [i].keyword = NULL;
      keyword_hash_table [i].hash_val = 0;
    }

    i = 0;
    while (keywords[i].name) {
      unsigned long h = hash_value (keywords [i].name);
      unsigned int idx = h % keyword_hash_table_size;
      while (keyword_hash_table [idx].keyword) idx = (idx + 1) % keyword_hash_table_size;
      keyword_hash_table [idx].keyword  = &keywords [i];
      keyword_hash_table [idx].hash_val = h;
      ++i;
    }

    /*
    for (i = 0; i < keyword_hash_table_size; ++i) {
      printf ("%3d: ", i);
      if (keyword_hash_table [i].keyword) {
        printf ("%12lu -> %s",
                keyword_hash_table [i].hash_val,
                keyword_hash_table [i].keyword->name);
      }
      else printf ("    ****");
      printf ("\n");
    }
    */
  }
} // build_keyword_hash_table


static int find_token (char *str)
{
  if (keyword_hash_table) {
    unsigned long h = hash_value (str);
    unsigned int idx = h % keyword_hash_table_size;
    while (keyword_hash_table [idx].keyword &&
           ((keyword_hash_table [idx].hash_val != h) ||
            (strcmp (keyword_hash_table [idx].keyword->name, str) != 0))) {
      idx = (idx + 1) % keyword_hash_table_size;
    }
    if (!keyword_hash_table [idx].keyword) {
      /* printf ("KKK not found: %s\n", str); */
      return 0;
    }
      /* printf ("KKK found: %s / %s \n", str, hash_table [idx].keyword->string); */
    return keyword_hash_table [idx].keyword->token;
  }
  return -1;
} /* find_token */


// ****************************************************************************


CANdbLDF::CANdbLDF (const char *filename)
  : CANdbFileIo (filename)
{
  file = NULL;
  current_db = NULL;
  file_eof = false;
  strcpy (string_buf, "");
  file = fopen(get_filename(), "r");
} // CANdbLDF::CANdbLDF


CANdbLDF::~CANdbLDF ()
{
  if (file != NULL) {
    fclose(file);
    file = NULL;
  }

  keyword_hash_table_size = 0;
  if (keyword_hash_table != NULL)
  {
    delete keyword_hash_table;
    keyword_hash_table = NULL;
  }

  for (int i = 0; i < (int)v_Ldf_Encoding_Types.size(); i++) {
    delete v_Ldf_Encoding_Types[i];
  }
  v_Ldf_Encoding_Types.clear();

  for (int i = 0; i < (int)v_Ldf_Signals.size(); i++) {
    delete v_Ldf_Signals[i];
  }
  v_Ldf_Signals.clear();
} // CANdbLDF::~CANdbLDF


// ****************************************************************************


char CANdbLDF::skip_space (void)
{
  signed char c;
  do {
    c = fgetc (file);
    if (c == '\n') lineno++;
    if (c == EOF) break;
  } while (c <= ' ');
  return c;
} // CANdbLDF::skip_space


int CANdbLDF::read_string (char c, LinToken& token)
{
  int i = 0;
  int abort = 0;

  if (c != '"') return -1;
  do {
    c = fgetc (file);
    if (i < CANDB_LDF_MAX_STRING) string_buf[i++] = c;

    if (c == '\n') lineno ++;

    if (c == '"') {
      c = fgetc (file);
      if (c != '"') {
        ungetc (c, file);
        abort = 1;
      }
    }
    else if (c == '\\') {
      c = fgetc (file);
      if (c == 'n') string_buf [i-1] = '\n';
      else if (c == 't') string_buf [i-1] = '\t';
      else if (c == 'r') string_buf [i-1] = '\r';
      else if (c == '"') string_buf [i-1] = c;
      else ungetc (c, file);
    }
  }
  while (abort == 0);
  string_buf [i-1] = '\0';

  token.string_const = string_buf;

  return T_STRING_CONST;
} // CANdbLDF::read_string


int CANdbLDF::read_value (char c, LinToken& token)
{
  int i = 0;

  if ((c == '.') || (c == '-')) {
    string_buf[i++] = c;
    c = fgetc (file);
  }

  if (! isdigit (c)) return -1;

  string_buf[i++] = c;
  for (; isdigit (c = fgetc (file)); i ++) string_buf[i] = c;

  /* Long- bzw. Integer-Zahl in Dezimalform */
  if ((c != '.') && (c != 'E') && (c != 'e') && (c != 'x') && (c != 'X')) {
    ungetc (c, file);
    string_buf[i] = '\0';
    token.int_const = (int)_atoi64 (string_buf);
    return T_INT_CONST;
  }

  /* Hex-Zahl */
  else if ((c == 'x') || (c == 'X')) {
    long hex;
    if (string_buf [0] == '0') {
      if (i != 1) return -1;
    }
    else if ((string_buf [0] == '+') || (string_buf [0] == '-')) {
      if ((string_buf [1] != '0') || (i != 2)) return -1;
    }
    else return -1;
    i --;
    for (; isxdigit (c = fgetc (file)); i++) string_buf [i] = c;
    ungetc (c, file);
    string_buf [i] = '\0';
    sscanf ((const char *) string_buf, "%lx", &hex);
    token.int_const = hex;
    return T_INT_CONST;
  }

  /* Double- bzw. Float-Zahl */
  else {
    if (c == '.') {
      string_buf[i] = c;
      for (i++; isdigit (c = fgetc (file)); i++) string_buf[i] = c;
    }
    if ((c == 'E') || (c == 'e')) {
      string_buf[i++] = c;
      c = fgetc (file);
      if ((c == '-') || (c == '+')) {
        string_buf [i++] = c;
        c = fgetc (file);
      }
      if (isdigit (c)) {
        string_buf [i++] = c;
        for (; isdigit (c = fgetc (file)); i++) string_buf [i] = c;
      }
      else return -1;
    }
    ungetc (c, file);
    string_buf [i] = '\0';

    sscanf (string_buf, "%lf", &token.double_const);

    return T_DOUBLE_CONST;
  }

} // CANdbLDF::read_value


/* Reads a token from the infile. Blank space is skipped first. If during this, there is a new line,
* the token T_NEWLINE is returned.
* At EOF, a T_NEWLINE is returned; the next call results in T_EOF.
* Comments are just skipped.
*
*/
int CANdbLDF::lex (LinToken& token)
{
  signed char c;
  char c1;

start:
  /* Leerzeichen ueberlesen */
  c = skip_space ();
  if (c == EOF) {
    if (file_eof) return T_EOF;
    file_eof = true;
    return T_NEWLINE; // Next call will result in T_EOF
  }

  /*
  if (lineno != linenoSave) {
    ungetc (c, file);
    return T_NEWLINE;
  }
  */

  if (c == '/') {
    //int startline = lineno;
    c1 = fgetc (file);
    if ((c1 == '*') || (c1 = '/')) {
      for (;;) {
        c = fgetc (file);
      next_char:
        if (c == '\n')
          lineno ++;
        if((c == '\n') && (c1 = '/'))
          goto start;
        if (c == EOF) {
          error ("Error: Unexpected eof in comment");
          return -1;
        }
        if (c == '*') {
          c = fgetc (file);
          if (c == '/') goto start;
          goto next_char;
        }
      }
    }
    else ungetc (c1, file);
  }

  /* symbol or keyword */
  if (isalpha (c) || (c == '_')) {
    int tok = 0,
        i = 0;

    //static int tib = 0;
    // if ((tib++ % 2) == 0) ib = ident_buf1; else ib = ident_buf2;
    char *ib = string_buf;

    do {
      ib [i++] = c;
      c = fgetc (file);
    } while (isalpha (c) ||
             isdigit (c) ||
             (c == '_'));

    ungetc (c, file);
    ib[i] = '\0';

    tok = find_token (ib);
    if (tok > 0) return tok;

    token.ident = ib;
    return T_IDENT;
  }

  /* Zahlen (hex, int, double) */
  if ((c == '.') || (c == '-') || (c == '+')) {
    c1 = fgetc (file);
    /*
    if (c1 == '.') {
      c2 = fgetc (file);
      if (c1 == '.') return T_VARARG;
      ungetc (c2, file);
    }
    ungetc (c1, file);
    */
    ungetc (c1, file);
    if (!isdigit (c1)) return c;
  }

  if (isdigit (c) || (c == '.') || (c == '-') || (c == '+')) {
    return read_value (c, token);
  }

  /* string */
  else if (c == '"') {
    return read_string (c, token);
  }
  else {
    /*
    c1 = fgetc (file);
    if ((c == '&') && (c1 == '&')) return T_LOGICAL_AND;
    if ((c == '|') && (c1 == '|')) return T_LOGICAL_OR;
    if ((c == '+') && (c1 == '+')) return T_INC;
    if ((c == '-') && (c1 == '-')) return T_DEC;
    if (c1 == '=') {
      if (c == '>') return T_GE;
      if (c == '<') return T_LE;
      if (c == '!') return T_NE;
      if (c == '=') return T_EQ;
    }
    ungetc (c1, file);
    */
    return c;
  }

  return T_UNKNOWN;
} // CANdbLDF::lex




// ****************************************************************************

int CANdbLDF::read_ldf_header()
{
  LinToken token;
  int t;
  t = lex(token);
  if (t != T_IDENT || _stricmp("LIN_description_file", token.ident) != 0) {
    error("File must start with LIN_description_file");
    return -1;
  }

  t = lex(token);
  if (t != ';') {
    error("';' expected after LIN_description_file");
    return -1;
  }
  
  t = lex(token);
  if (t != T_IDENT || _stricmp("LIN_protocol_version", token.ident) != 0) {
    error("LIN_protocol_version is missing");
    return -1;
  }

  t = lex(token);
  if (t != '=') {
    error("'=' expected after LIN_protocol_version");
    return -1;
  }

  t = lex(token);
  if (t != T_STRING_CONST) {
    error("LIN_protocol_version should specify version as a string");
    return -1;
  }

  CANdbAttributeDefinition *attr_def = new CANdbAttributeDefinition();
  attr_def->set_owner(CANDB_ATTR_OWNER_DB);
  attr_def->set_name("LIN_protocol_version");
  attr_def->set_type(CANDB_ATTR_TYPE_STRING);
  current_db->insert_attribute_definition(attr_def);
  CANdbAttribute *attr = new CANdbAttribute(attr_def);
  attr->set_string_value(token.string_const);
  CANdbAttributeList *attr_list = current_db->get_attributes();
  attr_list->insert(attr);

  t = lex(token);
  if (t != ';') {
    error("';' expected after LIN_protocol_version");
    return -1;
  }
  
  t = lex(token);
  if (t != T_IDENT || _stricmp("LIN_language_version", token.ident) != 0) {
    error("LIN_language_version is missing");
    return -1;
  }

  t = lex(token);
  if (t != '=') {
    error("'=' expected after LIN_language_version");
    return -1;
  }

  t = lex(token);
  if (t != T_STRING_CONST) {
    error("LIN_language_version should specify version as a string");
    return -1;
  }

  attr_def = new CANdbAttributeDefinition();
  attr_def->set_owner(CANDB_ATTR_OWNER_DB);
  attr_def->set_name("LIN_language_version");
  attr_def->set_type(CANDB_ATTR_TYPE_STRING);
  current_db->insert_attribute_definition(attr_def);
  attr = new CANdbAttribute(attr_def);
  attr->set_string_value(token.string_const);
  attr_list->insert(attr);

  t = lex(token);
  if (t != ';') {
    error("';' expected after LIN_language_version");
    return -1;
  }
  
  t = lex(token);
  if (t != T_IDENT || _stricmp("LIN_speed", token.ident) != 0) {
    error("LIN_speed is missing");
    return -1;
  }

  t = lex(token);
  if (t != '=') {
    error("'=' expected after LIN_speed");
    return -1;
  }

  t = lex(token);
  if (t != T_DOUBLE_CONST) {
    error("LIN_speed should specify speed as a number");
    return -1;
  }

  attr_def = new CANdbAttributeDefinition();
  attr_def->set_owner(CANDB_ATTR_OWNER_DB);
  // LinSpeedDefinition is standard Vector attribute for LIN_speed
  attr_def->set_name("LinSpeedDefinition");
  attr_def->set_type(CANDB_ATTR_TYPE_FLOAT);
  attr_def->set_float_min(1);
  attr_def->set_float_max(20);
  attr_def->set_float_default(token.double_const);
  current_db->insert_attribute_definition(attr_def);
  attr = new CANdbAttribute(attr_def);
  attr->set_float_value(token.double_const);
  attr_list->insert(attr);

  t = lex(token);
  if (t != T_IDENT || _stricmp("kbps", token.ident) != 0) {
    error("'kbps' expected after LIN_speed");
    return -1;
  }

  t = lex(token);
  if (t != ';') {
    error("';' expected after LIN_speed");
    return -1;
  }

  return t;
} // CANdbLDF::read_ldf_header

int CANdbLDF::read_ldf_channel_name()
{
  LinToken token;
  int t;
  t = lex(token);
  if (t != '=') {
    error("'=' expected after Channel_name");
    return -1;
  }

  t = lex(token);
  if (t != T_STRING_CONST) {
    error("Channel_name should be specified as a string");
    return -1;
  }

  channel_name = token.string_const;

  t = lex(token);
  if (t != ';') {
    error("';' expected after Channel_name");
    return -1;
  }
  
  return t;
} // CANdbLDF::read_ldf_channel_name

int CANdbLDF::read_ldf_signals()
{
  for (int i = 0; i < (int)v_Ldf_Signals.size(); i++) {
    delete v_Ldf_Signals[i];
  }
  v_Ldf_Signals.clear();
  LinToken token;
  int t;
  t = lex (token);
  CANdbSignal *signal = NULL;

  if (t != '{') {
    error ("'{' expected");
    return -1;
  }

  t = lex (token);
  do {
    if(t == T_IDENT) {
      signal = new CANdbSignal;
      signal->set_type(CANDB_UNSIGNED);
      signal->set_unit("");
      signal->set_name(token.ident);
      v_Ldf_Signals.push_back(signal);
    } else {
      error("Signal name expected");
      return -1;
    }

    t = lex(token);
    if(t != ':') {
      error("':' expected after signal name.");
      return -1;
    }

    t = lex(token);
    if(t == T_INT_CONST) {
      signal -> set_length (token.int_const);
      signal -> set_min_val (0);
      int val = token.int_const;
      signal -> set_max_val ( (2 << (val - 1)) - 1);
    } else {
      error("Integer expected for signal size");
      return -1;
    }

    t = lex(token);
    if(t != ',') {
      error("',' expected after signal length.");
      return -1;
    }

    t = lex(token);
    if(t != T_INT_CONST) {
      error("Integer init_value expected.");
      return -1;
    }

    t = lex (token);
    if(t != ',') {
      error("',' expected after init_value.");
      return -1;
    }

    t = lex(token);
    if(t != T_IDENT) {
      error("Publishing node name expected.");
      return -1;
    }

    t = lex(token);
    if(t == ';') {
      t = lex (token);
      continue;
    }

    do {
      if(t != ',') {
        error("',' Expected between node names.");
        return -1;
      }

      t = lex (token);
      if(t == T_IDENT) {
        signal->add_receive_node(current_db->find_node_by_name(token.ident));
      } else {
        error("Node name expected");
        return -1;
      }
      t = lex(token);
    } while ((t != ';') && (t > 0));

    t = lex(token);
  } while ((t != '}') && (t > 0));

  return t;
} // CANdbLDF::read_ldf_signals

static CANdbNode * dbLinMasterNode;

int CANdbLDF::read_ldf_nodes()
{
  CANdbNode *node = NULL;
  LinToken token;
  int t;
  t = lex (token);

  if (t != '{') {
    error ("'{' expected");
    return -1;
  }

  t = lex(token);

  if((t != T_IDENT) || (strcmp("Master", token.ident) != 0)) {
    error("Master expected");
    return -1;
  }

  t = lex(token);
  if(t != ':') {
    error("':' expected after MASTER specification.");
    return -1;
  }

  t = lex (token);
  if(t == T_IDENT) {
    dbLinMasterNode = new CANdbNode;
    dbLinMasterNode->set_name(token.ident);
    current_db->insert_node (dbLinMasterNode);
  } else {
    error ("Master node specification expected.");
    return -1;
  }

  // Read to the end of the line, skip bittime and jitter for now.
  while ((t != ';') && (t > 0)) t = lex(token);

  t = lex (token);
  if((t != T_IDENT) || (strcmp("Slaves", token.ident) != 0)) {
    error("Slaves expected.");
    return -1;
  }

  t = lex (token);
  if(t != ':') {
    error("':' expected after Slaves");
    return -1;
  }

  t = lex (token);

  do {
    if (t == T_IDENT) {
      node = new CANdbNode;
      node->set_name (token.ident);
      current_db->insert_node (node);
    } else {
      error("Node name expected.");
      return -1;
    }
    t = lex (token);

    if (t == ';') break;

    if (t != ',') {
      error("Comma expected.");
      return -1;
    }

    t = lex (token);
  } while ((t != ';') && (t > 0));

  return t;

} // CANdbLDF::read_ldf_nodes

int CANdbLDF::insert_ldf_signals (CANdbMessage * message, bool bSetDlc)
{
  LinToken token;
  int t;
  CANdbSignal * current_sig = NULL;
  int i = 0;
  int iMaxBitNo;
  bool bSignalFound = false;
  char szError[MAX_ERROR_STRING_LEN];

  t = lex (token);
  do {
    if (t == T_IDENT) {
      for(i = 0; i < (int)v_Ldf_Signals.size(); i++) {
        bSignalFound = false;
        if(strcmp (v_Ldf_Signals[i] -> get_name (), token.ident) == 0) {
          current_sig = new CANdbSignal();
          *current_sig = *v_Ldf_Signals [i];
          message -> insert_signal (current_sig);
          bSignalFound = true;
          break;
        }
      }
      if(!bSignalFound) {
        sprintf(szError, "Reference to unknown signal \"%s\".", token.ident);
        error(szError);
        return -1;
      }
    } else {
      error("Signal name expected");
      return -1;
    }

    t = lex (token);

    if (t != ',') {
      error("Comma expected after Signal name.");
      return -1;
    }

    t = lex (token);
    if(t == T_INT_CONST && current_sig) {
      current_sig->set_start_bit (token.int_const);
    } else {
      error ("Integer signal offset expected.");
      return -1;
    }

    t = lex (token);
    if (t != ';') {
      error ("';' expected after signal offset.");
      return -1;
    }

    t =lex (token);

  } while ((t != '}') && (t > 0));

  iMaxBitNo = current_sig->get_start_bit() + current_sig->get_length();
  if(bSetDlc && (t > 0))
    message->set_dlc((iMaxBitNo / 8) + ((iMaxBitNo % 8) ? 1:0));

  return t;
} // CANdbLDF::insert_ldf_signals

int CANdbLDF::read_ldf_frames ()
{
  LinToken token;
  int t;
  t = lex (token);
  bool bDlcSet = false; //Dlc explicitly set.
  char szError[MAX_ERROR_STRING_LEN];

  CANdbMessage *message = NULL;

  if (t != '{') {
    error ("'{' expected");
    return -1;
  }

  t = lex (token);
  do {
    bDlcSet = false;
    message = NULL;

    if (t == T_IDENT) {
      message = new CANdbMessage;
      message->set_name (token.ident);
    } else {
      error("Frame name expected.");
      return -1;
    }

    t = lex (token);
    if (t != ':') {
      error("':' expected after frame name.");
      return -1;
    }

    t = lex (token);
    if (t == T_INT_CONST) {
      message->set_id (token.int_const);
    } else {
      error("Integer expected for the Id.");
      return -1;
    }

    t = lex (token);
    if (t != ',') {
      error("',' expected after the Id.");
      return -1;
    }

    t = lex (token);
    if(t == T_IDENT) {
      message -> set_send_node (current_db->find_node_by_name (token.ident));
    } else {
      error("Publishing node name expected.");
      return -1;
    }

    t = lex (token);

    if ((t != '{') && (t > 0)) {
      if (t != ',') {
        error ("Comma expected.");
        return -1;
      }

      t = lex (token);
      if(t == T_INT_CONST) {
        if((token.int_const > 8) || (token.int_const < 0)) {
          sprintf(szError, "Frame %s: Dlc should be a value from 0 to 8.", message->get_name());
          error(szError);
          return -1;
        }

        message -> set_dlc (token.int_const);
        bDlcSet = true;
      } else {
        error ("Dlc expected.");
        return -1;
      }
      t = lex (token);
    }

    if ( t != '{') {
      error ("'{' expected.");
      return -1;
    }

    if (!message) return -1;

    t = insert_ldf_signals (message, !bDlcSet);
    if (t < 0) return t;

    current_db->insert_message (message);

    t = lex (token);

  } while ((t != '}') && (t > 0));

  return t;
} // CANdbLDF::read_ldf_frames

CANdbMessage* CANdbLDF::add_master_req()
{
  CANdbMessage* masterReq = NULL;
  CANdbSignal* signal = NULL;

  CANdbSignal form_sig;
  form_sig.set_length (8);
  form_sig.set_offset(0);
  form_sig.set_factor(1);
  form_sig.set_min_val(0);
  form_sig.set_max_val(255);
  form_sig.set_type(CANDB_UNSIGNED);

  masterReq = new CANdbMessage;
  masterReq->set_name("MasterReq");
  masterReq->set_id( 60 );
  masterReq->set_dlc(8);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB0");
  signal->set_start_bit(0);
  masterReq->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB1");
  signal->set_start_bit(8);
  masterReq->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB2");
  signal->set_start_bit(16);
  masterReq->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB3");
  signal->set_start_bit(24);
  masterReq->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB4");
  signal->set_start_bit(32);
  masterReq->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB5");
  signal->set_start_bit(40);
  masterReq->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB6");
  signal->set_start_bit(48);
  masterReq->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("MasterReqB7");
  signal->set_start_bit(56);
  masterReq->insert_signal(signal);

  current_db->insert_message(masterReq);
  return masterReq;
}

CANdbMessage* CANdbLDF::add_slave_resp()
{
  CANdbMessage* slaveResp = NULL;
  CANdbSignal* signal = NULL;

  CANdbSignal form_sig;
  form_sig.set_length (8);
  form_sig.set_offset(0);
  form_sig.set_factor(1);
  form_sig.set_min_val(0);
  form_sig.set_max_val(255);
  form_sig.set_type(CANDB_UNSIGNED);

  slaveResp = new CANdbMessage;
  slaveResp->set_name("SlaveResp");
  slaveResp->set_id(61);
  slaveResp->set_dlc(8);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB0");
  signal->set_start_bit(0);
  slaveResp->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB1");
  signal->set_start_bit(8);
  slaveResp->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB2");
  signal->set_start_bit(16);
  slaveResp->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB3");
  signal->set_start_bit(24);
  slaveResp->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB4");
  signal->set_start_bit(32);
  slaveResp->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB5");
  signal->set_start_bit(40);
  slaveResp->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB6");
  signal->set_start_bit(48);
  slaveResp->insert_signal(signal);

  signal = new CANdbSignal;
  *signal = form_sig;
  signal->set_name("SlaveRespB7");
  signal->set_start_bit(56);
  slaveResp->insert_signal(signal);

  current_db->insert_message (slaveResp);
  return slaveResp;
}

int CANdbLDF::read_ldf_diag_frames ()
{
  LinToken token;
  int t;

  t = lex ( token );
  if (t != '{') {
    error ("'{' expected");
    return -1;
  }

  t = lex (token);
  do {
    if(t == T_IDENT) {
      if(strcmp(token.ident, "MasterReq") == 0) {
        t = lex ( token );
        if (t !=  ':') {
          error(": expected");
          return -1;
        }

        t = lex (token);
        if( (t != T_INT_CONST) || (token.int_const != 60) ) {
          error( "MaterReq must have id 60!" );
          return -1;
        }

        t = lex (token);
        if( t != '{' ) {
          error ("'{' expected");
          return -1;
        }

        /*Just ignore the signals they are
          predefined in the LIN spec.*/
        t= lex( token );
        while ((t!= '}') && (t > 0))
          t= lex( token );

        add_master_req();
      } else if(strcmp(token.ident, "SlaveResp") == 0) {
        t = lex ( token );
        if ( t !=  ':') {
          error(": expected");
          return -1;
        }

        t = lex (token);
        if( (t != T_INT_CONST) || (token.int_const != 61) ) {
          error( "SlaveReq must have id 61!" );
          return -1;
        }

        /*Just ignore the signals they are
          predefined in the LIN spec.*/
        t = lex (token);
        if( t != '{' ) {
          error ("'{' expected");
          return -1;
        }
        t= lex( token );
        while ((t!= '}') && (t > 0))
          t= lex( token );

        add_slave_resp();
      } else {
        error ("Unknown Name of Diagnostic_frame.");
        return -1;
      }
    }
    //Table name
    t = lex (token);
  } while ((t != '}') && (t > 0));

  return t;
}

int CANdbLDF::read_ldf_event_trig_frames()
{
  LinToken token;
  int t;
  t = lex (token);

  CANdbMessage *message = NULL;

  if (t != '{') {
    error ("'{' expected");
    return -1;
  }

  t = lex (token);
  do {
    message = NULL;

    if (t == T_IDENT) {
      message = new CANdbMessage;
      message->set_name (token.ident);
    } else {
      error("Frame name expected.");
      return -1;
    }

    t = lex (token);
    if (t != ':') {
      error("':' expected after frame name.");
      delete message;
      return -1;
    }

    t = lex (token);
    if (t != T_IDENT) {
      error("Collision resolver schedule table identifier expected after frame name.");
      delete message;
      return -1;
    }

    t = lex (token);
    if (t != ',') {
      error("',' expected after the schedule table id.");
      delete message;
      return -1;
    }

    t = lex (token);
    if (t == T_INT_CONST) {
      message->set_id(token.int_const);
    } else {
      error("Integer expected for the Id.");
      delete message;
      return -1;
    }

    CANdbSignal *modeSignal = new CANdbSignal();
    modeSignal->set_type(CANDB_UNSIGNED);
    modeSignal->set_unit("");
    std::string modeSignalName(message->get_name());
    modeSignalName += "_id";
    modeSignal->set_name(modeSignalName.c_str());
    modeSignal->set_mode_signal(true);
    modeSignal->set_start_bit(0);
    modeSignal->set_length(8);
    message->insert_signal(modeSignal);

    t = lex (token);
    int dlc = -1;
    do
    {
      if (t != ',') {
        error("',' expected between frame names.");
        delete message;
        return -1;
      }

      t = lex (token);
      if(t == T_IDENT) {
        auto refMessage = current_db->find_message_by_name(token.ident);
        if (refMessage == NULL) {
          error("Reference to invalid or nonexisting frame (%s)", token.ident);
          delete message;
          return -1;
        }
        
        // Add mode dependent signals for all signals in the ref message
        if (dlc == -1) {
          dlc = refMessage->get_dlc();
        }
        else if (dlc != refMessage->get_dlc()) {
          error("All ref frames must have same DLC");
          delete message;
          return -1;
        }
        message->set_dlc(dlc);
        auto refSignal = refMessage->get_first_signal();
        while (refSignal != NULL)
        {
          auto signal = new CANdbSignal();
          *signal = *refSignal;
          signal->set_mode(refMessage->get_id());
          message->insert_signal(signal);
		      refSignal = refMessage->get_next_signal();
        }
      } else {
        error("Unconditional frame name expected.");
        delete message;
        return -1;
      }

      t = lex (token);
    } while (t != ';');
    
    current_db->insert_message (message);

    t = lex (token);

  } while ((t != '}') && (t > 0));

  return t;
} // CANdbLDF::read_ldf_event_trig_frames

int CANdbLDF::read_ldf_schedule_tables ()
{
  LinToken token;
  int t;
  CANdbMessage * dbMsg = NULL;

  t = lex (token);
  if (t != '{') {
    error ("'{' expected.");
    return -1;
  }

  t = lex (token);
  do {
    CANdbScheduleTable* scheduleTable = new CANdbScheduleTable();
    if (t == T_IDENT) {
      //Table name
      scheduleTable->set_name(token.ident);
    } else {
      error("Schedule table name expected.");
	    delete scheduleTable;
      return -1;
    }

    t = lex (token);
    if (t != '{') {
      error ("'{' expected after schedule table name");
	    delete scheduleTable;
      return -1;
    }

    t = lex (token);
    do {
      if (t == T_IDENT) {
        // Check if reserved frame or node config frame
        if (strcmp(token.ident, "AssignNAD") == 0 ||
            strcmp(token.ident, "AssignFrameIdRange") == 0 ||
            strcmp(token.ident, "AssignFrameId") == 0 ||
            strcmp(token.ident, "ConditionalChangeNAD") == 0 ||
            strcmp(token.ident, "DataDump") == 0 ||
            strcmp(token.ident, "SaveConfiguration") == 0 ||
            strcmp(token.ident, "FreeFormat") == 0)
        {
          t = lex (token);
          while (t != ';') {
            t = lex (token);
          }
          t = lex (token);
          continue;
        }
        
        //frame name
        dbMsg = current_db->find_message_by_name(token.ident);
        if (dbMsg == NULL && strcmp(token.ident, "MasterReq") == 0) {
          dbMsg = add_master_req();
        } else if (dbMsg == NULL && strcmp(token.ident, "SlaveResp") == 0) {
          dbMsg = add_slave_resp();
        }
        if(dbMsg == NULL) {
          error("Reference to invalid or nonexisting frame (%s)", token.ident);
		      delete scheduleTable;
          return -1;
        }
      } else {
        error ("Frame name expected.");
		    delete scheduleTable;
        return -1;
      }

      t = lex (token);
      if (t == T_IDENT) {
        if(_stricmp("delay", token.ident) != 0) {
          error("delay expected.");
		      delete scheduleTable;
          return -1;
        }
      }

      t = lex (token);
	    double frame_delay = 0.0;
      if (t == T_INT_CONST) {
        frame_delay = token.int_const;
      } else if (t == T_DOUBLE_CONST) {
        frame_delay = token.double_const;
      } else {
        error ("Real or integer expected for delay value.");
        delete scheduleTable;
        return -1;
      }

      t = lex (token);
      if ((t != T_IDENT) || (_stricmp("ms", token.ident) !=0)) {
        error ("ms expected.");
        delete scheduleTable;
        return -1;
      }

      t = lex (token);
      if (t != ';') {
        error ("';' expected.");
        delete scheduleTable;
        return -1;
      } else {
        scheduleTable->insert_message(dbMsg, frame_delay);
      }

      t = lex (token);
    } while ((t != '}') && (t > 0));

    t = lex (token);

    // Add table to current db
    current_db->insert_schedule_table(scheduleTable);

  } while ((t != '}') && (t > 0));

  return t;
} // CANdbLDF::read_ldf_schedule_tables

// ****************************************************************************

Ldf_Encoding_Type::Ldf_Encoding_Type(char * name)
{
  Name = NULL;
  if (name) {
    Name = new char [strlen (name) + 1];
    Name = strcpy (Name, name);
  }
} // Ldf_Encoding_Type::Ldf_Encoding_Type

void Ldf_Encoding_Type::set_name(char * name)
{
  if (name) {
    if(Name) delete [] Name;
    Name = new char [strlen (name) + 1];
    Name = strcpy (Name, name);
  }
} // Ldf_Encoding_Type::set_name

Ldf_Encoding_Type::~Ldf_Encoding_Type()
{
  if (Name) delete [] Name;
  Name = NULL;
  for (int i = 0; i < (int)v_Ldf_Physical_Values.size(); i++) {
    delete v_Ldf_Physical_Values[i];
  }
  v_Ldf_Physical_Values.clear();
  for (int i = 0; i < (int)v_Ldf_Logical_Values.size(); i++) {
    delete v_Ldf_Logical_Values[i];
  }
  v_Ldf_Logical_Values.clear();
} // Ldf_Encoding_Type::~Ldf_Encoding_Type
// ****************************************************************************

Ldf_Ascii_Value::Ldf_Ascii_Value()
{
} // Ldf_Ascii_Value::Ldf_Ascii_Value

Ldf_Ascii_Value::~Ldf_Ascii_Value()
{
} // Ldf_Ascii_Value::~Ldf_Ascii_Value

// ****************************************************************************

Ldf_Bcd_Value::Ldf_Bcd_Value()
{
} // Ldf_Bcd_Value::Ldf_Bcd_Value

Ldf_Bcd_Value::~Ldf_Bcd_Value()
{
} // Ldf_Bcd_Value::~Ldf_Bcd_Value

// ****************************************************************************

Ldf_Logical_Value::Ldf_Logical_Value(int iSig_Val, char * szText_info)
{
  ISig_Val = iSig_Val;
  SzText_info = szText_info;
  if(szText_info) {
    SzText_info = new char [strlen(szText_info) + 1];
    strcpy(SzText_info, szText_info);
  }
} // Ldf_Logical_Value::Ldf_Logical_Value

Ldf_Logical_Value::~Ldf_Logical_Value()
{
  if(SzText_info) delete [] SzText_info;
} // Ldf_Logical_Value::~Ldf_Logical_Value

void Ldf_Logical_Value::Set_Text_Info (char * szText_info)
{
  if(szText_info) {
    if(SzText_info) delete [] SzText_info;
    SzText_info = new char[strlen(szText_info) + 1];
    strcpy(SzText_info, szText_info);
  }
} // Ldf_Logical_Value::Set_Text_Info

// ****************************************************************************

Ldf_Physical_Value::Ldf_Physical_Value(int iMin, int iMax, double fScale,
                                   double fOffset, char *szText_info)
: IMin(0), IMax(0), FScale(0), FOffset(0), SzText_info(NULL)
{
  IMin = iMin; IMax = iMax;
  FScale = fScale, FOffset = fOffset;
  if(szText_info) {
    SzText_info = new char [strlen (szText_info) + 1];
    strcpy(SzText_info, szText_info);
  }
} // Ldf_Physical_Value::Ldf_Physical_Value

void Ldf_Physical_Value::Set_Text_Info (char * szText_info)
{
  if(szText_info) {
    if(SzText_info) delete [] SzText_info;
    SzText_info = new char[strlen(szText_info) + 1];
    strcpy(SzText_info, szText_info);
  }
} // Ldf_Physical_Value::Set_Text_Info


Ldf_Physical_Value::~Ldf_Physical_Value()
{
  if(SzText_info) delete [] SzText_info;
} // Ldf_Physical_Value::~Ldf_Physical_Value


// ****************************************************************************

//Adds the Logical, physical, bdc and ascii values of the Encoding type.

int CANdbLDF::insert_ldf_encoding_values (Ldf_Encoding_Type * sig_enc_type)
{
  Ldf_Physical_Value * phys_val = NULL;
  Ldf_Logical_Value * log_val = NULL;

  LinToken token;
  int t;

  t = lex (token);
  do {
    phys_val = NULL;
    log_val = NULL;
    if (t == T_IDENT) {
      if (_stricmp ("logical_value", token.ident) == 0) {
        sig_enc_type->v_Ldf_Logical_Values.push_back(new Ldf_Logical_Value());
        log_val = sig_enc_type->v_Ldf_Logical_Values.back();
        t = read_ldf_logical_value(log_val);
      }
      else if (_stricmp ("physical_value", token.ident) == 0) {
        sig_enc_type->v_Ldf_Physical_Values.push_back(new Ldf_Physical_Value());
        phys_val = sig_enc_type->v_Ldf_Physical_Values.back();
        t = read_ldf_physical_value(phys_val);
      }
      else if (_stricmp ("bcd_value", token.ident) == 0) {
        t = read_ldf_bcd_value();
      }
      else if (_stricmp ("ascii_value", token.ident) == 0) {
        t = read_ldf_ascii_value();
      }
      else {
        error ("logical_value|physical_value|bcd_value|ascii_value expected.");
        return -1;
      }
    } else {
      error ("logical_value|physical_value|bcd_value|ascii_value expected.");
      return -1;
    }

    t = lex (token);

  } while ((t != '}') && (t > 0));

  return t;
} // CANdbLDF::insert_ldf_encoding_values

int CANdbLDF::read_ldf_logical_value (Ldf_Logical_Value * log_val)
{
  int t;
  LinToken token;

  t = lex (token);
  if (t != ',') {
    error("',' expected.");
    return -1;
  }

  t = lex (token);
  if (t == T_INT_CONST) {
    log_val -> ISig_Val = token.int_const;
  } else {
    error("Integer signal value expected.");
    return -1;
  }

  t = lex (token);
  if(t == ';') return t = lex (token);

  if (t != ',') {
    error("',' expected.");
    return -1;
  }

  t = lex (token);
  if (t == T_STRING_CONST) {
    log_val -> Set_Text_Info(token.string_const);
  } else {
    error("String value for the text info expected.");
    return -1;
  }

  t = lex (token);
  if (t != ';') {
    error ("';' expected.");
    return -1;
  }

  return t;
} // CANdbLDF::read_ldf_logical_value

int CANdbLDF::read_ldf_bcd_value ()
{
  int t;
  LinToken token;
  t = lex (token);
  while ((t != ';') && (t > 0)) {
    t = lex (token);
  }
  return t;
} // CANdbLDF::read_ldf_bcd_value

int CANdbLDF::read_ldf_ascii_value ()
{
  int t;
  LinToken token;
  t = lex (token);
  while ((t != ';') && (t > 0)) {
    t = lex (token);
  }
  return t;
} // CANdbLDF::read_ldf_ascii_value

int CANdbLDF::read_ldf_physical_value (Ldf_Physical_Value * phys_val)
{
  int t;
  LinToken token;

  t = lex (token);
  if (t != ',') {
    error("',' expected.");
    return -1;
  }

  t = lex (token);
  if (t == T_INT_CONST) {
    phys_val -> IMin = token.int_const;
  } else {
    error("Integer min value expected.");
    return -1;
  }

  t = lex (token);
  if (t != ',') {
    error("',' expected.");
    return -1;
  }

  t = lex (token);
  if(t == T_INT_CONST) {
    phys_val -> IMax = token.int_const;
  } else {
    error ("Integer max value expected.");
    return -1;
  }

  t = lex (token);
  if (t != ',') {
    error("',' expected.");
    return -1;
  }


  t = lex (token);
  if (t == T_INT_CONST) {
    phys_val -> FScale = token.int_const;
  } else if (t == T_DOUBLE_CONST) {
    phys_val -> FScale = token.double_const;
  } else {
    error("Real or Integer expected for Scale value.");
    return -1;
  }

  t = lex (token);
  if (t != ',') {
    error("',' expected.");
    return -1;
  }

  t = lex (token);
  if (t == T_INT_CONST) {
    phys_val -> FOffset = token.int_const;
  } else if (t == T_DOUBLE_CONST) {
    phys_val -> FOffset = token.double_const;
  } else {
    error("Real or Integer expected for Offset value.");
    return -1;
  }

  t = lex (token);
  if (t == ';') return t;


  if (t != ',') {
    error("',' or expected.");
    return -1;
  }

  t = lex (token);
  if (t == T_STRING_CONST) {
    phys_val -> Set_Text_Info(token.string_const);
  } else {
    error("String value for the text info expected.");
    return -1;
  }

  t = lex (token);
  if (t != ';') {
    error ("';' expected.");
    return -1;
  }

  return t;
} // CANdbLDF::read_ldf_physical_value


void CANdbLDF::set_db_values_for_Ldf_Encoding(CANdbSignal * sig, Ldf_Encoding_Type * enc_type)
{
  Ldf_Physical_Value * phys_val = NULL;
  Ldf_Logical_Value * log_val = NULL;
  bool bPhysicalRangeAssigned = false;
  int i = 0;

  for(i = 0; i < (int)enc_type->v_Ldf_Physical_Values.size(); i++) {
    if(bPhysicalRangeAssigned) {
      warning("Cannot assign more than one Physical value for each Encoding Type.");
      break;
    }
    bPhysicalRangeAssigned = true;
    phys_val = enc_type->v_Ldf_Physical_Values[i];
    sig->set_offset(phys_val->FOffset);
    sig->set_factor(phys_val->FScale);
    sig->set_min_val(phys_val->FScale * phys_val->IMin + phys_val->FOffset);
    sig->set_max_val(phys_val->FScale * phys_val->IMax + phys_val->FOffset);
    if(phys_val->SzText_info)
      sig->set_unit(phys_val->SzText_info);
    else
      sig->set_unit("");
  }

  for(i = 0; i < (int)enc_type->v_Ldf_Logical_Values.size(); i++) {
    log_val = enc_type->v_Ldf_Logical_Values[i];
    sig->add_value(log_val->ISig_Val, log_val->SzText_info);
    if(log_val->ISig_Val > sig->get_max_val())
      sig->set_max_val( log_val->ISig_Val );
    if(log_val->ISig_Val < sig->get_min_val())
      sig->set_min_val( log_val->ISig_Val );
	  if (sig->get_unit() == NULL)
      sig->set_unit("");
  }
} // CANdbLDF::set_db_values_for_Ldf_Encoding

int CANdbLDF::read_ldf_signal_representations ()
{
  char szError[MAX_ERROR_STRING_LEN];
  LinToken token;
  int t;
  t = lex (token);
  int i = 0;
  Ldf_Encoding_Type * enc_type = NULL;
  CANdbSignal * sig = NULL;
  bool enc_type_found = false;
  bool signal_found = false;

  if (t != '{') {
    error("'{' expected");
    return -1;
  }

  t = lex (token);

  do {
    if (t == T_IDENT) {
      enc_type_found = false;
      enc_type = NULL;
      for (i = 0; i < (int)v_Ldf_Encoding_Types.size(); i++) {
        enc_type = v_Ldf_Encoding_Types[i];
        if(strcmp(enc_type -> Name, token.ident) == 0) {
          enc_type_found = true;
          break;
        }
      }
    } else {
      error ("Signal encoding type expected.");
      return -1;
    }

    if(!enc_type_found) {
      sprintf(szError, "Reference to unknown Signal_encoding_type \"%s\".", token.ident);
      error(szError);
      return -1;
    }

    t = lex (token);
    if(t != ':') {
      error("':' expected after signal encoding type.");
      return -1;
    }

    do {
      t = lex (token);
      if ((t == T_IDENT) && enc_type) {
        CANdbMessage *message = current_db->get_first_message ();
        signal_found = false;
        while (message) {
          sig = message->get_first_signal ();
          while (sig) {
            if(strcmp(sig -> get_name(), token.ident) == 0) {
              set_db_values_for_Ldf_Encoding( sig, enc_type );
              signal_found = true;
              // Do not break, since the same signal can be used both in normal frames and in event triggered frames
              // break;
            }
            sig = message->get_next_signal ();
          }
          message = current_db->get_next_message ();
        }

        if(!signal_found) {
          sprintf(szError, "Reference to unknown Signal \"%s\".", token.ident);
          error(szError);
          return -1;
        }

        /*
        for (i = 0; i < v_Ldf_Signals.size(); i++) {
          sig = v_Ldf_Signals [i];
          if(strcmp(sig -> get_name(), token.ident) == 0) {
            set_db_values_for_Ldf_Encoding( sig, enc_type );
            break;
          }
        }*/
      } else {
        error ("Error.");
        return -1;
      }

      t = lex (token);
      if(t == ';') continue;

      if(t != ',') {
        error ("',' expected");
        return -1;
      }

    } while ((t != ';') && (t > 0));

    if (t > 0)
      t = lex (token);

  } while ((t != '}') && (t > 0));

  return t;
} // CANdbLDF::read_ldf_signal_representations


int CANdbLDF::read_ldf_signal_encoding_types()
{
  for (int i = 0; i < (int)v_Ldf_Encoding_Types.size(); i++) {
    delete v_Ldf_Encoding_Types[i];
  }
  v_Ldf_Encoding_Types.clear();
  Ldf_Encoding_Type  * sig_enc_type = NULL;
  LinToken token;
  int t;

  t = lex (token);
  if (t != '{') {
    error("'{' expected");
    return -1;
  }

  t = lex (token);

  do {
    if (t == T_IDENT) {
      v_Ldf_Encoding_Types.push_back(new Ldf_Encoding_Type(token.ident));
      sig_enc_type = v_Ldf_Encoding_Types.back();
    } else {
      error ("Signal encoding type name expected");
      return -1;
    }

    t = lex (token);
    if(t != '{') {
      error ("'{' expected after signal encoding type name.");
      return -1;
    }

    t = insert_ldf_encoding_values (sig_enc_type);

    t = lex(token);

  } while ((t != '}') && (t > 0));

  return t;
} // CANdbLDF::read_ldf_signal_encoding_types

int CANdbLDF::read_file (CANdb *db)
{
  current_db = db;
  dbLinMasterNode = NULL;
  channel_name = "";

  build_keyword_hash_table ();

  if (!get_filename ()) {
    set_read_ok (false);
    return -1;
  }

  if (!file) {
    set_read_ok (false);
    file_eof = true;
    return -1;
  }
  file_eof = false;

  int t;
  LinToken token;

  t = read_ldf_header();
  if (t < 0) return -1;

  // Add bus type attribute and set it to LIN
  CANdbAttributeDefinition *bustype_def = new CANdbAttributeDefinition();
  bustype_def->set_owner(CANDB_ATTR_OWNER_DB);
  bustype_def->set_name("BusType");
  bustype_def->set_type(CANDB_ATTR_TYPE_STRING);
  current_db->insert_attribute_definition(bustype_def);
  CANdbAttribute *bustype_attr = new CANdbAttribute(bustype_def);
  bustype_attr->set_string_value("LIN");
  CANdbAttributeList *attr_list = current_db->get_attributes();
  attr_list->insert(bustype_attr);

  t = lex(token);
  do {

    if (t == T_CHANNEL_NAME) { // Channel name
      t = read_ldf_channel_name();
      if (t < 0) return -1;
    }

    if (t == T_SIGNALS) { // Signals
      t = read_ldf_signals();
      if (t < 0) return -1;
    }

    if (t == T_NODES) { // node list
      t = read_ldf_nodes();
      if (t < 0) return -1;
    }

    if (t == T_FRAMES) { // Frames
      t = read_ldf_frames();
      if (t < 0) return -1;
    }


    if (t == T_SIG_ENC_TYPES) { // Signal_encoding_types
      t = read_ldf_signal_encoding_types();
      if (t < 0) return -1;
    }

    if (t == T_SIG_REPS ) {
      t = read_ldf_signal_representations();
      if (t < 0) return -1;
    }

    if (t == T_SCHEDULE_TABS) {
      t = read_ldf_schedule_tables();
      if (t < 0) return -1;
    }

    if (t == T_DIAG_FRAMES) {
      t= read_ldf_diag_frames();
      if (t < 0) return -1;
    }

    if (t == T_EVENT_TRIG_FRAMES) {
      t= read_ldf_event_trig_frames();
      if (t < 0) return -1;
    }

    t = lex (token);
  } while (t > 0);

  set_read_ok (true);

  // Setup internal data structures
  CANdbMessage *message = current_db->get_first_message ();
  while (message) {
    // Fix naming if Channel_name is set
    if (!channel_name.empty()) {
      std::string name = message->get_name();
      name += "_";
      name += channel_name;
      message->set_name(name.c_str());
      CANdbSignal *signal = message->get_first_signal();
      while (signal) {
        name = signal->get_name();
        name += "_";
        name += channel_name;
        signal->set_name(name.c_str());
        signal = signal->get_next();
      }
    }
    message->setup ();
    message = current_db->get_next_message ();
  }

  for (int i = 0; i < (int)v_Ldf_Encoding_Types.size(); i++) {
    delete v_Ldf_Encoding_Types[i];
  }
  v_Ldf_Encoding_Types.clear();

  for (int i = 0; i < (int)v_Ldf_Signals.size(); i++) {
    delete v_Ldf_Signals[i];
  }
  v_Ldf_Signals.clear();

  return 0;
} // CANdbLDF::read_file


CANdbFileIo* CANdbLDF::build(const char *filename)
{
  CANdbLDF *fio = new CANdbLDF(filename);
  return fio;
} // CANdbLDF::build

// ****************************************************************************
