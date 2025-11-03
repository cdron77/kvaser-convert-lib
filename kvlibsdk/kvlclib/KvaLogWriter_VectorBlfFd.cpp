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
#include <stdint.h>
#include "common_defs.h"
#include "KvaLogWriter_VectorBlfFd.h"
#include "kvdebug.h"
#include "os_util.h"


struct event_buffer
{
  char buffer[256];
  int length;
};

static void blf_header_v1 (obj_header_t *objHeader);


//--------------------------------------------------------------------------
KvaLogWriter_VectorBlfFd::KvaLogWriter_VectorBlfFd()
{
  header_written = 0;
  object_count = 0;
}

KvaLogWriter_VectorBlfFd::~KvaLogWriter_VectorBlfFd()
{
}

//--------------------------------------------------------------------------
static uint8_t convert_flags (unsigned flags)
{
  int dir = (flags & (canMSG_TXACK | canMSG_TXRQ)) != 0; // handle TX flags
  int rtr = (flags & canMSG_RTR) != 0;
  int wu = (flags & canMSG_WAKEUP) != 0;
  int nerr = (flags & canMSG_NERR) != 0;
  return CAN_MSG_FLAGS_EXT(dir, rtr, wu, nerr);
}

static uint32_t convert_flags_fd64 (uint32_t flags){

  uint32_t vec_flags = 0;

  if (flags & canFDMSG_FDF) vec_flags |= BLF_FD64_EDL;
  if (flags & canMSG_NERR) vec_flags |= BLF_FD64_NERR;
  if (flags & canMSG_WAKEUP) vec_flags |= BLF_FD64_WAKEUP;
  if (flags & canMSG_TXACK) vec_flags |= BLF_FD64_TXACK;
  if (flags & canMSG_TXRQ) vec_flags |= BLF_FD64_TXRQ;
  if (flags & canFDMSG_BRS) vec_flags |= BLF_FD64_BRS;
  if (flags & canFDMSG_ESI) vec_flags |= BLF_FD64_ESI;

  if (flags & canMSG_EXT) vec_flags |= BLF_FD64_SRR; // no direct mapping for ext

  return vec_flags;
}

static uint16_t convert_flags_error_fd(uint32_t flags){

  uint16_t vec_flags = 0;

  if (flags & canFDMSG_FDF) vec_flags |= 0x0001;
  if (flags & canFDMSG_BRS) vec_flags |= 0x0002;
  if (flags & canFDMSG_ESI) vec_flags |= 0x0004;

  return vec_flags;
}

//--------------------------------------------------------------------------
static void write_container_header (struct event_buffer *ev_buf, uint32_t compressed_size,
                             uint32_t uncompressed_size)
{
  obj_header objHeader;
  blf_header_v1(&objHeader);  // BLF container header is of type V1

  objHeader.base.header_size = 16;  // For some reason Vector writes 16 for header size of
                                    // container object in their BLF files
                                    // even though the actual size is 32.
                                    // Seems to work.
  objHeader.base.obj_type = BL_OBJ_TYPE_LOG_CONTAINER;
  objHeader.base.obj_size = (sizeof(objHeader)) + compressed_size;

  objHeader.obj_flags = 0;
  objHeader.obj_timestamp = uncompressed_size;

  memcpy(ev_buf->buffer, &objHeader, sizeof(objHeader));
  ev_buf->length = sizeof(objHeader);
}

