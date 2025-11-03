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
#include "kvlclib.h"
#include "kvaConverter.h"
#include "kvdebug.h"
#include "zlib.h"

#include "TimeConv.h"

#include "KvaLogReader_VectorBlfFd.h"

const uint32_t CHUNK_MAXLEN = 64000;

// static void printfileheader(file_header* header);
// static void dump_container(std::vector<unsigned char> container);
// void print_header_base(obj_header_base* base);


// static void printfileheader(file_header* header){
//   char signature[5];
//   memset(signature, 0, sizeof signature);
//   memcpy(signature, &(header->signature), sizeof header->signature);

//   PRINTF(("== fileheader:\n"));
//   PRINTF(("signature          %s\n", signature));
//   PRINTF(("header_size        %u\n", header->header_size));
//   PRINTF(("binlog_version     %u\n", header->binlog_version));
//   PRINTF(("app_id             %u\n", header->app_id));
//   PRINTF(("options            %u\n", header->options));
//   PRINTF(("app_major          %u\n", header->app_major));
//   PRINTF(("app_minor          %u\n", header->app_minor));
//   PRINTF(("compressed_size    %zu\n", header->compressed_size));
//   PRINTF(("uncompressed_size  %zu\n", header->uncompressed_size));
//   PRINTF(("object_count       %u\n", header->object_count));
//   PRINTF(("app_build          %u\n", header->app_build));
//   PRINTF(("start year         %u\n", header->start_time[0]));
//   PRINTF(("start month        %u\n", header->start_time[1]));
//   PRINTF(("start weekday      %u\n", header->start_time[2]));
//   PRINTF(("start day          %u\n", header->start_time[3]));
//   PRINTF(("start hour         %u\n", header->start_time[4]));
//   PRINTF(("start minute       %u\n", header->start_time[5]));
//   PRINTF(("start second       %u\n", header->start_time[6]));
//   PRINTF(("start millisecond  %u\n", header->start_time[7]));
//   PRINTF(("stop year          %u\n", header->stop_time[0]));
//   PRINTF(("stop month         %u\n", header->stop_time[1]));
//   PRINTF(("stop weekday       %u\n", header->stop_time[2]));
//   PRINTF(("stop day           %u\n", header->stop_time[3]));
//   PRINTF(("stop hour          %u\n", header->stop_time[4]));
//   PRINTF(("stop minute        %u\n", header->stop_time[5]));
//   PRINTF(("stop second        %u\n", header->stop_time[6]));
//   PRINTF(("stop millisecond   %u\n", header->stop_time[7]));
// }

// void print_header_base(obj_header_base* base){
//     char sign[5];
//     memset(sign, 0, 5);
//     memcpy(sign, &(base->signature), 4);

//     PRINTF(("signature:          %s\n", sign));
//     PRINTF(("header_size:        %u\n", base->header_size));
//     PRINTF(("header_version:     %u\n", base->header_version));
//     PRINTF(("obj_size:           %u\n", base->obj_size));
//     PRINTF(("obj_type:           %u\n", base->obj_type));
// }

// static void dump_container(std::vector<unsigned char> container){
//   const char* fname = "data.dat";
//   FILE* fp;
//   PRINTF(("=== Dumping container data to %s\n",fname));
//   PRINTF(("=== container length = %d\n",container.size()));

//   fp = fopen(fname, "wb");

//   for (auto it=container.begin(); it!=container.end(); ++it){
//     fwrite(&*it, 1, 1, fp);
//   }
//   fclose(fp);
// }

KvlcStatus KvaLogReader_VectorBlfFd::open_file(const char *filename){
  KvlcStatus status;

  status = KvaLogReader::open_file(filename);
  if (status != kvlcOK) {
    return status;
  }

  return read_file_header();
}

