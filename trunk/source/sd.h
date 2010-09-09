#ifndef SD_H
#define SD_H

#include <vector>

/******************************************************************************
Filesystem organization
*******************************************************************************

The block 0 is laid out as follows:

	The first 14 bytes contain the root identifier, parameters, and the number of log entries
	+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
	|   R    |   O    |   O    |   T    |   v0   |   v1   |   v2   |   v3   | res(0) | res(0) | res(0) | res(0) | res(0) | numlog | ...
	+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
	Successive bytes contain the log entries (16 byte per log entry):
			-----+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
			 ... |  DID0  |  DID1  |  DID2  |  DID3  |  TIM0  |  TIM1  |  TIM2  |  TIM3  | START0 | START1 | START2 | START3 |  END0  |  END0  |  END0  |  END0  | ...
			-----+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----
	The last two bytes contain a 16-bit simple checksum on first 510 bytes:
																		-----+--------+--------+
																		 ... | XSUM0  | XSUM1  |
																		-----+--------+--------+

******************************************************************************/




typedef struct {
	unsigned long int device;
	unsigned long int starttime;
	unsigned long int blockstart;
	unsigned long int blockend;
} fs_logentry;

typedef struct {
	bool ok;

	unsigned char root[4];
	unsigned char version[4];
	unsigned char reserved[5];
	unsigned char xsum[2];
	std::vector<fs_logentry> logentry;
} sd_filesystem;

void fs_checksum(unsigned char *buffer,unsigned short n,unsigned char *x1,unsigned char *x2);
int fs_parseroot(unsigned char *data,sd_filesystem &filesystem);

#endif // SD_H
