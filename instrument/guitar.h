#include "../instrument.h"

const struct instrument guitar = {
	8,
	(double[]){1., .7, .3, .4, .4, .2, .4, .1},
	1. + .7 + .3 + .4 + .4 + .2 + .4 + .1,
	{
		1,
		44100, // 1.0s
		0.2,
		441,
	},
};
