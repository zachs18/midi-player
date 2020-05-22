default: Debug ;

CFLAGS := -lm -lpulse -lpulse-simple -Wall -g

PERCUSSION_HEADERS := $(wildcard percussion/*.h)
PERCUSSION_OBJS := $(PERCUSSION_HEADERS:.h=.o)

%.o: %.c $(wildcard *.h) $(PERCUSSION_HEADERS)
	gcc -c $< -o $@ $(CFLAGS)

compile: midi.o percussion.o instrument.o percussion
#	gcc $(CFLAGS) -static midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi
#	gcc $(CFLAGS) -static midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_violin -DSTARTING_INSTRUMENT=violin
	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi $(CFLAGS)
	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_violin $(CFLAGS) -DSTARTING_INSTRUMENT=violin
	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_flute $(CFLAGS) -DSTARTING_INSTRUMENT=flute
	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_square $(CFLAGS) -DSTARTING_INSTRUMENT=square
	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_saw $(CFLAGS) -DSTARTING_INSTRUMENT=saw
	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_sine $(CFLAGS) -DSTARTING_INSTRUMENT=sine
	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_violin $(CFLAGS) -DSTARTING_INSTRUMENT=violin

midi: compile ;

Debug: midi ;

Release: CFLAGS += -DNO_DEBUG -O2
Release: midi ;

clean:
	rm -f *.o midi
	$(MAKE) -C percussion clean

.PHONY: percussion
percussion:
	$(MAKE) -C percussion
