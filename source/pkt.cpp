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

#include <QFile>
#include "pkt.h"


//template <typename DATATYPE>
//void PacketClear(STREAM_GENERIC<DATATYPE> &p)
void StreamClear(STREAM_GENERIC &p,unsigned time_offset)
{
//	p.nan.clear();
	p.timetick.clear();
	p.time.clear();
	p.data.clear();
	p.nans.clear();
	p.time_offset=time_offset;
}
//template <typename DATATYPE>
//void PacketAddSample(STREAM_GENERIC<DATATYPE> &p,DATATYPE data,unsigned t,unsigned dt)
void StreamAddSample(STREAM_GENERIC &p,std::vector<int> data,unsigned t,unsigned dt)
{
	// First check whether there are missing samples, and inserts NANs
	// Check continuity between last sample (if any) and current sample
	// Assumptions: packets are stored in increasing time order. Packets can be missing, but are not temporally shuffled.
	if(p.data.size())
	{
		// At least one previous sample - check continuity
		if(p.timetick.back()+dt<t)
		{
			// Discontinuity... compute number of nan samples to add
			unsigned add = (t-dt-p.timetick.back())/dt;
			printf("Missing samples! Inserting %d NANs\n",add);
			for(unsigned mi=0;mi<add;mi++)
			{
//				p.nan.push_back(true);
				p.nans.push_back(std::vector<bool>(data.size(),true));
				p.timetick.push_back(p.timetick.back()+dt);
				p.time.push_back(0);
				p.data.push_back(p.data.back());				// Repeat the data (when saving to file, the user can dump NANs instead)
			}
		}
	}
	// Dummy thing
//	p.nan.push_back(false);
	p.nans.push_back(std::vector<bool>(data.size(),false));
	p.timetick.push_back(t);
	p.time.push_back(0);
	p.data.push_back(data);
	p.dt = dt;
}
//template <typename DATATYPE>
void StreamDump(STREAM_GENERIC &p,QTextStream &out,bool nan)
{
	for(unsigned i=0;i<p.data.size();i++)
	{
		QString str,str2;
		//str.sprintf("%c %010u  ",p.nan[i]?'X':'V',p.timetick[i]);
		str.sprintf("%010u  ",p.timetick[i]);
		for(unsigned j=0;j<p.data[i].size();j++)
		{
			if(nan && p.nans[i][j])
				str2.sprintf("       NaN ");
			else
				str2.sprintf("% 10d ",p.data[i][j]);
			str+=str2;
		}
		str+="\n";
		//printf("%s",str.toStdString().c_str());
		out << str;
	}
}

void StreamAddPacketV1(STREAM_GENERIC &stream,PREAMBLE &pre,const char *packet)
{
	for(unsigned s=0;s<pre.ns;s++)
	{
		int sample = *(const unsigned short*)(packet+PACKET_PREAMBLE+2*s);
		StreamAddSample(stream,std::vector<int>(1,sample),pre.time+s*pre.dt,pre.dt);
	}
}
void StreamAddPacketV6(STREAM_GENERIC &stream,PREAMBLE &pre,const char *packet)
{
	for(unsigned s=0;s<pre.ns;s++)
	{
		std::vector<int> v(3,0);
		for(unsigned i=0;i<3;i++)
		{
			v[i] = *(const signed short*)(packet+PACKET_PREAMBLE+(s*3+i)*2);
		}
		StreamAddSample(stream,v,pre.time+s*pre.dt,pre.dt);
	}
}

// Assumes there is enough data at the pointer location to read the preamble
void data2preamble(const char *data,PREAMBLE &pre)
{
	//for(unsigned i=0;i<4;i++)
		//pre.preamble[i]=data[i];
	pre.preamble=*(unsigned int*)data;
	pre.type=data[4];
	pre.time=*(unsigned int*)(data+5);
	pre.id=*(unsigned int*)(data+9);
	pre.entry=data[13];
	pre.ns=data[14];
	pre.dt=*(unsigned short*)(data+15);
	pre.opt=data[17];
	pre.zero[0]=data[18];
	pre.zero[1]=data[19];
}
void printpreamble(const PREAMBLE &pre)
{
	printf("Preamble: %08X\n",pre.preamble);
	printf("Type:  %d ",pre.type);
	switch(pre.type){	case PACKET_TYPE_AUDIO: printf("(audio)\n"); break;
							case PACKET_TYPE_ACC: printf("(acc)\n"); break;
							case PACKET_TYPE_SYSTEM: printf("(sys)\n"); break;
							case PACKET_TYPE_LIGHT: printf("(lit)\n"); break;
							case PACKET_TYPE_TMP: printf("(tmp)\n"); break;
							case PACKET_TYPE_HMC: printf("(hmc)\n"); break;
							default: printf("unk\n");
							}
	printf("Time:  %u\n",pre.time);
	printf("ID:    %08X\n",pre.id);
	printf("#logs: %u\n",(unsigned)pre.entry);
	printf("ns:    %d\n",(unsigned)pre.ns);
	printf("dt:    %08X\n",(unsigned)pre.dt);
	printf("opt:   %02X\n",(unsigned)pre.opt);
	printf("zero:  %02X %02X\n",(unsigned)pre.zero[0],(unsigned)pre.zero[1]);
}
// Returns the numbers of bytes required
unsigned packetbytesize(const PREAMBLE &pre)
{
	switch(pre.type)
	{
	case PACKET_TYPE_AUDIO:
		return 2+PACKET_PREAMBLE+pre.ns*2;
	case PACKET_TYPE_ACC:
		return 2+PACKET_PREAMBLE+pre.ns*2*3;
	case PACKET_TYPE_SYSTEM:
		return 2+PACKET_PREAMBLE+pre.ns*2;
	case PACKET_TYPE_LIGHT:
		return 2+PACKET_PREAMBLE+pre.ns*2;
	case PACKET_TYPE_TMP:
		return 2+PACKET_PREAMBLE+pre.ns*2;
	case PACKET_TYPE_HMC:
		return 2+PACKET_PREAMBLE+pre.ns*2*3;
	}
	return 0;
}


