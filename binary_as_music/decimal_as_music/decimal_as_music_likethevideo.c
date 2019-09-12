#include <stdio.h>
#include <stdbool.h>
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

/*
int note_midi_values[16] = {
//	36, 38, 40,
//	43, 45,
	48, 50, 52,
	55, 57,
	60, 62, 64,
	67, 69,
	60, 62, 64,
	67, 69,
	72,
};
*/
/*
int note_midi_values[8] = {
	55, 57,
	60, 62, 64,
	67, 69,
	72,
};
*/

int note_midi_values[16] = {
//	36, 38, 40,
//	43, 45,
	48, 50, 52,
	55, 57,
	60, 62, 64,
	67, 69,
//	72,

	0,0,0,0,0,0,
};


int lengths[16] = {
	8, 8,
	6, 6,
	4, 4,
	2, 2,
	1, 1,

	1,1,1,1,1,1,
};

void *smalloc(size_t size) {
	void *t = NULL;
	if ((t = malloc(size)) == NULL && t > 0) {
		fprintf(stderr, "Failed to allocate %zd bytes\n", size);
		exit(EXIT_FAILURE);
	}
	return t;
}

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

struct info {
	FILE *infile;
	pthread_mutex_t *inmutex;
	pthread_mutex_t *outmutex;
	pthread_barrier_t *barrier;
	pthread_barrier_t *beat_barrier;
	int id;
};
void *worker(void *info_);
void *chords(void *info_);

int main(int argc, char **argv) {
	FILE *infile = stdin;;
	double length_mul = 1/8.0;
	int workercount = 1;
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
		int r;
		r = sscanf(argv[2], "%lf", &tempo);
		if (r && tempo) length_mul = (15 / tempo); // tempo in bpm, we want bps/4
		else {
			fprintf(stderr, "Invalid tempo: %s\n", argv[2]);
			return EXIT_FAILURE;
		}
	}
	if (argc > 3) {
		int w, r;
		r = sscanf(argv[3], "%d", &w);
		if (!r || w < 1 || w > 16) {
			fprintf(stderr, "Invalid worker count: %s\n", argv[2]);
			return EXIT_FAILURE;
		}
		workercount = w;
	}

	struct info info = {
		infile,
		smalloc(sizeof(pthread_mutex_t)),
		smalloc(sizeof(pthread_mutex_t)),
		smalloc(sizeof(pthread_barrier_t)),
		smalloc(sizeof(pthread_barrier_t)),
		0,
	};

	pthread_mutex_init(info.inmutex, NULL);
	pthread_mutex_init(info.outmutex, NULL);
	pthread_barrier_init(info.barrier, NULL, 2);
	pthread_barrier_init(info.beat_barrier, NULL, 2+workercount);

	pthread_t *workers = smalloc((workercount+1)*sizeof(pthread_t*));
	pthread_create(&workers[0], NULL, chords, (void*) &info);
	pthread_barrier_wait(info.barrier);
	for (int w = 1; w <= workercount; ++w) {
		info.id = w;
		pthread_create(&workers[w], NULL, worker, (void*) &info);
		pthread_barrier_wait(info.barrier);
	}
	while (!feof(infile)) {
		pthread_barrier_wait(info.beat_barrier);
		msleep(1000 * length_mul);
//		msleep(940 * length_mul);
//		pthread_barrier_wait(info.beat_barrier);
//		msleep(60 * length_mul);
	}
	for (int w = 0; w <= workercount; ++w) {
		pthread_cancel(workers[w]);
	}

}

