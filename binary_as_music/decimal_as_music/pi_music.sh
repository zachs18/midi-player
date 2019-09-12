#!/bin/bash
cd /home/zachary/Programming/sound/binary_as_music/decimal_as_music
./pi_digits.py | ./video_decimal_as_music - 100 2>/dev/null | ../../midi_flute
