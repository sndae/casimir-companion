/*
	DIGIREC
	Copyright (C) 2010:
			Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PKT_H
#define PKT_H

#include <QTextStream>
#include <QByteArray>
#include <vector>

/******************************************************************************
Format of log files on hard disk
*******************************************************************************

	The raw binary dump of a log on hard disk is a hybrid container/data format.
	It replicates some of the informations contained in the root and log entry.

	It contains:
	- Copy of the 13 bytes root header, with 'ROOT' replaced by 'LOG\0'
	- Copy of the 16 bytes log entry
	- Data (packets)

The file is laid out as follows

	The first 13 bytes contain the log identifier, version and reserved data.
	+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
	|   L    |   O    |   G    |  \0    |   v0   |   v1   |   v2   |   v3   | res(0) | res(0) | res(0) | res(0) | res(0) | ...
	+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
	Successive bytes contain the log entries (16 byte per log entry):
	-----+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
	 ... |  DID0  |  DID1  |  DID2  |  DID3  |  TIM0  |  TIM1  |  TIM2  |  TIM3  | START0 | START1 | START2 | START3 |  END0  |  END0  |  END0  |  END0  | ...
	-----+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
	Successive bytes contain the packet, see packet data structure below.


******************************************************************************/

/******************************************************************************
*******************************************************************************
	Packet encapsulation
*******************************************************************************
******************************************************************************/

/*
	Packet data structure
	+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------------
	|   55   |   AA   |   00   |   FF   |  type  |  time  |  time  |  time  |  time  |   id   |   id   |   id   |   id   | entry# |   ns   |   dt   |   dt   |   opt  |   00   |   00   |  ... data ...
	+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------------

	-------------+--------+--------+
	... Data ... |  XSUM  |  XSUM  |
	-------------+--------+--------+


	type: 		type of packet
	time: 		time of the first sample of the packet
	dt:   		delta time between samples
	ns:			number of samples in packet (meaning specific to packet type)
	id:			device id
	entry#:		log entry
	opt:			optional (reserved, 0)

*/

// Size of the preamble in the data packet
#define PACKET_PREAMBLE				20			// This MUST be a multiple of 2

// Type of packets
#define PACKET_TYPE_AUDIO				0
#define PACKET_TYPE_ACC					1
#define PACKET_TYPE_SYSTEM				2
#define PACKET_TYPE_LIGHT				3
#define PACKET_TYPE_TMP					4
#define PACKET_TYPE_HMC					5

typedef struct {
	//unsigned char preamble[4];
	unsigned int preamble;
	unsigned char type;
	unsigned int time;
	unsigned int id;
	unsigned char entry;
	unsigned char ns;
	unsigned short dt;
	unsigned char opt;
	unsigned char zero[2];
} PREAMBLE;


//template <typename DATATYPE>
struct STREAM_GENERIC
{
	//std::vector<bool> nan;
	std::vector<unsigned> timetick;
	std::vector<double> time;
	//std::vector<DATATYPE> data;
	std::vector<std::vector<int > > data;
	std::vector<std::vector<bool > > nans;
	unsigned dt;
	unsigned time_offset;
};


void StreamDump(STREAM_GENERIC &p,QTextStream &out,bool nan=true);
void StreamClear(STREAM_GENERIC &p,unsigned time_offset);
void StreamAddSample(STREAM_GENERIC &p,std::vector<int> data,unsigned t,unsigned dt);
void StreamAddPacketV1(STREAM_GENERIC &stream,PREAMBLE &pre,const char *packet);
void StreamAddPacketV6(STREAM_GENERIC &stream,PREAMBLE &pre,const char *packet);
void data2preamble(const char *data,PREAMBLE &pre);
void printpreamble(const PREAMBLE &pre);
unsigned packetbytesize(const PREAMBLE &pre);
void demux(QByteArray &data,unsigned time_offset,STREAM_GENERIC &stream_audio,STREAM_GENERIC &stream_acc,STREAM_GENERIC &stream_hmc,STREAM_GENERIC &stream_sys,STREAM_GENERIC &stream_light,STREAM_GENERIC &stream_tmp);
bool StreamSave(STREAM_GENERIC &stream,QString filename);
void StreamMerge(std::vector<STREAM_GENERIC *> streams,STREAM_GENERIC &streamout);

#endif // PKT_H
