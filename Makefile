default: Debug ;

CFLAGS += -lm -lpulse -lpulse-simple -Wall -Wextra -ffast-math

HEADERS := $(wildcard *.h)

PERCUSSION_HEADERS := $(wildcard percussion/*.h)
PERCUSSION_OBJS := $(PERCUSSION_HEADERS:.h=.o)

%.o: %.c $(HEADERS) $(PERCUSSION_HEADERS)
	gcc -c -fPIC $< -o $@ $(CFLAGS)

#%.so: %.c $(HEADERS) $(PERCUSSION_HEADERS)
#	gcc -c -shared -fPIC $< -o $@ $(CFLAGS)

%.so: %.o $(HEADERS) $(PERCUSSION_HEADERS) percussion
	$(CC) -shared $< $(PERCUSSION_OBJS) -o $@ $(CFLAGS)

#compile: midi.o percussion.o instrument.o percussion
midi: midi.o percussion.so instrument.so
#	gcc $(CFLAGS) -static midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi
#	gcc $(CFLAGS) -static midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_violin -DSTARTING_INSTRUMENT=violin
	gcc midi.o -o midi $(CFLAGS) -L. -l:instrument.so -l:percussion.so -Wl,-rpath=/home/zachary/Programming/sound
#	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi $(CFLAGS)
#	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_violin $(CFLAGS) -DSTARTING_INSTRUMENT=violin
#	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_flute $(CFLAGS) -DSTARTING_INSTRUMENT=flute
#	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_square $(CFLAGS) -DSTARTING_INSTRUMENT=square
#	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_saw $(CFLAGS) -DSTARTING_INSTRUMENT=saw
#	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_sine $(CFLAGS) -DSTARTING_INSTRUMENT=sine
#	gcc midi.o percussion.o instrument.o $(PERCUSSION_OBJS) -o midi_violin $(CFLAGS) -DSTARTING_INSTRUMENT=violin

midi_keyboard: midi_keyboard.c
	gcc $(CFLAGS) `pkg-config --cflags gtk+-3.0` midi_keyboard.c `pkg-config --libs gtk+-3.0` -o midi_keyboard

Debug: CFLAGS += -g
Debug: midi midi_keyboard ;

Release: CFLAGS += -DNO_DEBUG -O2
Release: midi midi_keyboard ;

clean:
	rm -f *.o *.so midi midi_keyboard
	$(MAKE) -C percussion clean

.PHONY: percussion
percussion:
	$(MAKE) -C percussion
