#!/bin/bash
#if [[ "$1" = "--flute" ]]; then
#	./midi.py "$2" | ./midi_flute
#elif [[ "$1" = "--saw" ]]; then
#	./midi.py "$2" | ./midi_saw
#elif [[ "$1" = "--triangle" ]]; then
#	./midi.py "$2" | ./midi_triangle
#elif [[ "$1" = "--square" ]]; then
#	./midi.py "$2" | ./midi_square
#elif [[ "$1" = "--test" ]]; then
#	./midi.py "$2" | ./midi_test
#elif [[ "$1" = "--violin" ]]; then
#	./midi.py "$2" | ./midi_violin
#else
	./midi.py "$1" | ./midi
#fi
