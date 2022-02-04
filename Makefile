CC = gcc
CFLAGS_XML2 = $(shell xml2-config --cflags)
CFLAGS_CURL = $(shell curl-config --cflags)
CFLAGS = -Wall $(CFLAGS_XML2) $(CFLAGS_CURL) -std=gnu99 -g -DDEBUG1_
LD = gcc
LDFLAGS = -std=gnu99 -g
LDLIBS_XML2 = $(shell xml2-config --libs)
LDLIBS_CURL = $(shell curl-config --libs)
LDLIBS = $(LDLIBS_XML2) $(LDLIBS_CURL)

BUILD_FILES=utils.h utils.c

default: all

all: clean guess

filterwords: filterwords.c $(BUILD_FILES)
	$(CC) $(CFLAGS) -o filterwords $(BUILD_FILES) $< -lz -lcurl -pthread $(LDLIBS) -ggdb3

-include $(SRCS:.c=.d)

guess: guess.c $(BUILD_FILES)
	$(CC) $(CFLAGS) -o guess $(BUILD_FILES) $< -lz -lcurl -pthread $(LDLIBS) -ggdb3

-include $(SRCS:.c=.d)

clean:
	rm -f filterwords
	rm -f guess
	rm -f *.txt

clear:
	rm -f *.txt