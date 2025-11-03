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
#ifndef KVALOGREADER_VECTOR_BFL_FD_H_
#define KVALOGREADER_VECTOR_BLF_FD_H_

#include <vector>
#include "VectorBlfFd.h"
#include "KvaLogReader.h"

class KvaLogReader_VectorBlfFd : public KvaLogReader {
  public:
    KvaLogReader_VectorBlfFd();
    ~KvaLogReader_VectorBlfFd();

    KvlcStatus open_file(const char *filename);
    KvlcStatus read_row(imLogData *logEvent);
    uint64 event_count();
    bool isBinary() { return true; }

    KvlcStatus interpret_event(void *event,
                               imLogData *logEvent);

  private:
    bool zlib_compression;
    bool initial_bogus_trigger;
    bool initial_bogus_rtc;
    bool ending_bogus_rtc;
    bool eof;
    uint32_t container_pos;
    uint64_t current_eventno;
    uint64_t current_frameno;
    uint64_t end_of_measurement64;
    uint64_t last_time64;
    std::vector<unsigned char> container_data;
    KvlcStatus read_file_header();
    KvlcStatus read_next_event(char* buf, uint64_t* len);
    KvlcStatus read_next_container();
    KvlcStatus interpret_CAN_MESSAGE(void* buf, imLogData *logEvent);
    KvlcStatus interpret_CAN_MESSAGE2(void* buf, imLogData *logEvent);
    KvlcStatus interpret_CAN_FD_MESSAGE_64(void* buf, imLogData *logEvent);
    KvlcStatus interpret_CAN_ERROR (void* buf, imLogData *logEvent);
    KvlcStatus interpret_CAN_ERROR_ext (void* buf, imLogData *logEvent);
    KvlcStatus interpret_CAN_FD_ERROR_64(void* buf, imLogData *logEvent);
    KvlcStatus interpret_CAN_OVERLOAD(void* buf, imLogData *logEvent);
    KvlcStatus interpret_APP_TRIGGER(void* buf, imLogData *logEvent);
    KvlcStatus set_can_msg_common(uint32_t id,
                                  uint16_t channel,
                                  uint8_t dlc,
                                  uint64_t timestamp,
                                  imLogData *logEvent);
    KvlcStatus set_flags(uint8_t flags, imLogData *logEvent);
    KvlcStatus set_flags_fd64(uint32_t flags, imLogData *logEvent);
    KvlcStatus set_flags_error_fd(uint16_t flags, imLogData *logEvent);
    KvlcStatus read_bogus_rtc(uint64_t t_ns, uint64_t ns_since_1970, imLogData *logEvent);
    KvlcStatus read_bogus_trigger(uint64_t t_ns, uint64_t ns_since_1970, imLogData *logEvent);
};

#endif // KVALOGREADER_VECTOR_BLF_FD_H_