//--------------------------------------------------------------------------
KvlcStatus KvaLogWriter_VectorBlfFd::write_header()
{
  char buffer[sizeof(file_header)];
  file_header *header = (file_header *)buffer;
  KvlcStatus kvstatus = kvlcOK;

  memset(buffer, 0, sizeof(buffer));
  header->signature = BL_FILE_SIGNATURE;
  header->header_size = sizeof(buffer);
  header->binlog_version = 5;
  header->options = BL_COMPRESSION_NONE;
  header->app_id = 0;
  header->app_major = 0;
  header->app_minor = 0;
  header->app_build = 0;

  if (header_written)
    {
      int64_t pos = os_ftell(outfile);
      os_fseek(outfile, 0, SEEK_SET);
      header->compressed_size = file_size;
      header->uncompressed_size = file_size;
      header->object_count = object_count;
      if (start_of_logging) {
        struct tm newtime;
        get_calendar_time(start_of_logging / ONE_BILLION, &newtime);
        header->start_time[0] = newtime.tm_year+1900;
        header->start_time[1] = newtime.tm_mon + 1;
        header->start_time[2] = newtime.tm_wday;
        header->start_time[3] = newtime.tm_mday;
        header->start_time[4] = newtime.tm_hour;
        header->start_time[5] = newtime.tm_min;
        header->start_time[6] = newtime.tm_sec;
        header->start_time[7] = 0;
      }

      if (last_clock_event) {
        struct tm newtime;
        get_calendar_time((start_of_logging + last_clock_event) / ONE_BILLION, &newtime);
        header->stop_time[0] = newtime.tm_year+1900;
        header->stop_time[1] = newtime.tm_mon + 1;
        header->stop_time[2] = newtime.tm_wday;
        header->stop_time[3] = newtime.tm_mday;
        header->stop_time[4] = newtime.tm_hour;
        header->stop_time[5] = newtime.tm_min;
        header->stop_time[6] = newtime.tm_sec;
        header->stop_time[7] = 0;
      }

      fwrite(buffer, sizeof(buffer), 1, outfile);

      struct event_buffer ev_buf;
      uint64_t compressed_size = file_size - sizeof(buffer) - 32;
      uint32_t uncompressed_size = (uint32_t)compressed_size;
      write_container_header(&ev_buf, (uint32_t)compressed_size, uncompressed_size);
      fwrite(ev_buf.buffer, ev_buf.length, 1, outfile);

      os_fseek(outfile, pos, SEEK_SET);

      return kvlcOK;
    }
  else
    {
      kvstatus = write_file(buffer, sizeof(buffer));
      if (kvstatus == kvlcOK)
        {
          memset(buffer, 0, sizeof(buffer));
          kvstatus = write_file(buffer, 32);
        }
    }

  header_written = 1;
  return kvstatus;
}

//--------------------------------------------------------------------------
static void blf_header_v1 (obj_header_t *objHeader)
{
  memset(objHeader, 0, sizeof(obj_header_t));

  objHeader->base.signature = BL_OBJ_SIGNATURE;
  objHeader->base.header_version = 1;
  objHeader->base.header_size = sizeof(obj_header_t);

  objHeader->client_index = 0;
  objHeader->obj_version = 0;
  objHeader->obj_flags = BL_OBJ_FLAG_TIME_ONE_NANS;
}

//--------------------------------------------------------------------------
static KvlcStatus write_trigger (struct event_buffer *ev_buf, obj_header objHeader, imLogData_trigger * trig)
{
  app_trigger message;
  memset(&message, 0, sizeof(message));
  message.header = objHeader;

  message.header.base.obj_size = sizeof(message);
  message.header.base.obj_type = BL_OBJ_TYPE_APP_TRIGGER;

  message.pre_trigger_time = (uint64_t) MAX(trig->preTrigger, 0);
  message.post_trigger_time = (uint64_t) MAX(trig->postTrigger, 0);

  message.channel = 1;
  message.flags = BL_TRIGGER_FLAG_SINGLE_TRIGGER;
  message.app_specific2 = 0;

  memcpy(ev_buf->buffer, &message, sizeof(message));
  ev_buf->length = sizeof(message);
  return kvlcOK;
}

