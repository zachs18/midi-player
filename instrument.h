#ifndef INSTRUMENT_H_INCLUDED
#define INSTRUMENT_H_INCLUDED

struct envelope {
	int attack;
	int decay;
	double sustain;
	int release;

	// https://en.wikipedia.org/wiki/Envelope_(music)
	// times are in 1/44100 second intervals

	// attack is the time until full volume (plus 1, i.e. 1 represents 0)
	// decay is the time from full volume until sustain volume
	// sustain is the volume during sustain
	// release is the time from release to no volume
};

static const struct envelope default_envelope = {
	441,
	22050,
	0.5,
	4410,
};

struct instrument {
	int count;
	double *amplitudes;
	double fullamplitude;
	struct envelope envelope;
};

extern const struct instrument instruments[128];

#endif // ndef INSTRUMENT_H_INCLUDED
