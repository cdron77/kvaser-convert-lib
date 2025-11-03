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
#ifndef VECTOR_BFL_FD_H_
#define VECTOR_BFL_FD_H_

#include "common_defs.h"

#define BL_APPID_UNKNOWN          0
#define BL_APPID_CANALYZER        1
#define BL_APPID_CANOE            2
#define BL_APPID_CANSTRESS        3
#define BL_APPID_CANLOG           4
#define BL_APPID_CANAPE           5

#define BL_COMPRESSION_NONE     0
#define BL_COMPRESSION_SPEED    1
#define BL_COMPRESSION_DEFAULT  6
#define BL_COMPRESSION_MAX      9

#define BL_OBJ_SIGNATURE                0x4A424F4C
#define BL_FILE_SIGNATURE               0x47474F4C


#define BL_OBJ_FLAG_TIME_ONE_NANS       2
#define BL_TRIGGER_FLAG_SINGLE_TRIGGER  0


#define BL_OBJ_TYPE_UNKNOWN                       0       /* unknown object */
#define BL_OBJ_TYPE_CAN_MESSAGE                   1       /* CAN message object */
#define BL_OBJ_TYPE_CAN_ERROR                     2       /* CAN error frame object */
#define BL_OBJ_TYPE_CAN_OVERLOAD                  3       /* CAN overload frame object */
// #define BL_OBJ_TYPE_CAN_STATISTIC                 4       /* CAN driver statistics object */
#define BL_OBJ_TYPE_APP_TRIGGER                   5       /* application trigger object */

#define BL_OBJ_TYPE_LOG_CONTAINER                10       /* container object */

#define BL_OBJ_TYPE_CAN_ERROR_EXT                73

#define BL_OBJ_TYPE_CAN_MESSAGE2                 86   /* CAN message object - extended */

#define BL_OBJ_TYPE_CAN_FD_MESSAGE_64           101   /*CAN FD message object */
#define BL_OBJ_TYPE_CAN_FD_ERROR_64             104   /*CAN FD Error Frame object */

// get flags
#define CAN_MSG_DIR( f)          ( uint8_t)(   f & 0x0F)
#define CAN_MSG_RTR( f)          ( uint8_t)( ( f & 0x80) >> 7)
#define CAN_MSG_WU( f)           ( uint8_t)( ( f & 0x40) >> 6)
#define CAN_MSG_NERR( f)         ( uint8_t)( ( f & 0x20) >> 5)

// set flags
#define CAN_MSG_FLAGS_EXT( dir, rtr, wu, nerr) \
  (((rtr&1) << 7) + ((wu&1) << 6) + ((nerr&1) << 5) + (dir&0xF))






// VBLCANFDMessage only (seems deprecated but could support it if someone complains)


/*
#define CAN_MSG_FLAGS( dir, rtr) ( uint8_t)( ( ( uint8_t)( rtr & 0x01) << 7) | \
                                               ( uint8_t)( dir & 0x0F))
#define CAN_MSG_FLAGS_EXT( dir, rtr, wu, nerr) \
                                 ( uint8_t)( ( ( uint8_t)( rtr  & 0x01) << 7) | \
                                             ( ( uint8_t)( wu   & 0x01) << 6) | \
                                             ( ( uint8_t)( nerr & 0x01) << 5) | \
                                               ( uint8_t)( dir  & 0x0F))
*/

#define BLF_ID_EXT 0x80000000

// VBLCANFDMessage64 only
#define BLF_FD64_NERR 0x0004
#define BLF_FD64_WAKEUP 0x0008
#define BLF_FD64_TXACK 0x0040
#define BLF_FD64_TXRQ 0x0080
#define BLF_FD64_SRR 0x0200
#define BLF_FD64_EDL 0x1000 // same as FDF
#define BLF_FD64_BRS 0x2000
#define BLF_FD64_ESI 0x4000


typedef struct file_header_t{
    uint32_t signature;       // "LOGG", no null termination (verify)
    uint32_t header_size;       // 144 all files so far
    uint32_t binlog_version;    // binlog.h {major, minor, build, patch} in decimal form ABBCCDD
    uint8_t app_id;             // BL_APPID_xxx
    uint8_t options;            // BL_COMPORESSION_xxx = zlib flags
    uint8_t app_major;
    uint8_t app_minor;
    uint64_t compressed_size;   // actual file size
    uint64_t uncompressed_size;
    uint32_t object_count;
    uint32_t app_build;
    uint16_t start_time[8];     // {y, m, weekday, day, h, m, s, ms}
    uint16_t stop_time[8];      // {y, m, weekday, day, h, m, s, ms}
    uint8_t unknown[72];
} file_header;


