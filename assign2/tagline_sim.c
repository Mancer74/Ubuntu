////////////////////////////////////////////////////////////////////////////////
//
//  File          : tagline_sim.c
//  Description   : This is the main program for the CMPSC311 programming
//                  assignment #2 (beginning of TAGLINE simulated device
//                  driver and associated RAID device).
//
//   Author        : Patrick McDaniel
//   Created       : Mon Feb  9 11:07:05 EST 2015
//

// Include Files
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Project Includes
#include <cmpsc311_log.h>
#include <cmpsc311_unittest.h>
#include <raid_bus.h>
#include "tagline_driver.h"

// Defines
#define TLINE_ARGUMENTS "hvul:"
#define USAGE \
	"USAGE: tagline_sim [-h] [-v] [-l <logfile>] [-c <sz>] <workload-file>\n" \
	"\n" \
	"where:\n" \
	"    -h - help mode (display this message)\n" \
	"    -u - run the unit tests instead of the simulator\n" \
	"    -v - verbose output\n" \
	"    -l - write log messages to the filename <logfile>\n" \
	"\n" \
	"    <workload-file> - file contain the workload to simulate\n" \
	"\n" \

//
// Global Data
int verbose;
char rdbuf[TAGLINE_BLOCK_SIZE*MAX_TAGLINE_BLOCK_NUMBER]; // workload simulator read buffer
char wrbuf[TAGLINE_BLOCK_SIZE*MAX_TAGLINE_BLOCK_NUMBER]; // workload simulator write buffer
char tmbuf[TAGLINE_BLOCK_SIZE*MAX_TAGLINE_BLOCK_NUMBER]; // workload simulator temporary buffer

//
// Functional Prototypes

int simulate_TagLines(char *wload);
int tagline_read_block_validate(TagLineNumber tagnum, TagLineBlockNumber blocknum,
		uint16_t num_blocks, char *text);

//
// Functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : main
// Description  : The main function for the Tagline simulator
//
// Inputs       : argc - the number of command line parameters
//                argv - the parameters
// Outputs      : 0 if successful test, -1 if failure

