#include "../instrument.h"

const struct instrument piano = {
	5,
	(double[]){1., 1., .1, .2, .2},
	1. + 1. + .1 + .2 + .2,
	default_envelope,
/*	{
		2205,
		22050, // 0.3s
		0.2,
		4410,
	},*/
};
