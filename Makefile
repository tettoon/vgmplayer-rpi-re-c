# Makefile

S98PLAYER_OBJS := s98player.o m3u.o s98.o raspi_re.o modules.o util.o
VGMPLAYER_OBJS := vgmplayer.o vgm.o raspi_re.o modules.o util.o
CC := gcc
CFLAGS := -Wall -O2 -I.
LDFLAGS := -pthread -lrt -lz -lwiringPi

.PHONY: all
all: vgmplayer s98player

vgmplayer: $(VGMPLAYER_OBJS)
	$(CC) $(VGMPLAYER_OBJS) $(CFLAGS) $(LDFLAGS) -o $@

s98player: $(S98PLAYER_OBJS)
	$(CC) $(S98PLAYER_OBJS) $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	$(RM) $(S98PLAYER_OBJS) $(VGMPLAYER_OBJS) s98player vgmplayer