void *worker(void *info_) {
	const struct info *info = info_;
	FILE *const infile = info->infile;
	pthread_mutex_t *const inmutex = info->inmutex;
	pthread_mutex_t *const outmutex = info->outmutex;
	pthread_barrier_t *const init_barrier = info->barrier;
	pthread_barrier_t *const beat_barrier = info->beat_barrier;
	const int channel = info->id + 0x90;

	pthread_barrier_wait(init_barrier);

	int last_note_value = 0;
	int c;

	int values[6] = {
		0, // count
		0, // volume
		0, // note
		0, // length
		0,
		0,
	};

	while (!feof(infile)) {
		while (values[0] < 3) {
			pthread_mutex_lock(inmutex);
			do {
				c = getc(infile);
			} while ((c != EOF && c > 199) && (fprintf(stderr, "Skipping c == %d\n", c) || true));
			pthread_mutex_unlock(inmutex);



			if (c != EOF) {
				values[values[0]+1] = (c / 16) % 16; // BCD
				values[values[0]+2] = c % 16;
				fprintf(stderr, "Got c == %d, values[0] = %d, values[1] = %d, values[2] = %d, values[3] = %d, values[4] = %d\n", c, values[0], values[1], values[2], values[3], values[4]);
			}
			else {
				values[values[0]+1] = 0;
				values[values[0]+2] = 0;
				fprintf(stderr, "c == EOF, feof(infile) == %d\n", (int) feof(infile));
			}
			values[0] += 2;
		}

		int note = values[1], length = values[2], volume = values[3];

		pthread_mutex_lock(outmutex);
		putchar(channel); // MIDI Note on Channel 1
		putchar(last_note_value = note_midi_values[note]); // prev note
		putchar(100+8*volume); // velocity on
		fflush(stdout);
		pthread_mutex_unlock(outmutex);

		fprintf(stderr, "Channel %x playing %d at volume %d for %d half beats\n", channel, last_note_value, volume, lengths[length]);
		for (int b = 0; b < lengths[length]; ++b) {
//			pthread_barrier_wait(beat_barrier);
			pthread_barrier_wait(beat_barrier);
		}
//		pthread_barrier_wait(beat_barrier);


		pthread_mutex_lock(outmutex);
		putchar(channel); // MIDI Note on Channel
		putchar(last_note_value); // prev note
		putchar(0); // velocity off
		fflush(stdout);
		pthread_mutex_unlock(outmutex);

//		pthread_barrier_wait(beat_barrier);

		values[1] = values[2];
		values[2] = values[3];
		values[3] = values[4];
		values[4] = values[5];
		--values[0];
	}
	perror("Um\n");
	pthread_mutex_unlock(inmutex);
	fprintf(stderr, "Worker finished (channel %x)\n", channel);
	fflush(stderr);
}
void *chords(void *info_) {
	const struct info *info = info_;
	FILE *const infile = info->infile;
	pthread_mutex_t *const inmutex = info->inmutex;
	pthread_mutex_t *const outmutex = info->outmutex;
	pthread_barrier_t *const init_barrier = info->barrier;
	pthread_barrier_t *const beat_barrier = info->beat_barrier;
	const int channel = info->id + 0x90;

	pthread_barrier_wait(init_barrier);

	const static int notes[33][5] = {
		{1, 48}, // C
		{2, 52, 55}, // E G
		{1, 48}, // C
		{2, 52, 55}, // E G
		{1, 48}, // C
		{2, 52, 55}, // E G
		{1, 48}, // C
		{3, 47}, // B

		{1, 45}, // A
		{2, 48, 52}, // C E
		{1, 45}, // A
		{2, 48, 52}, // C E
		{1, 45}, // A
		{2, 48, 52}, // C E
		{1, 45}, // A
		{1, 43}, // G

		{1, 41}, // F
		{2, 45, 48}, // A C
		{1, 41}, // F
		{2, 45, 48}, // A C
		{1, 41}, // F
		{2, 45, 48}, // A C
		{1, 41}, // F
		{2, 45, 48}, // A C

		{1, 43}, // G
		{2, 47, 50}, // B D
		{1, 43}, // G
		{2, 47, 50}, // B D
		{1, 43}, // G
		{2, 47, 50}, // B D
		{1, 43}, // G
		{2, 47, 50}, // B D

		{3, 48, 52, 55, 60}, // C E G
	};


	while (!feof(infile)) {

		for (int i = 0; i < 32; ++i) {
	
			pthread_mutex_lock(outmutex);
			for (int n = 1; n <= notes[i][0]; ++n) {
				putchar(channel); // MIDI Note on Channel 1
				putchar(notes[i][n]);
				putchar(96); // velocity on
			}
			fflush(stdout);
			pthread_mutex_unlock(outmutex);

//			fprintf(stderr, "Channel %x sleeping for %d\n", channel, (int)(2000 * length_mul));
//			pthread_barrier_wait(beat_barrier);
//			pthread_barrier_wait(beat_barrier);
			pthread_barrier_wait(beat_barrier);
			pthread_barrier_wait(beat_barrier);
//			msleep(2000 * length_mul);

			pthread_mutex_lock(outmutex);
			for (int n = 1; n <= notes[i][0]; ++n) {
				putchar(channel); // MIDI Note on Channel 1
				putchar(notes[i][n]);
				putchar(0); // velocity off
			}
			fflush(stdout);
			pthread_mutex_unlock(outmutex);
		}
	}

	pthread_mutex_lock(outmutex);
	for (int n = 1; n <= notes[32][0]; ++n) {
		putchar(channel); // MIDI Note on Channel 1
		putchar(notes[32][n]);
		putchar(96); // velocity on
	}
	fflush(stdout);
	pthread_mutex_unlock(outmutex);

//		fprintf(stderr, "Channel %x sleeping for %d\n", channel, (int)(2000 * length_mul));

	for (int i = 0; i < 10; ++i) {
		pthread_barrier_wait(beat_barrier);
	}

	pthread_mutex_lock(outmutex);
	for (int n = 1; n <= notes[32][0]; ++n) {
		putchar(channel); // MIDI Note on Channel 1
		putchar(notes[32][n]);
		putchar(0); // velocity off
	}
	fflush(stdout);
	pthread_mutex_unlock(outmutex);


}
