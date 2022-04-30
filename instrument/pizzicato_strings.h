#include "../instrument.h"

const struct instrument pizzicato_strings = {
	8,
	(double[]){1., .7, .3, .4, .4, .2, .4, .1},
	1. + .7 + .3 + .4 + .4 + .2 + .4 + .1,
	{
		1,
		17640.0, // 0.4s
		0.0,
		0,
	},
};
