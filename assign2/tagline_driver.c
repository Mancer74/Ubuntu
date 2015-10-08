///////////////////////////////////////////////////////////////////////////////
//
//  File           : tagline_driver.c
//  Description    : This is the implementation of the driver interface
//                   between the OS and the low-level hardware.
//
//  Author         : ?????
//  Created        : ?????

// Include Files
#include <stdlib.h>
#include <string.h>
#include <cmpsc311_log.h>

// Project Includes
#include "raid_bus.h"
#include "tagline_driver.h"

//
// Functions

RAIDOpCode make_raid_request(uint8_t request_type, uint8_t num_of_blks, uint8_t disk_num, uint32_t block_ID)
{
	
	RAIDOpCode result = 0;
	uint8_t unused = 0;
	result |=  request_type;
	result <<= 8;
	result |=  num_of_blks;
	result <<= 8;
	result |= disk_num;
	result <<= 8;
	result |= unused;
	result <<= 32;
	result |= block_ID;
	return(result);
}

int extract_raid_response(RAIDOpCode resp, *uint8_t request_type, *uint8_t num_of_blks, *uint8_t disk_num, *uint32_t block_ID)
{
	block_ID = resp & 0xffffffff;
	disk_num = resp & 0xff0000000000;
	num_of_blks = resp & 0xff000000000000;
	request_type  = resp & 0xff00000000000000;
	int success_bit = resp & 0x100000000;
	return success_bit;
}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_driver_init
// Description  : Initialize the driver with a number of maximum lines to process
//
// Inputs       : maxlines - the maximum number of tag lines in the system
// Outputs      : 0 if successful, -1 if failure

int tagline_driver_init(uint32_t maxlines) {

	RAIDOpCode init = make_raid_request(RAID_INIT, 1, RAID_DISKS, 0);
	RAIDOpCode init_response = raid_bus_request(init, NULL);
	
	for(int i = 0; i < RAID_DISKS; i++)
	{
		RAIDOpCode format = make_raid_request(RAID_FORMAT, 0, i, 0);
		RAIDOpCode format_response = raid_bus_request(format, NULL);	
	}
	
	// Return successfully
	logMessage(LOG_INFO_LEVEL, "TAGLINE: initialized storage (maxline=%u)", maxlines);
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_read
// Description  : Read a number of blocks from the tagline driver
//
// Inputs       : tag - the number of the tagline to read from
//                bnum - the starting block to read from
//                blks - the number of blocks to read
//                bug - memory block to read the blocks into
// Outputs      : 0 if successful, -1 if failure

int tagline_read(TagLineNumber tag, TagLineBlockNumber bnum, uint8_t blks, char *buf) {

	// Return successfully
	logMessage(LOG_INFO_LEVEL, "TAGLINE : read %u blocks from tagline %u, starting block %u.",
			blks, tag, bnum);
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_write
// Description  : Write a number of blocks from the tagline driver
//
// Inputs       : tag - the number of the tagline to write from
//                bnum - the starting block to write from
//                blks - the number of blocks to write
//                bug - the place to write the blocks into
// Outputs      : 0 if successful, -1 if failure

int tagline_write(TagLineNumber tag, TagLineBlockNumber bnum, uint8_t blks, char *buf) {

	// Return successfully
	logMessage(LOG_INFO_LEVEL, "TAGLINE : wrote %u blocks to tagline %u, starting block %u.",
			blks, tag, bnum);
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_close
// Description  : Close the tagline interface
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int tagline_close(void) {
	RAIDOpCode close = make_raid_request(RAID_CLOSE, 0, 0, 0);
	RAIDOpCode close_response = raid_bus_request(close, NULL);
	// Return successfully
	logMessage(LOG_INFO_LEVEL, "TAGLINE storage device: closing completed.");
	return(0);
}
