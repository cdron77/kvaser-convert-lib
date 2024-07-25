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

// Packet handling
#include "pciefd_packet.h"

int dlcToBytesFD(int dlc)
{
    dlc &= 0xf;

    switch (dlc) {
    case 9:
        return 12;
    case 10:
        return 16;
    case 11:
        return 20;
    case 12:
        return 24;
    case 13:
        return 32;
    case 14:
        return 48;
    case 15:
        return 64;
    default:
        return dlc;
    }
}

int bytesToDlc(int bytes)
{
    if (bytes > 64) {
        printk("<1>Invalid number of bytes, %u.", bytes);
    }

    if (bytes > 48)
        return DLC64;
    else if (bytes > 32)
        return DLC48;
    else if (bytes > 24)
        return DLC32;
    else if (bytes > 20)
        return DLC24;
    else if (bytes > 16)
        return DLC20;
    else if (bytes > 12)
        return DLC16;
    else if (bytes > 8)
        return DLC12;
    else
        return bytes;
}

// +----------------------------------------------------------------------------
// Received data packets
// +----------------------------------------------------------------------------
void setId(struct kcan_packet_hdr *packet, int id)
{
    id <<= RTPACKET_ID_LSHIFT;
    id &= RTPACKET_ID_MSK;

    packet->id &= ~RTPACKET_ID_MSK;
    packet->id |= id;
}

void setDLC(struct kcan_packet_hdr *packet, int dlc)
{
    packet->control &= ~RTPACKET_DLC_MSK;
    packet->control |= RTPACKET_DLC(dlc);
}

// +----------------------------------------------------------------------------
// Received offset packets
// +----------------------------------------------------------------------------
static int signext(int value, int signpos)
{
    if (value & (1 << signpos)) {
        return value | ~((1UL << (signpos + 1)) - 1);
    }

    return value;
}

int getOffsetQuantas(struct kcan_packet_hdr *packet)
{
    int quantas = OPACKET_OFFSET_GET(packet->id); // 13-bits

    return signext(quantas, 12);
}

// +----------------------------------------------------------------------------
// | Received error packets
// +----------------------------------------------------------------------------
static etype_t getErrorType(struct kcan_packet_hdr *packet)
{
    return (etype_t)EPACKET_TYPE_GET(packet->control);
}

static eseg_t getErrorSegment(struct kcan_packet_hdr *packet)
{
    return (eseg_t)EPACKET_SEG_GET(packet->control);
}

static eseg2_t getErrorSegment2(struct kcan_packet_hdr *packet)
{
    return (eseg2_t)EPACKET_SEG2_GET(packet->control);
}

static int isTransmitError(struct kcan_packet_hdr *packet)
{
    return EPACKET_DIR_GET(packet->control);
}

int getErrorField(struct kcan_packet_hdr *packet)
{
    if (getErrorSegment(packet) == ESEG_OTHER) {
        return getErrorSegment2(packet) + ESEG_OTHER;
    }

    return getErrorSegment(packet);
}

int getErrorFieldPos(struct kcan_packet_hdr *packet)
{
    if (getErrorSegment(packet) == ESEG_OTHER) {
        return 1; // No field pos available, always report bit pos 1
    }

    return (int)getErrorSegment2(packet);
}

static const char *etype_str[] = {
  "No", "Bit", "Bit (ssp)", "Stuff (arbitration)", "Stuff", "Form", "Ack", "Crc"
};

static const char *epos_str[] = {
  "eid",
  "crc",
  "bid",
  "dlc",
  "eof",
  "overload flag",
  "active error flag",
  "passive error flag",
  "error delim",
  "intermission",
  "sof",
  "srr/rtr",
  "ide",
  "rtr",
  "r0",
  "fd r0",
  "r1",
  "brs",
  "esi",
  "data",
  "crc delim",
  "ack slot",
  "ack delim",
  "BUG"
};

void printErrorCode(struct kcan_packet_hdr *packet)
{
    unsigned int len = 0;
    char buf[512];

    if (isTransmitError(packet)) {
        len += sprintf(buf + len, "#                                        Transmit ");
    } else {
        len += sprintf(buf + len, "#                                        Receive ");
    }

    len += sprintf(buf + len, "%s Error ", etype_str[getErrorType(packet)]);
    len += sprintf(buf + len, "in field %s ", epos_str[getErrorField(packet)]);

    if (getErrorSegment(packet) != ESEG_OTHER) {
        len += sprintf(buf + len, "pos %u ", getErrorFieldPos(packet));
    }

    len += sprintf(buf + len, "\n");

#ifdef PCIEFD_DEBUG
    printk("<1>%s", buf);
#endif
}

// +----------------------------------------------------------------------------
// | Transmit packets
// +----------------------------------------------------------------------------
void setRemoteRequest(struct kcan_packet_hdr *packet)
{
    packet->id |= RTPACKET_RTR(1);
}

void setExtendedId(struct kcan_packet_hdr *packet)
{
    packet->id |= RTPACKET_IDE(1) | RTPACKET_SRR(1);
}

void setBitRateSwitch(struct kcan_packet_hdr *packet)
{
    packet->control |= RTPACKET_BRS(1);
}

void setAckRequest(struct kcan_packet_hdr *packet)
{
    packet->control |= TPACKET_AREQ(1);
}

void setTxRequest(struct kcan_packet_hdr *packet)
{
    packet->control |= TPACKET_TREQ(1);
}

void setSingleShotMode(struct kcan_packet_hdr *packet)
{
    packet->control |= TPACKET_TSSM(1);
}

int setupBaseFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno)
{
    packet->id = RTPACKET_RTR(0) | RTPACKET_IDE(0) | RTPACKET_SRR(0) | RTPACKET_ID(id);
    packet->control = RTPACKET_DLC(dlc) | RTPACKET_BRS(0) | RTPACKET_FDF(0) | RTPACKET_SEQ(seqno);

    return dlcToBytes(dlc);
}

int setupExtendedFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno)
{
    packet->id = RTPACKET_RTR(0) | RTPACKET_IDE(1) | RTPACKET_SRR(1) | RTPACKET_ID(id);
    packet->control = RTPACKET_DLC(dlc) | RTPACKET_BRS(0) | RTPACKET_FDF(0) | RTPACKET_SEQ(seqno);

    return dlcToBytes(dlc);
}

int setupFDBaseFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno)
{
    packet->id = RTPACKET_RTR(0) | RTPACKET_IDE(0) | RTPACKET_SRR(0) | RTPACKET_ID(id);
    packet->control = RTPACKET_DLC(dlc) | RTPACKET_BRS(0) | RTPACKET_FDF(1) | RTPACKET_SEQ(seqno);

    return dlcToBytesFD(dlc);
}

int setupFDExtendedFormat(struct kcan_packet_hdr *packet, int id, int dlc, int seqno)
{
    packet->id = RTPACKET_RTR(0) | RTPACKET_IDE(1) | RTPACKET_SRR(1) | RTPACKET_ID(id);
    packet->control = RTPACKET_DLC(dlc) | RTPACKET_BRS(0) | RTPACKET_FDF(1) | RTPACKET_SEQ(seqno);

    return dlcToBytesFD(dlc);
}
