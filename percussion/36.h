#include "../percussion.h"

extern signed short _binary_Bass_Drum_raw_start[];
extern signed short _binary_Bass_Drum_raw_end[];
extern int _binary_Bass_Drum_raw_size;

const struct percussion bass_drum_36 = {
	_binary_Bass_Drum_raw_start,
	_binary_Bass_Drum_raw_end,
};
