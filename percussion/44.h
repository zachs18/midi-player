#include "../percussion.h"

extern signed short _binary_Pedal_High_Hat_raw_start[];
extern signed short _binary_Pedal_High_Hat_raw_end[];
extern int _binary_Pedal_High_Hat_raw_size;

const struct percussion pedal_high_hat_44 = {
	_binary_Pedal_High_Hat_raw_start,
	_binary_Pedal_High_Hat_raw_end,
};
