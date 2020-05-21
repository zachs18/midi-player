#include "../instrument.h"

const struct instrument triangle = {
	7,
	(double[]){1., 0., -1/9., 0., 1/25., 0., -1/49.},
	1. + 1/9. + 1/25. + 1/49.,
	default_envelope,
};
