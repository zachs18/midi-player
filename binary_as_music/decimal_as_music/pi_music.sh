#!/bin/bash
cd /home/zachary/Programming/sound/binary_as_music/decimal_as_music
python3 pi_digits.py   | ./video_decimal_as_music - 90 2>/dev/null | ../../midi_flute