KvlcStatus KvaLogReader_VectorBlfFd::read_file_header(){
  KvlcStatus stat;
  char buffer[256];
  file_header header;

  stat = read_file((char*)&header, sizeof(header));
  if (stat != kvlcOK){
    return stat;
  }

  // if header size has changed
  if (sizeof(header) < header.header_size){
    stat = read_file(buffer, header.header_size - sizeof(header));
    if (stat != kvlcOK){
      return stat;
    }
  }

  // header.options is zlib compression level 0..9
  if (header.options > 0 && header.options < 10) {
    zlib_compression = true;
  } else if (header.options == 0) {
    zlib_compression = false;
  } else {
    return kvlcFail;
  }


  start_of_measurement64 = convert_time(header.start_time[0],   // y
                                        header.start_time[1],   // m
                                        header.start_time[3],   // d
                                        header.start_time[4],   // h
                                        header.start_time[5],   // m
                                        header.start_time[6]);  // s



  // convert_time() returns seconds. Convert to ns
  start_of_measurement64 *= ONE_BILLION;

  // add the ms component as ns
  start_of_measurement64 += header.start_time[7] * 1000000; // ms

  end_of_measurement64 = convert_time(header.stop_time[0],   // y
                                        header.stop_time[1],   // m
                                        header.stop_time[3],   // d
                                        header.stop_time[4],   // h
                                        header.stop_time[5],   // m
                                        header.stop_time[6]);  // s

  end_of_measurement64 *= ONE_BILLION;
  end_of_measurement64 += header.stop_time[7] * 1000000; // ms

  // printfileheader(&header);

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::read_next_event(char* buf, uint64_t* len){
  int ret;
  obj_header_base* base = (obj_header_base*)buf;

  if (container_data.size() == 0)
  {
    ret = read_next_container();
    if (ret!=kvlcOK)
    {
      return ret;
    }
  }

  // Skip up to 3 NULL characters at beginning
  {
    int i;
    for (i=0; i<4; i++){
      if (container_pos + i >= container_data.size()) {
        break;
      }

      if (container_data[container_pos + i] != 0) {
        break;
      }
    }

    if (i>3){
      PRINTF(("Error during parsing: File pos: %d\n", file_position));
      return kvlcERR_CONVERTING;
    } else {
      container_pos += i;
    }
  }

  if (container_pos >= container_data.size())
  {
    ret = read_next_container();
    if (ret != kvlcOK)
    {
      return ret;
    }
  }


  // partial obj_header_base (end of container)
  if (container_data.size() - container_pos < sizeof(obj_header_base))
  {
    // read first part of header
    uint32_t part1_size = (uint32_t)container_data.size() - container_pos;
    uint32_t part2_size = (uint32_t)sizeof(obj_header_base)-part1_size;
    memcpy(buf, container_data.data() + container_pos, part1_size);
    container_pos += part1_size;

    // open next container and read rest of header
    ret = read_next_container();
    if (ret != kvlcOK) {
      return ret;
    }

    memcpy(buf+part1_size, container_data.data() + container_pos, part2_size);
    container_pos += part2_size;
  }
  else
  {
    memcpy(buf, container_data.data() + container_pos, sizeof(obj_header_base));
    container_pos += sizeof(obj_header_base);
  }


  // check signature
  if (base->signature != BL_OBJ_SIGNATURE)
  {
    PRINTF(("Error during parsing: No object signature found. File pos: %d\n", file_position));
    return kvlcERR_INTERNAL_ERROR;
  }

  // obj_size larger than buffer?
  if (base->obj_size > *len){
    PRINTF(("Error during parsing: Object too large for buffer. File pos: %d\n", file_position));
    return kvlcERR_CONVERTING;
  }

  *len = base->obj_size;


  // check if rest of event continues outside of bounds
  if (container_pos + base->obj_size - sizeof(obj_header_base) > container_data.size()) {
    // partial event at the end of containers!
    // copy first part of overlapping event to buffer

    uint32_t part1_size = (uint32_t)container_data.size() -  container_pos;
    uint32_t part2_size = base->obj_size - part1_size - sizeof(obj_header_base);

    memcpy(buf+sizeof(obj_header_base), container_data.data() + container_pos, part1_size);
    container_pos = (uint32_t)container_data.size();

    // read next container
    ret = read_next_container();
    if (ret!=kvlcOK){
      return ret;
    }

    memcpy(buf + sizeof(obj_header_base) + part1_size, container_data.data() + container_pos, part2_size);
    container_pos += part2_size;

  } else {
    memcpy(buf+sizeof(obj_header_base), container_data.data() + container_pos, base->obj_size - sizeof(obj_header_base));
    container_pos += base->obj_size - sizeof(obj_header_base);
  }

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd:: read_next_container(){
  int ret;
  int have;
  KvlcStatus stat;
  unsigned char in[CHUNK_MAXLEN];
  unsigned char out[CHUNK_MAXLEN];
  z_stream strm;
  obj_header head;

  size_t read_len;
  size_t container_len;

  char buf[sizeof(obj_header)+3];

  container_data.clear();


  // blf objects may have 1..3 trailing NULL characters apparantly
  // with this in mind try to read container object header

  stat = read_file((char*)buf, sizeof(obj_header));
  if (stat != kvlcOK){
    PRINTF(("Error during file read. File pos: %jd\n", file_position));
    return stat;
  }

  if (buf[0]!=0)
  {
    memcpy(&head, &buf, sizeof(obj_header));
  } else
  {
    int i=0;

    // count null characters
    for(i=1; i<4; i++){
      if (buf[i]!=0)
        break;
    }

    // should be max 3 (most seen with either containers or events)
    if (i==4)
    {
      PRINTF(("Expected at most 3 leading null before container\n"));
      return kvlcERR_CONVERTING;
    } else
    {
      // read the rest of the header
      stat = read_file(buf+sizeof(obj_header), i);
      if (stat != kvlcOK)
      {
        PRINTF(("Error during file read. File pos: %jd\n", file_position));
        return stat;
      }
      memcpy(&head, buf+i, sizeof(obj_header));
    }
  }


  if (head.base.signature!=BL_OBJ_SIGNATURE){
    PRINTF(("Errpr: No obj signature found for container. File pos: %jd\n", file_position));
    return kvlcERR_CONVERTING;
  }

  if (head.base.obj_size < sizeof(head)) {
    PRINTF(("Format error. Invalid object size specified. File pos: %jd\n", file_position));
    return kvlcERR_CONVERTING;
  }
  container_len = head.base.obj_size - sizeof(head);


  // no compression
  if (!zlib_compression){
    do {
      read_len = MIN(CHUNK_MAXLEN, container_len - container_data.size());
      stat = read_file((char*)in, read_len);
      try {
        container_data.insert(container_data.end(), &in[0], &in[read_len]);
      } catch (const std::exception &e) {
        (void) e;
        PRINTF(("Error: Failure during buffering. File pos: %jd\n", file_position));
        return kvlcERR_BUFFER_SIZE;
      }
    } while(container_data.size() < container_len);

  // compression
  } else {

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);

    if (ret != Z_OK) {
      PRINTF(("Error during decompression, file_pos: %jd\n"));
      return kvlcERR_CONVERTING;
    }

    /* decompress until deflate stream ends or end of file */
    do {
      read_len = MIN(CHUNK_MAXLEN, container_len - container_data.size());
      strm.avail_in = (uInt) read_len;

      stat = read_file((char*)in, read_len);

      if (stat != kvlcOK) {
        (void)inflateEnd(&strm);
        PRINTF(("Error during file read. File pos: %jd\n", file_position));
        return stat;
      }

      if (strm.avail_in == 0){
          break;
      }
      strm.next_in = in;
      /* run inflate() on input until output buffer not full */
      do {
        strm.avail_out = CHUNK_MAXLEN;
        strm.next_out = out;

        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        switch (ret) {
        case Z_NEED_DICT:
          ret = Z_DATA_ERROR;
          PRINTF(("Error during decompression: Inflate dictionary error. File pos: %jd\n", file_position));
          return kvlcERR_CONVERTING;
        case Z_DATA_ERROR:
          PRINTF(("Error during decompression: Inflate stream data error. File pos: %jd\n", file_position));
          return kvlcERR_CONVERTING;
        case Z_MEM_ERROR:
          (void)inflateEnd(&strm);
          PRINTF(("Error during decompression: Memory allocation. File pos: %jd\n", file_position));
          return kvlcERR_CONVERTING;
        }
        have = CHUNK_MAXLEN - strm.avail_out;
        try {
          container_data.insert(container_data.end(), &out[0], &out[have]);
        } catch (const std::exception &e) {
          (void) e;
          PRINTF(("Error during decompression: Failure during buffering. File pos: %jd\n", file_position));
          return kvlcERR_BUFFER_SIZE;
        }

      } while (strm.avail_out == 0);

      /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);

    if (ret != Z_STREAM_END) {
      PRINTF(("Error during decompression: Inflate stream did not reach end. File pos: %jd\n", file_position));
      return kvlcERR_CONVERTING;
    }
  }

  if (container_data.size() > UINT32_MAX){
    PRINTF(("Error during decompression: Container object too large (> %lu). File pos = %jd\n", UINT32_MAX, file_position));
    return kvlcERR_CONVERTING;
  }

  if (container_data.size() == 0){
      return kvlcEOF;
  }

  container_pos = 0;

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::read_row(imLogData *logEvent){

  KvlcStatus stat;
  obj_header_base base;

  char buf[2048];
  uint64_t len = sizeof(buf);

  // TO GET TIMESTAMPS WORKING:
  if (initial_bogus_rtc){
    initial_bogus_rtc = false;
    return read_bogus_rtc(0, start_of_measurement64, logEvent);
  }

  // TO GET TIMESTAMPS WORKING:
  if (initial_bogus_trigger){
    initial_bogus_trigger = false;
    return read_bogus_trigger(0, start_of_measurement64, logEvent);
  }

  if (eof){
    return kvlcEOF;
  }

  memset(&base, 0, sizeof(obj_header_base));

  stat = read_next_event(buf, &len);
  if (stat != kvlcOK){

    // TO GET TIMESTAMPS WORKING:
    if (stat == kvlcEOF && ending_bogus_rtc){
      ending_bogus_rtc = false;
      eof = true;
      return read_bogus_rtc(last_time64, end_of_measurement64, logEvent);
    }
    return stat;
  }

  return interpret_event(buf, logEvent);
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_event(void *event, imLogData *logEvent){

  uint32_t* signature;
  KvlcStatus status;

  signature = (uint32_t*) event;

  if (*signature != BL_OBJ_SIGNATURE) {
    char buf[5];
    memcpy(buf, (char*)signature, 4);
    PRINTF(("Error: No object signature found. File pos: \n", file_position));

    return kvlcERR_INVALID_LOG_EVENT;
  }

  switch (((obj_header_base*)event)->obj_type){
    case BL_OBJ_TYPE_CAN_MESSAGE:
      status = interpret_CAN_MESSAGE(event, logEvent);
      break;
    case BL_OBJ_TYPE_CAN_MESSAGE2:
      status = interpret_CAN_MESSAGE2(event, logEvent);
      break;
    case BL_OBJ_TYPE_CAN_FD_MESSAGE_64:
      status = interpret_CAN_FD_MESSAGE_64(event, logEvent);
      break;
    case BL_OBJ_TYPE_CAN_ERROR:
      status = interpret_CAN_ERROR(event, logEvent);
      break;
    case BL_OBJ_TYPE_CAN_ERROR_EXT:
      status = interpret_CAN_ERROR_ext(event, logEvent);
      break;
    case BL_OBJ_TYPE_CAN_FD_ERROR_64:
      status = interpret_CAN_FD_ERROR_64(event, logEvent);
      break;
    case BL_OBJ_TYPE_APP_TRIGGER:
      status = interpret_APP_TRIGGER(event, logEvent);
      break;
    case BL_OBJ_TYPE_CAN_OVERLOAD:
      status = interpret_CAN_OVERLOAD(event, logEvent);
      break;
    case BL_OBJ_TYPE_LOG_CONTAINER:
      PRINTF(("Error: Expected log object but got container object! File pos: %jd\n", file_position));
      return kvlcERR_CONVERTING;
    default:
      logEvent->common.type = ILOG_TYPE_EXTERNAL;
      status = kvlcOK;
  }

  last_time64 = logEvent->common.time64;

  return status;
}

KvlcStatus KvaLogReader_VectorBlfFd::read_bogus_rtc(uint64_t t_ns, uint64_t ns_since_1970, imLogData *logEvent){

  logEvent->common.type = ILOG_TYPE_RTC;
  logEvent->common.new_data = true;
  logEvent->common.time64 = t_ns;
  logEvent->common.nanos_since_1970 = ns_since_1970;
  logEvent->common.new_data = true;

  return kvlcOK;
}
KvlcStatus KvaLogReader_VectorBlfFd::read_bogus_trigger(uint64_t t_ns, uint64_t ns_since_1970, imLogData *logEvent){
  logEvent->common.type = ILOG_TYPE_TRIGGER;


  logEvent->trig.preTrigger = (uint32_t)ns_since_1970;
  logEvent->trig.postTrigger = (uint32_t)ns_since_1970;

  logEvent->trig.common.time64 = t_ns;

  logEvent->trig.active = true;

  logEvent->common.new_data = true;

  current_eventno++;
  return kvlcOK;
}


KvlcStatus KvaLogReader_VectorBlfFd::interpret_CAN_MESSAGE(void* buf, imLogData *logEvent){
  CAN_message* blf_msg = (CAN_message*) buf;

  set_can_msg_common(blf_msg->ID, blf_msg->channel, blf_msg->DLC, blf_msg->header.obj_timestamp, logEvent);
  set_flags((uint8_t)blf_msg->flags, logEvent);
  if (blf_msg->ID & BLF_ID_EXT){
    blf_msg->ID &= ~BLF_ID_EXT;
  } else {
    logEvent->msg.flags |= canMSG_STD;
  }
  memcpy(logEvent->msg.data, blf_msg->data, 8);

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_CAN_MESSAGE2(void* buf, imLogData *logEvent){
  CAN_message2* blf_msg = (CAN_message2*) buf;

  set_can_msg_common(blf_msg->ID, blf_msg->channel, blf_msg->DLC, blf_msg->header.obj_timestamp, logEvent);
  set_flags((uint8_t)blf_msg->flags, logEvent);
  if (blf_msg->ID & BLF_ID_EXT){
    blf_msg->ID &= ~BLF_ID_EXT;
    logEvent->msg.flags |= canMSG_EXT;
  } else {
    logEvent->msg.flags |= canMSG_STD;
  }
  memcpy(logEvent->msg.data, blf_msg->data, 8);

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_CAN_FD_MESSAGE_64(void* buf, imLogData *logEvent) {
  CANFD_message64* blf_msg = (CANFD_message64*) buf;

  set_can_msg_common(blf_msg->ID, blf_msg->channel, blf_msg->DLC, blf_msg->header.obj_timestamp, logEvent);
  set_flags_fd64(blf_msg->flags, logEvent);

  if (blf_msg->ID & BLF_ID_EXT){
    blf_msg->ID &= ~BLF_ID_EXT;
    logEvent->msg.flags |= canMSG_EXT;
  } else {
    logEvent->msg.flags |= canMSG_STD;
  }

  memcpy(logEvent->msg.data, blf_msg->data, 64);

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_CAN_ERROR (void* buf, imLogData *logEvent){
  CAN_error_frame* blf_msg = (CAN_error_frame*) buf;

  set_can_msg_common(0, blf_msg->channel, 0, blf_msg->header.obj_timestamp, logEvent);
  logEvent->msg.flags |= canMSG_ERROR_FRAME;

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_CAN_ERROR_ext (void* buf, imLogData *logEvent){
  CAN_error_frame_ext* blf_msg = (CAN_error_frame_ext*) buf;

  set_can_msg_common(blf_msg->ID, blf_msg->channel, blf_msg->DLC, blf_msg->header.obj_timestamp, logEvent);
  logEvent->msg.flags = 0;
  if (blf_msg->ID & BLF_ID_EXT){
    blf_msg->ID &= ~BLF_ID_EXT;
    logEvent->msg.flags |= canMSG_EXT;
  } else {
    logEvent->msg.flags |= canMSG_STD;
  }
  logEvent->msg.flags |= canMSG_ERROR_FRAME;

  memcpy(logEvent->msg.data, blf_msg->data, 8);

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_CAN_FD_ERROR_64(void* buf, imLogData *logEvent){
  CANFD_error_frame64* blf_msg = (CANFD_error_frame64*) buf;

  set_can_msg_common(blf_msg->ID, blf_msg->channel, blf_msg->DLC, blf_msg->header.obj_timestamp, logEvent);
  set_flags_error_fd(blf_msg->ext_flags, logEvent);

  if (blf_msg->ID & BLF_ID_EXT){
    blf_msg->ID &= ~BLF_ID_EXT;
    logEvent->msg.flags |= canMSG_EXT;
  } else {
    logEvent->msg.flags |= canMSG_STD;
  }
  logEvent->msg.flags |= canMSG_ERROR_FRAME;

  memcpy(logEvent->msg.data, blf_msg->data, 64);

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_CAN_OVERLOAD(void* buf, imLogData *logEvent){

  CAN_overload_frame* blf_msg = (CAN_overload_frame*) buf;
  set_can_msg_common(0, blf_msg->channel, 0, blf_msg->header.obj_timestamp, logEvent);
  logEvent->msg.flags |= canMSGERR_OVERRUN;
  logEvent->msg.flags |= canMSG_STD;
  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::interpret_APP_TRIGGER(void* buf, imLogData *logEvent){
  app_trigger* blf_msg = (app_trigger*) buf;

  logEvent->trig.preTrigger = (int32_t)blf_msg->pre_trigger_time;
  logEvent->trig.postTrigger = (uint32_t)blf_msg->post_trigger_time;

  logEvent->trig.common.time64 = blf_msg->header.obj_timestamp;

  logEvent->trig.active = true;

  logEvent->common.type = ILOG_TYPE_TRIGGER;
  logEvent->common.new_data = true;

  current_eventno++;

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::set_can_msg_common(uint32_t id, uint16_t channel, uint8_t dlc, uint64_t timestamp, imLogData *logEvent){

  logEvent->common.type = ILOG_TYPE_MESSAGE;
  logEvent->common.new_data = true;

  logEvent->msg.id = id;

  logEvent->msg.channel = channel - 1;
  logEvent->msg.dlc = dlc;

  logEvent->common.time64 = timestamp;
  logEvent->msg.frame_counter = (unsigned long) ++current_frameno;
  current_eventno++;

  return kvlcOK;
}

KvlcStatus KvaLogReader_VectorBlfFd::set_flags(uint8_t flags, imLogData *logEvent){

  logEvent->msg.flags = 0;

  if (CAN_MSG_NERR(flags))
    logEvent->msg.flags |= canMSG_NERR;

  if (CAN_MSG_RTR(flags))
    logEvent->msg.flags |= canMSG_RTR;

  if (CAN_MSG_WU(flags))
    logEvent->msg.flags |= canMSG_WAKEUP;

  if (CAN_MSG_DIR(flags))
    logEvent->msg.flags |= canMSG_TXACK; // Or canMSG_TXRQ!

  return kvlcOK;
}


KvlcStatus KvaLogReader_VectorBlfFd::set_flags_fd64(uint32_t flags, imLogData *logEvent){

  logEvent->msg.flags = 0;

  logEvent->msg.flags |= canFDMSG_FDF;

  if (flags & BLF_FD64_NERR) logEvent->msg.flags |= canMSG_NERR;
  if (flags & BLF_FD64_WAKEUP) logEvent->msg.flags |= canMSG_WAKEUP;
  if (flags & BLF_FD64_TXACK) logEvent->msg.flags |= canMSG_TXACK;
  if (flags & BLF_FD64_TXRQ) logEvent->msg.flags |= canMSG_TXRQ;
  if (flags & BLF_FD64_BRS) logEvent->msg.flags |= canFDMSG_BRS;
  if (flags & BLF_FD64_ESI) logEvent->msg.flags |= canFDMSG_ESI;

  // use SRR to infer EXT ID
  if (flags & BLF_FD64_SRR) logEvent->msg.flags |= canMSG_EXT;

  return kvlcOK;
}


// flags_ext (mFlagsExt)
KvlcStatus KvaLogReader_VectorBlfFd::set_flags_error_fd(uint16_t flags, imLogData *logEvent){
  logEvent->msg.flags = 0;

  if (flags & 0x0001) logEvent->msg.flags |= canFDMSG_FDF;
  if (flags & 0x0002) logEvent->msg.flags |= canFDMSG_BRS;
  if (flags & 0x0004) logEvent->msg.flags |= canFDMSG_ESI;

  return kvlcOK;
}


uint64 KvaLogReader_VectorBlfFd::event_count(){
  return -1;
}

KvaLogReader_VectorBlfFd::KvaLogReader_VectorBlfFd(){
  container_pos = 0;
  current_eventno = 0;
  current_frameno = 0;
  last_time64 = 0;
  initial_bogus_rtc = true;
  initial_bogus_trigger = true;
  ending_bogus_rtc = true;
  eof = false;
}

KvaLogReader_VectorBlfFd::~KvaLogReader_VectorBlfFd(){
}

class KvaReaderMaker_VectorBlfFd : public KvaReaderMaker
{
  public:
    KvaReaderMaker_VectorBlfFd() : KvaReaderMaker(KVLC_FILE_FORMAT_VECTOR_BLF_FD) {}
    int getName(char *str) { return sprintf(str, "%s", "Vector BLF FD"); }
    int getExtension(char *str) { return sprintf(str, "%s", "blf"); }
    int getDescription(char *str) { return sprintf(str, "%s", "CAN frames in Vector BLF format"); }

  private:
    KvaLogReader *OnCreateReader() {
      return new KvaLogReader_VectorBlfFd();
    }
}  registerKvaLogReader_VectorBlfFd;
