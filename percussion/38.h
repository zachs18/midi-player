#include "../percussion.h"

extern signed short _binary_Acoustic_Snare_raw_start[];
extern signed short _binary_Acoustic_Snare_raw_end[];
extern int _binary_Acoustic_Snare_raw_size;

const struct percussion acoustic_snare_38 = {
	_binary_Acoustic_Snare_raw_start,
	_binary_Acoustic_Snare_raw_end,
};
