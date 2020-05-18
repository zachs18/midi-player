#!/bin/bash
gcc percussion.c -c -o percussion.o

gcc midi.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=sine -o midi_sine
gcc midi.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=saw -o midi_saw
gcc midi.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=square -o midi_square
gcc midi.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=triangle -o midi_triangle
gcc midi.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=flute -o midi_flute
gcc midi.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=violin -o midi_violin
gcc midi.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=test -o midi_test

gcc midi_creepy.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=violin -o creepy_midi_violin
gcc midi_creepy.c percussion.o -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=violin -DCREEPDOWN -o creepy_midi_violin_down

gcc `pkg-config --cflags gtk+-3.0` midi_keyboard.c `pkg-config --libs gtk+-3.0` -o midi_keyboard
gcc `pkg-config --cflags gtk+-3.0` midi_keyboard_loop.c `pkg-config --libs gtk+-3.0` -o midi_keyboard_loop
