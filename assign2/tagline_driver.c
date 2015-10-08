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

// Defines
#define NUM_OF_TAGLINES		1  // number of taglines recieved

uint32_t memory[RAID_DISKS][RAID_DISKBLOCKS];
int Current_Filled[RAID_DISKS];
typedef struct
{
	uint16_t tag_name;	//the name of the tagline
	int addresses[MAX_TAGLINE_BLOCK_NUMBER][2];
} TAGLINE;


TAGLINE *tags[(NUM_OF_TAGLINES + 1)];

for(int i = 0; i <= NUM_OF_TAGLINES; i++)
{	
	TAGLINE new_tag;
	tags[i] = &new_tag;
}

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

	RAIDOpCode read = make_raid_request(RAID_READ, 
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
	
	uint8_t disk_to_write = 0;

	//figure out which disk has least written to it and use it
	for (int i = 0; i < RAID_DISKS; i++)
        {
		if (Current_Filled[i] == 0)
	        {
			disk_to_write = i;
			i = RAID_DISKS;
		}
		else if (Current_Filled[i] < current_Filled[disk_to_write])
			disk_to_write = i;
	}

	//figure out if the tag/bnum is old or new
	int tag_index = -1;
	int next_tag_index = 0;
	while(tags[next_tag_index]->tag_name != NULL)
	{
		if (tags[next_tag_index]->tag_name == tag)
			tag_index = i;	
		next_tag_index++;
	}
	//if the tagline is new make a request to the least filled disk
	if (index == -1)
	{
		RAIDOpCode write = make_raid_request(RAID_WRITE, blks, disk_to_write, current_Filled[disk_to_write]);
		RAIDOpCode write_result = raid_bus_request(write, buf);

		//this tag is new, so record the name in the next null index of tags	
		tags[next_tag_index]->name = tag;
		//log the locations
		for (int i = 0; i < blks; i++)
		{
			tags[next_tag_index]->addresses[0][bnum + i] = disk_to_write;
			tags[next_tag_index]->addresses[1][bnum + i] = (current_Filled[disk_to_write] + i);
		}
		//increment the array for counting memory
		current_Filled[disk_to_write] += blks;
	}

	//otherwise check the tagblocks
	else
	{	
		for (int i = 0; i < blks; i++)
		{
			if (tags[next_tag_index]->addresses[0][bnum + i] != NULL)
			{
				RAIDOpCode write = make_raid_request(RAID_WRITE, blks, disk_to_write, current_Filled[disk_to_write]);
				RAIDOpCode write_result = raid_bus_request(write, buf);
			}
		}
	}

	RAIDOpCode write = make_raid_request(RAID_WRITE, blks, disk_to_write,/* delete this 0->*/0, 
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
