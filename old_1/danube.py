import sys
import time

bpm = 120

def playnote(note, amp):
	if amp != 0:
		sys.stdout.buffer.write(bytes([0, note, amp]))
	else:
		sys.stdout.buffer.write(bytes([1, note]))
	sys.stdout.buffer.flush()

def rest(t):
	time.sleep(t/(bpm/60))

song = [69, 69, 73, 76, 76, 0, [76+12, 73+12], [76+12, 73+12], 0, [73+12, 69+12], [73+12, 69+12]]

for chord in song:
	if isinstance(chord, list):
		for note in chord:
			playnote(note, 0x3f)
	else:
		playnote(chord, 0x3f)
	rest(0.95)
	if isinstance(chord, list):
		for note in chord:
			playnote(note, 0)
	else:
		playnote(chord, 0)
	rest(0.05)