void demux(QByteArray &data,unsigned time_offset,STREAM_GENERIC &stream_audio,STREAM_GENERIC &stream_acc,STREAM_GENERIC &stream_hmc,STREAM_GENERIC &stream_sys,STREAM_GENERIC &stream_light,STREAM_GENERIC &stream_tmp)
{
	//STREAM_GENERIC<unsigned short> packet_audio;
	StreamClear(stream_audio,time_offset);
	StreamClear(stream_acc,time_offset);
	StreamClear(stream_hmc,time_offset);
	StreamClear(stream_sys,time_offset);
	StreamClear(stream_light,time_offset);
	StreamClear(stream_tmp,time_offset);

	printf("Demultiplexing %d bytes\n",data.size());

	unsigned ptr=0;
	unsigned decodepackets=0;

	printf("Sizeof preamble: %d. packet_preamble: %d\n",sizeof(PREAMBLE),PACKET_PREAMBLE);



	//for(unsigned _i=0;_i<360;_i++)
	while(ptr<data.size())
	{
		// Check if enough data to contain a packet preamble
		if(ptr+PACKET_PREAMBLE>=data.size())
		{
			printf("some bytes left, but not enough for a packet preamble. ptr: %u. size: %u\n",ptr,data.size());
			break;
		}
		PREAMBLE pre;
		data2preamble(data.constData()+ptr,pre);

		//memcpy(&pre,data.constData()+ptr,)


		if(pre.preamble==0xFF00AA55)
		{	
			// This could be a packet...
			unsigned size = packetbytesize(pre);

			// Check that we have enough data to decode this packet
			if(ptr+size>=data.size())
			{
				printf("Packet preamble detected, but not enough data left for the complete packet\n");
				break;
			}
			//printf("Found a packet at %d - %d. Type %d\n",ptr,ptr+size-1,pre.type);
			//printpreamble(pre);

			//if(!((decodepackets%7)==0 || (decodepackets%9)==0))	// Simulation of packet loss
			if(1)
			{
				// Now decode the packet
				switch(pre.type)
				{
				case PACKET_TYPE_AUDIO:
					StreamAddPacketV1(stream_audio,pre,data.constData()+ptr);
					break;
				case PACKET_TYPE_ACC:
					StreamAddPacketV6(stream_acc,pre,data.constData()+ptr);
					break;
				case PACKET_TYPE_SYSTEM:
					StreamAddPacketV1(stream_sys,pre,data.constData()+ptr);
					break;
				case PACKET_TYPE_LIGHT:
					StreamAddPacketV1(stream_light,pre,data.constData()+ptr);
					break;
				case PACKET_TYPE_TMP:
					StreamAddPacketV1(stream_tmp,pre,data.constData()+ptr);
					break;
				case PACKET_TYPE_HMC:
					StreamAddPacketV6(stream_hmc,pre,data.constData()+ptr);
					break;
				default:
					printf("Unknown packet type, skipping\n");
				}
			} // Simulation of packet loss
			// Skip the packet
			ptr += size;
			decodepackets++;
		}
		else
		{
			// Skip one byte
			ptr++;
		}
	}
	printf("Demux terminated. ptr: %u. size: %u. Decoded %d packets.\n",ptr,data.size(),decodepackets);
	// Dump the data...
	//printf("Audio\n");
	//StreamDump(stream_audio);
	//printf("Acceleration\n");
	//StreamDump(stream_acc);
	//printf("HMC\n");
	//StreamDump(stream_hmc);
	//printf("System\n");
	//StreamDump(stream_sys);
	//printf("Light\n");
	//StreamDump(stream_light);
	//printf("TMP\n");
	//StreamDump(stream_tmp,out);
}
bool StreamSave(STREAM_GENERIC &stream,QString filename)
{
	QFile file(filename);
	file.open(QIODevice::WriteOnly|QIODevice::Truncate| QIODevice::Text);
	QTextStream out(&file);
	StreamDump(stream,out);
	file.close();
}

