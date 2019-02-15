#!/usr/bin/env python3
import sys
import os
import time
import mido

def playnote(note, amp):
	if amp != 0:
		sys.stdout.buffer.write(bytes([0, note, amp]))
	else:
		sys.stdout.buffer.write(bytes([1, note]))
	sys.stdout.buffer.flush()

play = mido.MidiFile(sys.argv[1]).play()
while True:
	try:
		for msg in play:
			#sys.stderr.write(str(msg.bytes()))
			if msg.type == 'note_on':
				sys.stdout.buffer.write(bytes(msg.bytes()))
			elif msg.type == 'note_off':
				sys.stdout.buffer.write(bytes(msg.bytes()))
			sys.stdout.buffer.flush()
		break
	except KeyboardInterrupt:
		#sys.stdout.close()
		#https://docs.python.org/3/faq/library.html#why-doesn-t-closing-sys-stdout-stdin-stderr-really-close-it
		exit()
exit()
for msg in mido.MidiFile(sys.argv[1]).play():
	#sys.stderr.write(str(msg.bytes()))
	if msg.type == 'note_on' and msg.velocity > 0:
		sys.stdout.buffer.write(bytes(msg.bytes()))
	elif msg.type == 'note_off':
		sys.stdout.buffer.write(bytes(msg.bytes()))
	elif msg.type == 'note_on' and msg.velocity == 0:
		sys.stdout.buffer.write(bytes([0x80+msg.channel, msg.note, 0]))
	sys.stdout.buffer.flush()