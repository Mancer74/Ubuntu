#ifndef RAID_DRIVER_INCLUDED
#define TAGLINE_DRIVER_INCLUDED

////////////////////////////////////////////////////////////////////////////////
//
//  File           : tagline_driver.h
//  Description    : This is the header file for the driver interface between
//                   the OS and the low-level driver.
//
//  Author         : Patrick McDaniel
//  Creates        : Mon Jun 15 07:51:29 EDT 2015
//

// Includes
#include "raid_bus.h"

// Project Includes
#define MAX_TAGLINE_BLOCK_NUMBER  128
#define TAGLINE_BLOCK_SIZE        RAID_BLOCK_SIZE
#define RAID_DISKS                5
#define RAID_DISKBLOCKS           8192

// Type definitions
typedef uint16_t TagLineNumber;
typedef uint32_t TagLineBlockNumber;

//
// Interface functions

int tagline_driver_init(uint32_t maxlines);
	// Initialize the driver with a number of maximum lines to process

int tagline_read(TagLineNumber tag, TagLineBlockNumber bnum, uint8_t blks, char *buf);
	// Read a number of blocks from the tagline driver

int tagline_write(TagLineNumber tag, TagLineBlockNumber bnum, uint8_t blks, char *buf);
	// Write a number of blocks from the tagline driver

int tagline_close(void);
	// Close the tagline interface

#endif /* RAID_DRIVER_INCLUDED */
