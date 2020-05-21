#include "../instrument.h"

const struct instrument flute = {
	5,
	(double[]){1., 1., .1, .2, .2},
	1. + 1. + .1 + .2 + .2,
	default_envelope,
};
