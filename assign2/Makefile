#
# CMPSC311 - F15 Assignment #2
# Makefile - makefile for the assignment
#

# Make environment
INCLUDES=-I. 
CC=gcc
CFLAGS=-I. -c -g -Wall $(INCLUDES)
LINKARGS=-g
LIBS=-lraidlib -lm -lcmpsc311 -L. -lgcrypt -lpthread -lcurl
                    
# Suffix rules
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS)  -o $@ $<
	
# Files
OBJECT_FILES=	tagline_sim.o \
				tagline_driver.o \
				
RAIDLIB=libraidlib.a

# Productions
all : tagline_sim

tagline_sim : $(OBJECT_FILES) 
	$(CC) $(LINKARGS) $(OBJECT_FILES) -o $@ $(LIBS)

clean : 
	rm -f tagline_sim $(OBJECT_FILES)
	
test: tagline_sim 
	./tagline_sim -v sample-workload.dat
