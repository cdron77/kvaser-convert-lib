/*
**             Copyright 2023 by Kvaser AB, Molndal, Sweden
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

#ifndef __PCIEFD_PACKET_H__
#define __PCIEFD_PACKET_H__

#include "VCanOsIf.h"
#include "pciefd_packet_defs.h"

// Packet handling
#define USE_CAN_FD (1)

#define KCAN_NR_OF_HDR_WORDS 2
#define KCAN_NR_OF_RX_HDR_WORDS 4

#if USE_CAN_FD
#define NR_OF_DATA_WORDS 16
#else
#define NR_OF_DATA_WORDS 2
#endif

static inline unsigned int dlcToBytes(unsigned int dlc)
{
    dlc &= 0xf;
    return dlc > 8 ? 8 : dlc;
}

int dlcToBytesFD(int dlc);
int bytesToDlc(int bytes);

static inline unsigned int bytesToWordsCeil(unsigned int bytes)
{
    return (bytes + 3) / 4;
}

/** KCAN packet header. */
struct __attribute__ ((packed, aligned (4))) kcan_packet_hdr {
    uint32_t id;
    uint32_t control;
};

/** Receive-, Acknowledge data- or Receive status packet. */
struct __attribute__ ((packed, aligned (4))) kcan_rx_packet {
    struct kcan_packet_hdr hdr;
    uint64_t timestamp;
    char data[NR_OF_DATA_WORDS];
};

static inline int packetChannelId(struct kcan_packet_hdr *packet)
{
    return RPACKET_CHID_GET(packet->control);
}

// +----------------------------------------------------------------------------
// | Status packet
// | * Can also be used for error packets as packet->id has the same fields
// +----------------------------------------------------------------------------
static inline int statusInResetMode(struct kcan_packet_hdr *packet)
{
    return SPACKET_IRM_GET(packet->id);
}

static inline int statusResetModeChangeDetected(struct kcan_packet_hdr *packet)
{
    return SPACKET_RMCD_GET(packet->id);
}

static inline int statusReset(struct kcan_packet_hdr *packet)
{
    return SPACKET_RESET_GET(packet->id);
}

static inline int statusOverflow(struct kcan_packet_hdr *packet)
{
    return SPACKET_ROVF_GET(packet->control);
}

static inline int statusErrorWarning(struct kcan_packet_hdr *packet)
{
    return SPACKET_EWLR_GET(packet->control);
}

static inline int statusErrorPassive(struct kcan_packet_hdr *packet)
{
    return SPACKET_EPLR_GET(packet->control);
}

static inline int statusBusOff(struct kcan_packet_hdr *packet)
{
    return SPACKET_BOFF_GET(packet->id);
}

static inline int statusReceiveErrorCount(struct kcan_packet_hdr *packet)
{
    return SPACKET_RXERR_GET(packet->id);
}

static inline int statusTransmitErrorCount(struct kcan_packet_hdr *packet)
{
    return SPACKET_TXERR_GET(packet->id);
}

static inline unsigned int statusCmdSeqNo(struct kcan_packet_hdr *packet)
{
    return SPACKET_CMD_SEQ_GET(packet->control);
}

// +----------------------------------------------------------------------------
// Received data packets
// +----------------------------------------------------------------------------
static inline int getId(struct kcan_packet_hdr *packet)
{
    return RTPACKET_ID_GET(packet->id);
}

void setId(struct kcan_packet_hdr *packet, int id);

static inline int isErrorPassive(struct kcan_packet_hdr *packet)
{
    return RPACKET_EPLR_GET(packet->control);
}

static inline int receiverOverflow(struct kcan_packet_hdr *packet)
{
    return RPACKET_ROVF_GET(packet->control);
}

static inline int errorWarning(struct kcan_packet_hdr *packet)
{
    return RPACKET_EWLR_GET(packet->control);
}

static inline int isFlexibleDataRateFormat(struct kcan_packet_hdr *packet)
{
    return RTPACKET_FDF_GET(packet->control);
}

static inline int isAlternateBitRate(struct kcan_packet_hdr *packet)
{
    return RTPACKET_BRS_GET(packet->control);
}

static inline int errorStateIndicated(struct kcan_packet_hdr *packet)
{
    return RTPACKET_ESI_GET(packet->control);
}

static inline int getSRR(struct kcan_packet_hdr *packet)
{
    return RTPACKET_SRR_GET(packet->id);
}

static inline int isExtendedId(struct kcan_packet_hdr *packet)
{
    return RTPACKET_IDE_GET(packet->id);
}

static inline int isRemoteRequest(struct kcan_packet_hdr *packet)
{
    return RTPACKET_RTR_GET(packet->id);
}

static inline unsigned int getDLC(struct kcan_packet_hdr *packet)
{
    return RTPACKET_DLC_GET(packet->control);
}

void setDLC(struct kcan_packet_hdr *packet, int dlc);

static inline unsigned int getSeqNo(struct kcan_packet_hdr *packet)
{
    return RTPACKET_SEQ_GET(packet->control);
}

int getOffsetQuantas(struct kcan_packet_hdr *packet);

static inline int getOffsetNbits(struct kcan_packet_hdr *packet)
{
    return OPACKET_NBITS_GET(packet->id);
}

static inline int getBusLoad(struct kcan_packet_hdr *packet)
{
    return BPACKET_BUS_LOAD_GET(packet->id);
}

// +----------------------------------------------------------------------------
// | Received ack packets
// +----------------------------------------------------------------------------
static inline unsigned int getAckSeqNo(struct kcan_packet_hdr *packet)
{
    return APACKET_SEQ_GET(packet->id);
}

static inline int isFlushed(struct kcan_packet_hdr *packet)
{
    return APACKET_FLUSHED_GET(packet->id);
}

static inline int isNack(struct kcan_packet_hdr *packet)
{
    return APACKET_NACK_GET(packet->id);
}

static inline int isABL(struct kcan_packet_hdr *packet)
{
    return APACKET_ABL_GET(packet->id);
}

static inline int isControlAck(struct kcan_packet_hdr *packet)
{
    return APACKET_CONTROL_ACK_GET(packet->id);
}

// +----------------------------------------------------------------------------
// | Received txrq packets
// +----------------------------------------------------------------------------
static inline unsigned int getTxrqSeqNo(struct kcan_packet_hdr *packet)
{
    return APACKET_SEQ_GET(packet->id);
}

// +----------------------------------------------------------------------------
// | Received error packets
// +----------------------------------------------------------------------------
static inline int getTransmitErrorCount(struct kcan_packet_hdr *packet)
{
    return EPACKET_TXE_GET(packet->id);
}

static inline int getReceiveErrorCount(struct kcan_packet_hdr *packet)
{
    return EPACKET_RXE_GET(packet->id);
}

int getErrorField(struct kcan_packet_hdr *packet);
int getErrorFieldPos(struct kcan_packet_hdr *packet);
void printErrorCode(struct kcan_packet_hdr *packet);

// +----------------------------------------------------------------------------
// | Transmit packets
// +----------------------------------------------------------------------------
void setRemoteRequest(struct kcan_packet_hdr *packet);
void setExtendedId(struct kcan_packet_hdr *packet);
void setBitRateSwitch(struct kcan_packet_hdr *packet);
void setAckRequest(struct kcan_packet_hdr *packet);
void setTxRequest(struct kcan_packet_hdr *packet);
void setSingleShotMode(struct kcan_packet_hdr *packet);

int setupBaseFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno);
int setupExtendedFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno);
int setupFDBaseFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno);
int setupFDExtendedFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno);

#endif