//--------------------------------------------------------------------------
static KvlcStatus write_errorframe (struct event_buffer *ev_buf, obj_header objHeader, imLogData_canMessage *msg)
{
  CAN_error_frame_ext message;
  memset(&message, 0, sizeof(message));
  message.header = objHeader;

  message.header.base.obj_size = sizeof(message);
  message.header.base.obj_type = BL_OBJ_TYPE_CAN_ERROR_EXT;

  message.channel = msg->channel + 1;
  message.DLC = msg->dlc;
  message.ID = msg->id;
  message.length = 0;

  memcpy(&message.data, msg->data, 8);

  memcpy(ev_buf->buffer, &message, sizeof(message));
  ev_buf->length = sizeof(message);
  return kvlcOK;
}

//--------------------------------------------------------------------------
static KvlcStatus write_errorframe_fd (struct event_buffer *ev_buf, obj_header objHeader, imLogData_canMessage *msg)
{
  CANFD_error_frame64_t message;
  memset(&message, 0, sizeof(message));
  message.header = objHeader;

  message.header.base.obj_size = sizeof(message);
  message.header.base.obj_type = BL_OBJ_TYPE_CAN_FD_ERROR_64;

  message.channel = msg->channel + 1;
  message.ext_flags = convert_flags_error_fd(msg->flags);
  message.DLC = msg->dlc;
  message.ID = msg->id;
  memcpy(&message.data, msg->data, 64);

  memcpy(ev_buf->buffer, &message, sizeof(message));
  ev_buf->length = sizeof(message);
  return kvlcOK;
}


//--------------------------------------------------------------------------
static KvlcStatus write_overloadframe (struct event_buffer *ev_buf, obj_header objHeader, imLogData_canMessage *msg)
{
  CAN_overload_frame message;
  memset(&message, 0, sizeof(message));
  message.header = objHeader;

  message.header.base.obj_size = sizeof(message);
  message.header.base.obj_type = BL_OBJ_TYPE_CAN_OVERLOAD;

  message.channel = msg->channel + 1;
  message.dummy = 0;

  memcpy(ev_buf->buffer, &message, sizeof(message));
  ev_buf->length = sizeof(message);
  return kvlcOK;
}

static KvlcStatus write_messagefd (struct event_buffer *ev_buf, obj_header objHeader, imLogData_canMessage *msg)
{
  CANFD_message64 message;
  memset(&message, 0, sizeof(message));
  message.header = objHeader;

  message.header.base.obj_size = sizeof(message);
  message.header.base.obj_type = BL_OBJ_TYPE_CAN_FD_MESSAGE_64;

  message.channel = msg->channel + 1;
  message.flags = convert_flags_fd64(msg->flags);
  message.DLC = msg->dlc;
  if (!(msg->flags & canMSG_RTR)) {
    // RTR frames have no data, should be 0 according to spec
    message.valid_data_bytes = dlcToNumBytesFD(msg->dlc);
  }
  message.ID = msg->id;

  if (msg->flags & canMSG_EXT){
    message.ID |= BLF_ID_EXT;
  }

  memcpy(&message.data, msg->data, 64);

  memcpy(ev_buf->buffer, &message, sizeof(message));
  ev_buf->length = sizeof(message);
  return kvlcOK;
}

//--------------------------------------------------------------------------
static KvlcStatus write_message (struct event_buffer *ev_buf, obj_header objHeader, imLogData_canMessage *msg)
{

  if (msg->flags & canMSG_ERROR_FRAME)
  {
    if (msg->flags & canFDMSG_FDF){
      return write_errorframe_fd(ev_buf, objHeader, msg);
    } else {
      return write_errorframe(ev_buf, objHeader, msg);
    }
  }
  else if (msg->flags & canMSGERR_OVERRUN)
  {
    return write_overloadframe(ev_buf, objHeader, msg);
  }
  else if (msg->flags & canFDMSG_FDF)
  {
    return write_messagefd(ev_buf, objHeader, msg);
  }

  CAN_message2 message;
  memset(&message, 0, sizeof(message));
  message.header = objHeader;

  message.header.base.obj_size = sizeof(message);
  message.header.base.obj_type = BL_OBJ_TYPE_CAN_MESSAGE2;

  message.channel = msg->channel + 1;
  message.flags = convert_flags(msg->flags);
  message.DLC = msg->dlc;
  message.ID = msg->id;

  if (msg->flags & canMSG_EXT){
    message.ID |= BLF_ID_EXT;
  }

  memcpy(&message.data, msg->data, 8);

  memcpy(ev_buf->buffer, &message, sizeof(message));
  ev_buf->length = sizeof(message);
  return kvlcOK;
}

