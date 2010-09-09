#include <stdio.h>
#include "sd.h"

void fs_checksum(unsigned char *buffer,unsigned short n,unsigned char *x1,unsigned char *x2)
{
	unsigned char xsum1,xsum2;
	xsum1=0;
	xsum2=0;
	for(unsigned short i=0;i<n;i++)
	{
		xsum1^=buffer[i];
		xsum2+=buffer[i];
	}
	*x1=xsum1;
	*x2=xsum2;
}


int fs_parseroot(unsigned char *buffer,sd_filesystem &filesystem)
{
	unsigned char rv;
	//unsigned char xsum1,xsum2;
	unsigned short numentry;
	unsigned long int nextsector;
	fs_logentry *logentry;

	filesystem.ok=false;

	for(unsigned i=0;i<4;i++)
		filesystem.root[i]=buffer[i];
	if(buffer[0]!='R' || buffer[1]!='O' || buffer[2]!='O' || buffer[3]!='T')
	{
		printf("Root sector error: start marker\n");
		return -2;
	}
	rv=0;
	for(unsigned char i=4;i<13;i++)
		if(buffer[i]!=0)
			rv=1;
	if(rv)
	{
		printf("Root sector error: flags\n");
		return -2;
	}


	fs_checksum(buffer,510,&filesystem.xsum[0],&filesystem.xsum[1]);
	if(filesystem.xsum[0]!=buffer[510] || filesystem.xsum[1]!=buffer[511])
	{
		printf("Root sector error: invalid checksum\n");
		return -3;
	}

	numentry=buffer[13];
	if(numentry>31)
	{
		printf("Root sector error: invalid entry number\n");
		return -4;
	}

	printf("Number of log entries: %d\n",numentry);
	filesystem.logentry.clear();
	for(unsigned short i=0;i<numentry;i++)
	{
		logentry = (fs_logentry*)(buffer + 14 + i*16);
		filesystem.logentry.push_back(*logentry);

		printf("Log %d\t Device %lX\t StartTime %lu\t StartBlock %lu\t EndBlock %lu\n",i,logentry->device,logentry->starttime,logentry->blockstart,logentry->blockend);
	}
	if(numentry==0)
		nextsector=1;
	else
	{
		logentry = (fs_logentry*)(buffer + 14 + (numentry-1)*16);
		nextsector=logentry->blockend+1;
	}
	printf("nextsector: %ld\n",nextsector);

	filesystem.ok=true;
	return 0;
}
