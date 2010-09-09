#ifndef XCOMMAND_H
#define XCOMMAND_H

/******************************************************************************
*******************************************************************************
Command interface
*******************************************************************************

	The format of the received commands is as follows:
		STX CMD PAYLOAD[8] CKSUM ETX		: 12 bytes

	The format of the response commands is as follows:
		STX CMD PAYLOAD[8] CKSUM ETX		: 12 bytes

	For debug purposes: STX=S, ETX=E.

******************************************************************************/
#define XCOMMAND_SIZE 20
typedef struct
{
	unsigned char cmd;
	unsigned char payload[XCOMMAND_SIZE-4];
	unsigned char xsum;
} XCOMMAND;



#endif // XCOMMAND_H
