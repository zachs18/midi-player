default: all ;

HEADERS := $(wildcard *.h)
OBJS := $(HEADERS:.h=.o)

.PHONY: test
test:
	gcc

.PHONY: all
all: ${OBJS} Makefile ;

%.raw: %.ogg
	ffmpeg -loglevel warning -i "$<" -f s16le "$@"

%.o:
#	objcopy --input binary --output elf64-x86-64 -i 4 --interleave-width=2 --byte 0 $< $@
	objcopy --input binary --output elf64-x86-64 $< $@

clean:
	rm -f *.o *.raw

35.o: Acoustic_Bass_Drum.raw
36.o: Bass_Drum.raw
37.o: Side_Stick.raw
38.o: Acoustic_Snare.raw

40.o: Electric_Snare.raw

44.o: Pedal_High_Hat.raw

55.o: Splash_Cymbal.raw

57.o: Crash_Cymbal_2.raw

