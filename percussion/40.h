#include "../percussion.h"

extern signed short _binary_Electric_Snare_raw_start[];
extern signed short _binary_Electric_Snare_raw_end[];
extern int _binary_Electric_Snare_raw_size;

const struct percussion electric_snare_40 = {
	_binary_Electric_Snare_raw_start,
	_binary_Electric_Snare_raw_end,
};
