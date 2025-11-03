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

// CANdbLdf.h: Read & write Vector CANdb (.dbc) files into/from  CANdb classes

#ifndef CANDB_LDF_H
#define CANDB_LDF_H

// ****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include "CANdb.h"

// ****************************************************************************

#define CANDB_LDF_MAX_STRING    1000

// ****************************************************************************

void candb_set_string (char **var, const char *s);

// ****************************************************************************



using namespace std;

union LinToken {
        int     int_const;
        double  double_const;
        char    *string_const;
        char    *ident;
        int     token;
      };

// ****************************************************************************

class Ldf_Logical_Value
{
public:
  void Set_Text_Info (char * szTextInfo);

  char* SzText_info;
  int ISig_Val;
public:

  Ldf_Logical_Value(int iSig_Val = 0, char * szText_info = NULL);
  ~Ldf_Logical_Value();
};


class Ldf_Ascii_Value
{
public:
public:
  Ldf_Ascii_Value();
  ~Ldf_Ascii_Value();
};

class Ldf_Bcd_Value
{
public:
  Ldf_Bcd_Value();
  ~Ldf_Bcd_Value();
};

class Ldf_Physical_Value
{
public:
  void Set_Text_Info (char * szTextInfo);
  int IMin, IMax;
  double FScale, FOffset;
  char * SzText_info;
public:
  Ldf_Physical_Value(int iMin = 0, int iMax = 0, double fScale = 1,
                   double fOffset = 0, char *szText_info = NULL);

  ~Ldf_Physical_Value();
};

class Ldf_Encoding_Type
{
public:
  char * Name;
  void set_name(char * name);
  std::vector<Ldf_Physical_Value*> v_Ldf_Physical_Values;
  std::vector<Ldf_Logical_Value*> v_Ldf_Logical_Values;
public:
  Ldf_Encoding_Type(char * name);
  Ldf_Encoding_Type() { Name = NULL; };
  ~Ldf_Encoding_Type();
};

class CANdbLDF : public CANdbFileIo {
      private:
        FILE    *file;
        char    string_buf [CANDB_LDF_MAX_STRING + 10];
        bool    file_eof;
        CANdb   *current_db;
        std::string channel_name;
        std::vector<Ldf_Encoding_Type*> v_Ldf_Encoding_Types;
        //XtmSignalIfNotification * notif;

      private:

        std::vector<CANdbSignal*> v_Ldf_Signals;

        int read_ldf_header();
        int read_ldf_channel_name();
        int read_ldf_signals();
        int read_ldf_nodes();
        int read_ldf_frames();
        int read_ldf_schedule_tables();
        int read_ldf_diag_frames();
        int read_ldf_event_trig_frames();

        CANdbMessage* add_master_req();
        CANdbMessage* add_slave_resp();

        int insert_ldf_signals(CANdbMessage * message, bool bSetDlc);
        int read_ldf_signal_encoding_types();
        int insert_ldf_encoding_values(Ldf_Encoding_Type * sig_enc_type);
        int read_ldf_signal_representations ();

        int read_ldf_bcd_value();
        int read_ldf_ascii_value();
        int read_ldf_physical_value(Ldf_Physical_Value * phys_val);
        int read_ldf_logical_value(Ldf_Logical_Value * log_val);

        void set_db_values_for_Ldf_Encoding(CANdbSignal * sig, Ldf_Encoding_Type * enc_type);

        char skip_space(void);
        int read_string(char c, LinToken& token);
        int read_value(char c, LinToken& token);
        int lex(LinToken& t);

      public:
        CANdbLDF(const char *filename);
        virtual ~CANdbLDF();

        virtual int read_file(CANdb *db);
        virtual int save_file(CANdb *db) { return db == NULL ? 0 : 0; };

        // static:
        static CANdbFileIo* build(const char *filename);
      };


// ****************************************************************************

#endif // CANDB_LDF_H

