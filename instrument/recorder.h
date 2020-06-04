#include "../instrument.h"

const struct instrument recorder = {
	5,
	(double[]){1., 0.8, 1., .2, .2},
	1. + 0.8 + 1. + .2 + .2,
	default_envelope,
};
