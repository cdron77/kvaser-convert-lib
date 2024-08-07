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

#ifndef KVALOGWRITER_SignalRow_H_
#define KVALOGWRITER_SignalRow_H_

//#include <math.h>
#include "common_defs.h"
#include "MuxChecker.h"


#define MAX_COLUMNS           65536

#ifndef __GNUC__
  typedef unsigned __int64 time_uint64;
  typedef signed   __int64 time_int64;
#endif

typedef struct {
  // values that changes a lot
  double value;
  bool defined;
  bool missing;
  char channel;     // from 0 and up, 

  bool accessed;

  char *enumeration;

  // values that almost only changes when the signal is defined
  char numDecimals;
  char *name;
  char *unit;
  char *fullname;
  char dataType;    // signed, unsigned, float, double, etc 
  double factor;    // Note that value is prescaled
  double offset;    // Factor & offset only used as information in some formats
  double lowerLimit;    // Minimum allowed physical value for a signal
  double upperLimit;    // Maximum allowed physical value for a signal
  uint64_t message;      // Common number for all signals in a group
  MuxChecker *muxch;
} SignalType;


class SignalRow {
private:
  SignalType *values;
  int numColumns;
  char separator;
  int64_t timestamp;
  uint64_t abstime; // Nanoseconds since 1970-01-01 00:00:00
  uint32_t channel;
  bool printTimeStamp;
  bool printRTC;
  void expandIfNecessary(int column);
  bool updated;

public:
  SignalRow(int _numColumns, char _separator);
  SignalRow(char _separator); // allocates MAX_COLUMNS
  ~SignalRow();

  // Copy signal values and descriptions
  void copySignalValues(const SignalRow *other);

  int getSize();

  // Clears the entire row excluding headers
  void clearRow();

  // Clear one element excluding header
  void clearVal(int column);

  // Clear one element including header
  void clear(int column);

  // Print column header to buffer (CSV)
  int printNames(char *p);

  // Print units to buffer (CSV)
  int printUnits(char *p);

  // Will place a copy of the string in the internal memory of SignalRow at specified column
  bool setSignalValue(int column, double val);
  bool setSignalValue(int column, const char *str);
  double getSignalValue(int column);
  const char * getSignalString(int column);
  bool isSignalDefined(int column);
  bool isSignalEnumeration(int column);
  uint8_t getNumDecimals(int column);
  void setNumDecimals(int column, uint8_t numdecimals);

  // Sets the separator char
  void setSeparatorChar(char _separator);

  void setTimeStamp(int64_t ts);
  void setAbsTime(uint64_t abs);
  int64_t getTimeStamp();
  uint64_t getAbsTime();

  void setChannel(uint32_t channel);
  uint32_t getChannel();

  void setPrintProperties(bool ts, bool rtc);

  void setFullName(int column, const char *name);
  void setName(int column, const char *name);
  int getFullName(char *buf, size_t column);
  int getName(char *buf, int column);

  void setUnit(int column, const char *unit);
  int getUnit(char *buf, size_t column);

  bool isSignalMissing(int column);
  void setSignalMissing(int column);

  void setScaling(int column, double factor, double offset);
  double getFactor(int column);
  double getOffset(int column);
  void setMux(int column, int mode, void *mux_signal, MuxCallback mcb);
  bool inEvent(int column, unsigned char *event_data, unsigned int len);
  void setMessageId(int column, void* id);
  uint64_t getMessageId(int column);

  void resetUpdated();
  void resetDefined();
  bool isUpdated() { return updated; }

  bool isAccessed(int column);
  void setLimits(int column, double lower, double upper);
  double getLowerLimit(int column);
  double getUpperLimit(int column);

};

#endif /* KVALOGWRITER_SignalRow_H_ */
