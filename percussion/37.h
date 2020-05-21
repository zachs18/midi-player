#include "../percussion.h"

extern signed short _binary_Side_Stick_raw_start[];
extern signed short _binary_Side_Stick_raw_end[];
extern int _binary_Side_Stick_raw_size;

const struct percussion side_stick_37 = {
	_binary_Side_Stick_raw_start,
	_binary_Side_Stick_raw_end,
};
