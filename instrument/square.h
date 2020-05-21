#include "../instrument.h"

const struct instrument square = {
	7,
	(double[]){1., 0., 1/3., 0., 1/5., 0., 1/7.},
	1. + 1/3. + 1/5. + 1/7.,
	default_envelope,
};