typedef struct obj_header_base_t
{
  uint32_t     signature;
  uint16_t     header_size;
  uint16_t     header_version;
  uint32_t     obj_size;
  uint32_t     obj_type;
} obj_header_base;


typedef struct obj_header_t
{
  obj_header_base       base;
  uint32_t              obj_flags;
  uint16_t              client_index;
  uint16_t              obj_version;
  uint64_t              obj_timestamp;
} obj_header;

typedef struct obj_header2_t
{
  obj_header_base       base;
  uint32_t              obj_flags;
  uint8_t               timestamp_status;
  uint8_t               reserved;
  uint16_t              objectVersion;
  uint64_t              obj_timestamp;
  uint64_t              original_timestamp;
} obj_header2;

//BL_OBJ_TYPE_CAN_MESSAGE: 1
typedef struct CAN_message_t
{
  obj_header    header;
  uint16_t      channel;
  uint8_t       flags;
  uint8_t       DLC;
  uint32_t      ID;
  uint8_t       data[8];
} CAN_message;

//BL_OBJ_TYPE_CAN_MESSAGE2: 86
typedef struct CAN_message2_t
{
  obj_header      header;
  uint16_t        channel;
  uint8_t         flags;
  uint8_t         DLC;
  uint32_t        ID;
  uint8_t         data[8];
  uint32_t        frame_length;
  uint8_t         bit_count;
  uint8_t         reserved1;
  uint16_t        reserved2;
} CAN_message2;


//BL_OBJ_TYPE_CAN_ERROR: 2
typedef struct CAN_error_frame_t
{
  obj_header      header;
  uint16_t        channel;
  uint16_t        length;
} CAN_error_frame;

// BL_OBJ_TYPE_CAN_ERROR_EXT: 73
typedef struct CAN_error_frame_ext_t
{
  obj_header      header;
  uint16_t        channel;
  uint16_t        length;
  uint32_t        flags;
  uint8_t         ECC;
  uint8_t         position;
  uint8_t         DLC;
  uint8_t         reserved1;
  uint32_t        frame_length_in_ns;
  uint32_t        ID;
  uint16_t        flags_ext;
  uint16_t        reserved2;
  uint8_t         data[8];
} CAN_error_frame_ext;

//BL_OBJ_TYPE_CAN_OVERLOAD: 3
typedef struct CAN_overload_frame_t
{
  obj_header      header;
  uint16_t        channel;
  uint16_t        dummy;
} CAN_overload_frame;

//BL_OBJ_TYPE_APP_TRIGGER: 5
typedef struct app_trigger_t
{
  obj_header      header;
  uint64_t        pre_trigger_time;
  uint64_t        post_trigger_time;
  uint16_t        channel;
  uint16_t        flags;
  uint32_t        app_specific2;
} app_trigger;


/* BL_OBJ_TYPE_LOG_CONTAINER: 10
    reader:
        read object header and object size, then
        1. (unzipped) read objects from container data
        2. (zip) decompress then do the above
*/

//BL_OBJ_TYPE_CAN_FD_MESSAGE_64: 101
typedef struct CANFD_message64_t
{
  obj_header      header;
  uint8_t         channel;
  uint8_t         DLC;
  uint8_t         valid_data_bytes;
  uint8_t         tx_count;
  uint32_t        ID;
  uint32_t        frame_length;
  uint32_t        flags;
  uint32_t        btr_cfg_arb;
  uint32_t        btr_cfg_data;
  uint32_t        time_offset_brs_ns;
  uint32_t        time_offset_crc_del_ns;
  uint16_t        bit_count;
  uint8_t         dir;
  uint8_t         ext_data_offset;
  uint32_t        CRC;
  uint8_t         data[64];
  uint32_t        BTR_ext_arb;
  uint32_t        BTR_ext_data;
} CANFD_message64;

//BL_OBJ_TYPE_CAN_FD_ERROR_64: 104
typedef struct CANFD_error_frame64_t
{
  obj_header      header;
  uint8_t         channel;
  uint8_t         DLC;
  uint8_t         valid_data_bytes;
  uint8_t         ECC;
  uint16_t        flags;
  uint16_t        error_code_ext;
  uint16_t        ext_flags;
  uint8_t         ext_data_offset;
  uint8_t         reserved1;
  uint32_t        ID;
  uint32_t        frame_length;
  uint32_t        btr_cfg_arb;
  uint32_t        btr_cfg_data;
  uint32_t        time_offset_brs_ns;
  uint32_t        time_offset_crc_del_ns;
  uint32_t        CRC;
  uint16_t        error_position;
  uint16_t        reserved2;
  uint8_t         data[64];
  uint32_t        BTR_ext_arb;
  uint32_t        BTR_ext_data;
} CANFD_error_frame64;

#endif /* VECTOR_BFL_FD_H_ */
