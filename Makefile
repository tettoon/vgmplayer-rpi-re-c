# Makefile

PROGRAM := vgmplayer
OBJS := vgmplayer.o vgm.o raspi_re.o modules.o
CC := gcc
CFLAGS := -Wall -O2 -I.
LDFLAGS := -pthread -lrt -lz -lwiringPi

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	$(RM) $(OBJS) $(PROGRAM)

