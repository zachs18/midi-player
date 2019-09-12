#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

/*
Based on https://www.youtube.com/watch?v=HV1-AjwDJwM

For each byte on stdin:

High nibble:

0	1	2	3	4	5	6	7
C2	D2	E2	G2	A2	C3	D3	E3

8	9	A	B	C	D	E	F
G3	A3	C4	D4	E4	G4	A4	C5

Low nibble:

0	1	2	3	4	5	6	7
whole____	dotted half______	half_____________

8	9	A	B	C	D	E	F
dotted quarter___	quarter__________	eigth____


/*
double freqs[16] = {
	65.40639132514966,
	73.41619197935188,
	82.4068892282175,
	97.99885899543733,
	110.0,
	130.8127826502993,
	146.8323839587038,
	164.81377845643496,
	195.99771799087463,
	220.0,
	261.6255653005986,
	293.6647679174076,
	329.6275569128699,
	391.99543598174927,
	440.0,
	523.2511306011972,
};
*/

int note_midi_values[16] = {
	36, 38, 40,
	43, 45,
	48, 50, 52,
	55, 57,
	60, 62, 64,
	67, 69,
	72,
};

int lengths[16] = {
	8, 8,
	6, 6, 6,
	4, 4, 4,
	3, 3, 3,
	2, 2, 2,
	1, 1,
};

int msleep(long msec) { // https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
	struct timespec ts;
	int res;

	if (msec < 0) {
		errno = EINVAL;
		return -1;
	}

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;

	do {
		res = nanosleep(&ts, &ts);
	} while (res && errno != EINTR);

	return res;
}

int main(int argc, char **argv) {
	FILE *infile = stdin;;
	double length_mul = 1/4.0;
	if (argc > 1) {
		if (strcmp(argv[1], "-")) {
			infile = fopen(argv[1], "rb");
			if (!infile) {
				fprintf(stderr, "Could not open file: %s\n", argv[1]);
				return EXIT_FAILURE;
			}
		}
	}
	if (argc > 2) {
		double tempo;
		sscanf(argv[2], "%lf", &tempo);
		if (tempo) length_mul = (30 / tempo); // tempo in bpm, we want bps/2
		else {
			fprintf(stderr, "Invalid tempo: %s\n", argv[2]);
			return EXIT_FAILURE;
		}
	}

	pthread_t workers[2];
	

}

void *worker(void *info_) {
	const struct info *info = info_;
	FILE *infile = info->infile;
	int length_mul = info->length_mul;
	pthread_mutex_t *inmutex = info->inmutex;
	pthread_mutex_t *outmutex = info->outmutex;

	int last_note_value = 0;
	char c;

	pthread
	while ((c = getc(infile)) != EOF) {
		int note = (c >> 4) & 0x0f;
		int length = c & 0x0f;

		if (last_note_value) {
			putchar(0x90); // MIDI Note on Channel 1
			putchar(last_note_value); // prev note
			putchar(0); // velocity off
		}

		putchar(0x90); // MIDI Note on Channel 1
		putchar(last_note_value = note_midi_values[note]); // prev note
		putchar(128); // velocity on
		fflush(stdout);

		fprintf(stderr, "Sleeping for %d\n", (int)(lengths[length] * 1000 * length_mul));
		msleep(lengths[length] * 1000 * length_mul);
	}

	putchar(0x90); // MIDI Note on Channel 1
	putchar(last_note_value);
	putchar(0);
}
