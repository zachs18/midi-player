#include "../instrument.h"

const struct instrument saw = {
	7,
	(double[]){1., -1/2., 1/3., -1/4., 1/5., -1/6., 1/7.},
	1. + 1/2. + 1/3. + 1/4. + 1/5. + 1/6. + 1/7.,
	default_envelope,
};