//--------------------------------------------------------------------------
KvlcStatus KvaLogWriter_VectorBlfFd::write_row(imLogData *logEvent)
{
  KvlcStatus kvstatus;
  struct event_buffer ev_buf;

  obj_header objHeader;
  blf_header_v1(&objHeader);
  objHeader.obj_timestamp = logEvent->common.time64 + get_property_offset();

  if (!isOpened) {
    PRINTF(("KvaLogWriter_VectorBlfFd::write_row, file is not opened\n"));
    return kvlcERR_FILE_ERROR;
  }

  if (!start_of_logging) setStartOfLogging(logEvent);
  last_clock_event = objHeader.obj_timestamp;

  switch (logEvent->common.type)
    {
    case ILOG_TYPE_TRIGGER:
      kvstatus = write_trigger(&ev_buf, objHeader, &logEvent->trig);
      break;

    case ILOG_TYPE_MESSAGE:
      if (logEvent->msg.flags & canMSGERR_OVERRUN) {
        overrun_occurred = true;
      }

      kvstatus = write_message(&ev_buf, objHeader, &logEvent->msg);
      break;

    case ILOG_TYPE_RTC:
    case ILOG_TYPE_CANOTHER:
    case ILOG_TYPE_VERSION:
      // Nothing corresponding in BLF.
      ev_buf.length = 0;
      kvstatus = kvlcOK;
      break;

    default:
      PRINTF(("logEvent->type = %d, frame_counter = %d\n",
              logEvent->common.type,
              logEvent->common.event_counter));
      kvstatus = kvlcERR_INVALID_LOG_EVENT;
      break;
    }

  if (kvstatus == kvlcOK && ev_buf.length > 0)
    {
      kvstatus = write_file(ev_buf.buffer, ev_buf.length);
      object_count += 1;
    }

  return kvstatus;
}

KvlcStatus KvaLogWriter_VectorBlfFd::close_file()
{
  header_written = 0;
  object_count = 0;
  return KvaLogWriter::close_file();
}

#define NAME        "Vector BLF FD"
#define EXTENSION   "blf"
#define DESCRIPTION "CAN frames in Vector BLF format"

static class KvaWriterMaker_VectorBlfFd : public KvaWriterMaker
{
  public:
    KvaWriterMaker_VectorBlfFd() : KvaWriterMaker(KVLC_FILE_FORMAT_VECTOR_BLF_FD) {
      propertyList[KVLC_PROPERTY_START_OF_MEASUREMENT] = true;
      propertyList[KVLC_PROPERTY_FIRST_TRIGGER] = true;
      propertyList[KVLC_PROPERTY_USE_OFFSET] = true;
      propertyList[KVLC_PROPERTY_OFFSET] = true;
      propertyList[KVLC_PROPERTY_CROP_PRETRIGGER] = true;
      propertyList[KVLC_PROPERTY_TIME_LIMIT] = true;
      propertyList[KVLC_PROPERTY_OVERWRITE] = true;
    }
    int getName(char *str) { return sprintf(str, "%s", NAME); }
    int getExtension(char *str) { return sprintf(str, "%s", EXTENSION); }
    int getDescription(char *str) { return sprintf(str, "%s", DESCRIPTION); }

  private:
    KvaLogWriter *OnCreateWriter() {
      return new KvaLogWriter_VectorBlfFd();
    }
}  registerKvaLogWriter_VectorBlfFd;
