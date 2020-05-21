#include "../instrument.h"

const struct instrument violin = {
	8,
	(double[]){1., .6, .6, .7, .4, .2, .4, .1},
	1. + .6 + .6 + .7 + .4 + .2 + .4 + .1,
	default_envelope,
};
