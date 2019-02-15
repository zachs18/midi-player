#!/usr/bin/env python3
import sys
import time
import mido

def playnote(note, amp):
	if amp != 0:
		sys.stdout.buffer.write(bytes([0, note, amp]))
	else:
		sys.stdout.buffer.write(bytes([1, note]))
	sys.stdout.buffer.flush()

for msg in mido.MidiFile(sys.argv[1]).play():
	if msg.type == 'note_on':
		playnote(msg.note, msg.velocity)
	elif msg.type == 'note_off':
		playnote(msg.note, 0)
