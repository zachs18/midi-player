#include "../percussion.h"

extern signed short _binary_Acoustic_Bass_Drum_raw_start[];
extern signed short _binary_Acoustic_Bass_Drum_raw_end[];
extern int _binary_Acoustic_Bass_Drum_raw_size;

const struct percussion acoustic_bass_drum_35 = {
	_binary_Acoustic_Bass_Drum_raw_start,
	_binary_Acoustic_Bass_Drum_raw_end,
};
