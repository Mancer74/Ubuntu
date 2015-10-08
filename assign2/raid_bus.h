#ifndef RAID_BUS_INCLUDED
#define RAID_BUS_INCLUDED

////////////////////////////////////////////////////////////////////////////////
//
//  File           : RAID_BUS.h
//  Description    : This is the header file for the bus implementation
//                   of the RAID storage system.
//
//  Author         : Patrick McDaniel
//  Create         : Sat Sep  6 08:24:25 EDT 2014
//

// Includes
#include <stdint.h>

// Defines
#define RAID_BLOCK_SIZE   1024 // Block size in bytes
#define RAID_TRACK_BLOCKS 64   // Number of blocks per track
#define RAID_MAX_XFER     255  // The maximum blocks per transfer
//
// Type definitions

// These are the request types
typedef enum {
	RAID_INIT         = 0,  // Initialize the RAID interface
	RAID_CLOSE        = 1,  // Close the RAID interface
	RAID_FORMAT       = 2,  // Format a disk in the array
	RAID_READ         = 3,  // Read consecutive blocks in the disk array
	RAID_WRITE        = 4,  // Write consecutive blocks in the disk array
	RAID_HASHBLOCK    = 5,  // Log a hash value for blocks of a disk
	RAID_MAXVAL       = 6,  // Max value
} RAID_REQUEST_TYPES;
extern const char *RAID_REQUEST_TYPE_LABELS[RAID_MAXVAL];

/*

 Request/Response Specification

  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |     req       |     blocks    |   disk num    |             |R|
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                             block ID                          |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Bits    Description
  -----   -------------------------------------------------------------

    0-7   request type
   8-23   number of blocks (each block is 1k)
  15-23   disk number
  24-30   unused (for now)
     63 - R (result) this is the result bit (0 success, 1 is failure)
  32-63   block ID

*/

// These are the fields of the RAID opcodes
typedef enum {
	RAID_OPCODE_REQTYPE = 0, // The request type
	RAID_OPCODE_BLOCKS  = 1, // The number of blocks/tracks
	RAID_OPCODE_DISKID  = 2, // The disk ID of the operation
	RAID_OPCODE_UNUSED  = 3, // This is an unused field
	RAID_OPCODE_STATUS  = 4, // The status field
	RAID_OPCODE_BLOCKID = 5, // The starting block number
	RAID_OPCODE_MAXVAL  = 6, // Max value
} RAID_OPCODE_FIELDS;
extern const char *RAID_OPCODE_FIELDS_LABELS[RAID_OPCODE_MAXVAL];

// RAID request and response types
typedef uint64_t  RAIDOpCode;
typedef uint8_t   RAIDDiskID;
typedef uint32_t  RAIDBlockID;

//
// RAID interface

RAIDOpCode raid_bus_request(RAIDOpCode request, void *buf);
	// This is the interface to the RAID array bus operations

int raid_log_opcode(RAIDOpCode request, const char *txt);
	// This function prints out the opcode and its contents to log

int raid_save_store(char *fname);
	// Write the contents of the RAID store to disk file.

int raid_load_store(char *fname);
	// Read the contents of the storage device from a disk file.

//
// Unit testing for the module

int raid_unit_test( void );
	// This is a function used to test the RAID interfaces and code.

#endif
