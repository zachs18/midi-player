#!/bin/bash
gcc midi.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=sine -o midi_sine
gcc midi.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=saw -o midi_saw
gcc midi.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=square -o midi_square
gcc midi.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=triangle -o midi_triangle
gcc midi.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=flute -o midi_flute
gcc midi.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=violin -o midi_violin

gcc midi_creepy.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=violin -o creepy_midi_violin
gcc midi_creepy.c -lm -lpulse -lpulse-simple -DSTARTING_INSTRUMENT=violin -DCREEPDOWN -o creepy_midi_violin_down
