#!/usr/bin/env python3
import sys
import os
import time
import mido
import struct
import array

Envelope = struct.Struct("iidixxxx")

def make_instrument(channel: int, amps: "List[float]") -> bytes:
	return b"\xF0" + \
		struct.pack("B", ((len(amps) << 4) | (channel & 15))) + \
		array.array("d", amps).tobytes() + \
		b"\xF1" + \
		Envelope.pack(4410, 44100, 0.3, 10000)


def playnote(note, amp):
	if amp != 0:
		sys.stdout.buffer.write(bytes([0, note, amp]))
	else:
		sys.stdout.buffer.write(bytes([1, note]))
	sys.stdout.buffer.flush()

file = mido.MidiFile(sys.argv[1])
sys.stderr.write("%.2f seconds\n" % file.length)
play = file.play()
while True:
	try:
		for msg in play:
			#sys.stderr.write(str(msg.bytes()))
			if msg.type == 'note_on':
				sys.stdout.buffer.write(bytes(msg.bytes()))
			elif msg.type == 'note_off':
				sys.stdout.buffer.write(bytes(msg.bytes()))
			elif msg.type == 'program_change':
				sys.stdout.buffer.write(bytes(msg.bytes()))
#				if msg.program > 1:
#					sys.stdout.buffer.write(bytes(msg.bytes()))
#				else:
#					amps = [1.0, 0, 1/4, 0, 1/9, 0, 1/16, 0, 1/49]
#					sys.stdout.buffer.write(make_instrument(msg.channel, amps))
			sys.stdout.buffer.flush()
		break
	except KeyboardInterrupt:
		#sys.stdout.close()
		#https://docs.python.org/3/faq/library.html#why-doesn-t-closing-sys-stdout-stdin-stderr-really-close-it
		exit()
exit()
"""
for msg in mido.MidiFile(sys.argv[1]).play():
	#sys.stderr.write(str(msg.bytes()))
	if msg.type == 'note_on' and msg.velocity > 0:
		sys.stdout.buffer.write(bytes(msg.bytes()))
	elif msg.type == 'note_off':
		sys.stdout.buffer.write(bytes(msg.bytes()))
	elif msg.type == 'note_on' and msg.velocity == 0:
		sys.stdout.buffer.write(bytes([0x80+msg.channel, msg.note, 0]))
	elif msg.type == 'program_change':
		sys.stdout.buffer.write(bytes(msg.bytes()))
	sys.stdout.buffer.flush()
"""