/******************************************************************************
	Merge multiple streams
*******************************************************************************
	In order to merge the streams the function must:
		1. Identify which is the smallest time delta among the streams
		2. Identify the stream starting the earliest
		3. Identify the stream finishing the latest

******************************************************************************/
void StreamMerge(std::vector<STREAM_GENERIC *> streams,STREAM_GENERIC &streamout)
{
	unsigned dt;									// dt of the merged stream
	unsigned start,stop;							// Start/stop time of the merged stream
	std::vector<unsigned> dts;					// dt of each stream
	std::vector<unsigned> starts;				// Start time of each stream
	std::vector<unsigned> stops;				// Stop time of each stream
	std::vector<unsigned> numchans;			// Number of channels of each stream
	std::vector<unsigned> channeloffset;

	// Dump some quick infos
	for(unsigned i=0;i<streams.size();i++)
	{
		printf("Stream %d. dt: %u. start: %u. end: %u. Num samples: %u\n",i,streams[i]->dt,streams[i]->timetick.front(),streams[i]->timetick.back(),streams[i]->data.size());
	}

	// Find the smallest delta among the streams.
	dts.resize(streams.size(),0);
	for(unsigned i=0;i<streams.size();i++)
		dts[i] = streams[i]->dt;
	for(unsigned i=0;i<streams.size();i++)
	{
		if(i==0)
			dt = streams[i]->dt;
		else
		{
			if(streams[i]->dt<dt)
				dt = streams[i]->dt;
		}
	}
	printf("Smallest dt: %d\n",dt);

	// Find the stream starting the earliest
	starts.resize(streams.size(),0);
	for(unsigned i=0;i<streams.size();i++)
		starts[i] = streams[i]->timetick.front();
	for(unsigned i=0;i<streams.size();i++)
	{
		if(i==0)
			start = streams[i]->timetick.front();
		else
		{
			if(streams[i]->timetick.front()<start)
				start = streams[i]->timetick.front();
		}
	}
	printf("Start timetick: %d\n",start);

	// Find the stream finishing the latest
	stops.resize(streams.size(),0);
	for(unsigned i=0;i<streams.size();i++)
		stops[i] = streams[i]->timetick.back();
	for(unsigned i=0;i<streams.size();i++)
	{
		if(i==0)
			stop = streams[i]->timetick.back();
		else
		{
			if(streams[i]->timetick.back()>stop)
				stop = streams[i]->timetick.back();
		}
	}
	printf("Stop timetick: %d\n",stop);

	// Count the number of channels
	numchans.resize(streams.size(),0);
	for(unsigned i=0;i<streams.size();i++)
		numchans[i] = streams[i]->data.front().size();
	unsigned channels=0;
	for(unsigned i=0;i<streams.size();i++)
		channels += streams[i]->data.front().size();

	printf("Total number of channels: %u\n",channels);

	// Number of samples
	unsigned samples = (stop-start+dt)/dt;
	printf("Number of samples to generate: %d\n",samples);

	// Generate the merged data structure
	StreamClear(streamout,0);
	streamout.nans.resize(samples,std::vector<bool>(channels,false));
	streamout.time.resize(samples,0.0);
	streamout.timetick.resize(samples,0);
	streamout.data.resize(samples,std::vector<int>(channels,0));
	streamout.dt=dt;

	// Comput the channel offsets (positions) in the new structure
	channeloffset.resize(streams.size(),0);
	for(unsigned i=1;i<streams.size();i++)
		channeloffset[i]=channeloffset[i-1]+streams[i-1]->data.front().size();
	printf("Position of the data in the new structure: ");
	for(unsigned i=0;i<streams.size();i++)
		printf("%u ",channeloffset[i]);
	printf("\n");


	printf("New data: samples: %u channels: %u\n",streamout.data.size(),streamout.data.front().size());

	// Copy the data in the new structure...
	for(unsigned ti=0;ti<samples;ti++)
	//for(unsigned ti=0;ti<=10;ti++)
	{
		unsigned t = ti*dt+start;

		streamout.timetick[ti]=t;

		for(unsigned s=0;s<streams.size();s++)
		{
			// Need to find the index in stream s of the sample at time t (if any)

			if(t<starts[s] || t>stops[s])
			{
				// The time is outside of what this channel provides. We set nans in the channel
				for(unsigned c=0;c<numchans[s];c++)
				{
					streamout.nans[ti][c+channeloffset[s]]=true;
					//streamout.data[t][c+channeloffset[s]]=0;
				}
			}
			else
			{
				unsigned i=(t-starts[s])/dts[s];
				//printf("time %u maps to time %u on stream %u\n",t,streams[s]->timetick[i],s);
				for(unsigned c=0;c<numchans[s];c++)
				{
					streamout.nans[ti][c+channeloffset[s]]=streams[s]->nans[i][c];
					streamout.data[ti][c+channeloffset[s]]=streams[s]->data[i][c];
				}
			}
		}
	}
	printf("Merge completed\n");

}

/*
	//QFile file;
	QTextStream out;

	QFile file("dat_audio.dat");
	file.open(QIODevice::WriteOnly|QIODevice::Truncate| QIODevice::Text);
	out = QTextStream(&file);
	StreamDump(stream_audio,out);
	file.close();



}

*/