int main(int argc, char *argv[]) {

	// Local variables
	int ch, verbose = 0, unit_tests = -0, log_initialized = 0;

	// Process the command line parameters
	while ((ch = getopt(argc, argv, TLINE_ARGUMENTS)) != -1) {

		switch (ch) {
		case 'h': // Help, print usage
			fprintf(stderr, USAGE);
			return( -1 );

		case 'v': // Verbose Flag
			verbose = 1;
			break;

		case 'u': // Unit Tests Flag
			unit_tests = 1;
			break;

		case 'l': // Set the log filename
			initializeLogWithFilename(optarg);
			log_initialized = 1;
			break;

		default:  // Default (unknown)
			fprintf(stderr, "Unknown command line option (%c), aborting.\n", ch);
			return( -1 );
		}
	}

	// Setup the log as needed
	if (! log_initialized) {
		initializeLogWithFilehandle(CMPSC311_LOG_STDERR);
	}
	if (verbose) {
		enableLogLevels(LOG_INFO_LEVEL);
	}

	// If we are running the unit tests, do that
	if (unit_tests) {

		// Enable verbose, run the tests and check the results
		enableLogLevels(LOG_INFO_LEVEL);

		// 311 Library Unit Tests
		if (cmpsc311_unittests()) {
			logMessage(LOG_ERROR_LEVEL, "Tagline unit tests failed.\n\n");
		} else {
			logMessage(LOG_INFO_LEVEL, "Tagline unit tests completed successfully.\n\n");
		}

		// RAID unit tests
		if (raid_unit_test()) {
			logMessage(LOG_ERROR_LEVEL, "Tagline unit tests failed.\n\n");
		} else {
			logMessage(LOG_INFO_LEVEL, "Tagline unit tests completed successfully.\n\n");
		}

	} else {

		// The filename should be the next option
		if (optind >= argc) {

			// No filename
			fprintf( stderr, "Missing command line parameters, use -h to see usage, aborting.\n" );
			return( -1 );

		}

		// Run the simulation
		if (simulate_TagLines(argv[optind]) == 0) {
			logMessage(LOG_INFO_LEVEL, "Tagline simulation completed successfully.\n\n");
		} else {
			logMessage(LOG_INFO_LEVEL, "Tagline simulation failed.\n\n");
		}
	}

	// Return successfully
	return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : simulate_Taglines
// Description  : The main control loop for the processing of the Tagline
//                simulation and associated drivers.
//
// Inputs       : wload - the name of the workload file
// Outputs      : 0 if successful test, -1 if failure

int simulate_TagLines(char *wload) {

	// Local variables
	char line[1024], command[128], text[1204], txt[5];
	FILE *fhandle = NULL;
	int32_t err=0, linecount, i;
	uint16_t num_blocks;
	TagLineNumber tagnum;
	TagLineBlockNumber blocknum;

	// Open the workload file
	linecount = 0;
	if ((fhandle=fopen(wload, "r")) == NULL) {
		logMessage(LOG_ERROR_LEVEL, "Failure opening the workload file [%s], error: %s.\n",
			wload, strerror(errno));
		return(-1);
	}

	// While file not done
	while (!feof(fhandle)) {

		// Get the line and bail out on fail
		if (fgets(line, 1024, fhandle) != NULL) {

			// Parse out the string
			linecount ++;
			if (sscanf(line, "%s %hu %hu %u %s", command, &tagnum, &num_blocks, &blocknum, text) != 5) {

				// Bad data, error out
				logMessage(LOG_ERROR_LEVEL, "Tagline un-parsable workload string, aborting [%s], line %d",
						line, linecount);
				fclose(fhandle);
				return(-1);

			} else {

				// Just log the contents
				logMessage(LOG_INFO_LEVEL, "INPUT cmd=%s tag=%u #blks=%u start-blk=%u data=%s",
						command, tagnum, num_blocks, blocknum, text);

				// If there is write processing to perform
				if (strncmp(command, "INIT", 5) == 0) {

					// Call the initialize function for the tagline storae
					if (tagline_driver_init(tagnum)) {
						// Error out
						logMessage(LOG_ERROR_LEVEL, "INIT failed on raid array (%d tags)", tagnum);
						err = 1;
					}

				} else if (strncmp(command, "CLOSE", 5) == 0) {

					// Close the tagline storage device
					if (tagline_close()) {
						// Error out
						logMessage(LOG_ERROR_LEVEL, "Close failed on raid array.");
						err = 1;
					}

				} else if (strncmp(command, "READ", 6) == 0) {

					// First check to make sure our input is sane
					if (strlen(text) != num_blocks) {
						// Error out
						logMessage(LOG_ERROR_LEVEL, "Text/number blocks mismatch in input data");
						err = 1;
					} else {

						// Setup the read block to check against
						for (i=0; i<num_blocks; i++) {
							memset(&rdbuf[i*TAGLINE_BLOCK_SIZE], text[i], TAGLINE_BLOCK_SIZE);
						}

						// Read the blocks from the tagline
						if (tagline_read(tagnum, blocknum, num_blocks, tmbuf)) {
							// Error out
							logMessage(LOG_ERROR_LEVEL, "READ failed on tagline storage device (%u)", tagnum);
							err = 1;
						}

						// Now compare the read bytes to see if it is correct
						if (memcmp(rdbuf, tmbuf, num_blocks*TAGLINE_BLOCK_SIZE)) {
							// Error out
							logMessage(LOG_ERROR_LEVEL, "Read blocks data mismatch return from tagline storage.");
							logMessage(LOG_ERROR_LEVEL, "Mismatch [%d] != [%d]", (int)rdbuf[0], (int)tmbuf[0]);
							err = 1;
						}

					}

				}  else if (strncmp(command, "WRITE", 6) == 0) {

					// Setup the write block to send to storage device
					for (i=0; i<num_blocks; i++) {
						memset(&wrbuf[i*TAGLINE_BLOCK_SIZE], text[i], TAGLINE_BLOCK_SIZE);
					}

					// Call the block write function
					if (tagline_write(tagnum, blocknum, num_blocks, wrbuf)) {
						// Error out
						logMessage(LOG_ERROR_LEVEL, "WRITE failed on tagline storage (%d)");
						err = 1;
					}

				} else if (strncmp(command, "tagline", 7) == 0) {

					// Need to save some data here!
					logMessage(LOG_INFO_LEVEL, "Getting tagline final data (%s)", command);

					// TODO: this single block reads are only for first version
					// do a bunch of reads to make sure that the data matches workload indicators
					for (i=0; i<strlen(text); i++) {

						// Setup text, then request validation
						txt[0] = text[i];
						txt[1] = 0x0;
						if (tagline_read_block_validate(tagnum, i, 1, txt)) {
							logMessage(LOG_ERROR_LEVEL, "Tagline validation failed for tag line [%d], aborting.", tagnum);
							return(-1);
						} else {
							logMessage(LOG_INFO_LEVEL, "Tagline validation successful for tag line [%d]", tagnum);
						}
					}

					// Finished validating, success!!!
					logMessage(LOG_INFO_LEVEL, "Tagline validation successful for all taglines, success!!!!");
				}

			}

			// Check for the virtual level failing
			if (err) {
				logMessage(LOG_ERROR_LEVEL, "RAID system failed, aborting [%d]", err);
				fclose(fhandle);
				return(-1);
			}
		}
	}

	// Close the workload file, successfully
	fclose(fhandle);
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_read_block_read
// Description  : The perform a read and validate the result by extracting the
//                data and comparing the memory bytes.
//
// Inputs       : tagnum - the tag line number
//                blocknum - the block number of the tagline to read
//                bum_blocks - the number of blocks to read
//                text - the block contents to validate
// Outputs      : 0 if successful test, -1 if failure

int tagline_read_block_validate(TagLineNumber tagnum, TagLineBlockNumber blocknum,
		uint16_t num_blocks, char *text) {

	// Local variables
	int i;

	// First check to make sure our input is sane
	if (strlen(text) != num_blocks) {

		// Error out
		logMessage(LOG_ERROR_LEVEL, "Text/number blocks mismatch in input data");
		return(-1);

	} else {

		// Setup the read block to check against
		for (i = 0; i < num_blocks; i++) {
			memset(&rdbuf[i * TAGLINE_BLOCK_SIZE], text[i], TAGLINE_BLOCK_SIZE);
		}

		// Read the blocks from the tagline
		if (tagline_read(tagnum, blocknum, num_blocks, tmbuf)) {
			// Error out
			logMessage(LOG_ERROR_LEVEL,
					"READ failed on tagline storage device (%u)", tagnum);
			return(-1);
		}

		// Now compare the read bytes to see if it is correct
		if (memcmp(rdbuf, tmbuf, num_blocks * TAGLINE_BLOCK_SIZE)) {
			// Error out
			logMessage(LOG_ERROR_LEVEL,
					"Read blocks data mismatch return from tagline storage.");
			return(-1);
		}

	}

	// Return successfully
	return(0);
}
