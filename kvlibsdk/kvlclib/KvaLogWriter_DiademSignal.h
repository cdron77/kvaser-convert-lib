/*
**             Copyright 2021 by Kvaser AB, Molndal, Sweden
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

#ifndef KVALOGWRITER_DIADEMSIGNAL_H_
#define KVALOGWRITER_DIADEMSIGNAL_H_

#include "KvaLogWriter_Signal.h"
#include <string.h>
#include <math.h>

#define CSV_OVERRUN_STRING "# Overrun occurred during logging, messages were lost. #"

class DiademInfo;

class KvaLogWriter_DiademSignal : public KvaLogWriter_Signal {
  char outbuffer[MAX_COLUMNS * MAX_ELEM_LEN + 10000];
  long long currentSize;
public:
 enum NoValueMode {
               NvModeNever,
               NvModeTime,
               NvModeSamples,
             };
  KvaLogWriter_DiademSignal();
  ~KvaLogWriter_DiademSignal();
  KvlcStatus write_signal_header();
  KvlcStatus write_signals();
  KvlcStatus  write_signal_trailer ();
  bool isBinary() { return false; }
  long long get_bytes_written() { return currentSize; };
  unsigned int    filesize = 0;
  
  
  DiademInfo     *info = NULL;
  unsigned int    time_values;
  int getChannelCount();
  std::string outfile_name_data_without_path;
  
  
  FILE *datafile = NULL;
  KvlcStatus close_file();

  
  bool property_time_implicit = true;
  NoValueMode property_novalue_mode =  NvModeNever;
  int property_novalue_time = 1000;
  int property_novalue_samples = 100;
  // DIAdem uses NAN now
  double property_novalue_value = 9.90000000000000E+0034;
  int property_time_start = 0;
  int property_time_interval = 10;
  bool property_novalue_use_diadem_default = true;
  bool headerWritten = false;
};
#endif /*KVALOGWRITER_DIADEMSIGNAL_H_*/