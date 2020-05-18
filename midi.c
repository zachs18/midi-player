/***
	This file is adapted from PulseAudio's pacat-simple.c.
	
	This file is NOT part of PulseAudio itself.

	PulseAudio is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published
	by the Free Software Foundation; either version 2.1 of the License,
	or (at your option) any later version.

	PulseAudio is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <math.h>
#include <poll.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "percussion.h"

#define BUFSIZE 128
#define RATE 44100
#define AMPLITUDE 32000
#define NOTE_COUNT 8
#define CHANNEL_COUNT 16

#define CHANNEL_MASK 0x0f
#define MSG_MASK 0xf0
#define NOTE_ON 0x90
#define NOTE_OFF 0x80

struct spectrum {
	int count;
	double *amps;
	double fullamp;
};

static struct spectrum violin = {
	8,
	(double[]){1., .6, .6, .7, .4, .2, .4, .1},
	1. + .6 + .6 + .7 + .4 + .2 + .4 + .1,
};

static struct spectrum sine = {
	1,
	(double[]){1.},
	1.,
};
static struct spectrum test = {
	3,
	(double[]){0, 1., 0.6},
	1.6,
};

static struct spectrum saw = {
	7,
	(double[]){1., -1/2., 1/3., -1/4., 1/5., -1/6., 1/7.},
	1. + 1/2. + 1/3. + 1/4. + 1/5. + 1/6. + 1/7.,
};

static struct spectrum square = {
	7,
	(double[]){1., 0., 1/3., 0., 1/5., 0., 1/7.},
	1. + 1/3. + 1/5. + 1/7.,
};

static struct spectrum triangle = {
	7,
	(double[]){1., 0., -1/9., 0., 1/25., 0., -1/49.},
	1. + 1/9. + 1/25. + 1/49.,
};

static struct spectrum flute = {
	5,
	(double[]){1., 1., .1, .2, .2},
	1. + 1. + .1 + .2 + .2,
};

int main(int argc, char*argv[]) {

	/* The Sample format to use */
	static const pa_sample_spec ss = {
		.format = PA_SAMPLE_S16LE,
		.rate = RATE,
		.channels = 1
	};
	//FILE *temp = fopen("temp.raw", "wb");
	
	struct pollfd pollfd = {STDIN_FILENO, POLLIN, 0};

	pa_simple *s = NULL;
	int prog_ret = 1;
	int error;
	int notes[CHANNEL_COUNT][NOTE_COUNT] = {0};
	double freqs[CHANNEL_COUNT][NOTE_COUNT] = {0.};
	int amps[CHANNEL_COUNT][NOTE_COUNT] = {0};
	double *percussion_sample_positions[NOTE_COUNT] = {0}; // channel 9 percussion samples
	struct spectrum *instruments = calloc(CHANNEL_COUNT, sizeof(struct spectrum));
	for (int i = 0; i < CHANNEL_COUNT; ++i) instruments[i] = STARTING_INSTRUMENT;
	//instruments[0] = violin;
	

	time_t start_time = time(NULL);
	signal(SIGINT, SIG_IGN); // so the python script wil catch it
	
	//int sample_count; //  = RATE / freq;

	
	//int sample_count = 2*RATE / freq;
	double sample_dt = 1 / (double)RATE; // delta time
	int sample_index = 0;

	/* Create a new playback stream */
	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		goto finish;
	}
	// Can't use poll and stdio, have to use read
	for (;;) {
		signed short buf[BUFSIZE];
		ssize_t buf_bytes = BUFSIZE*sizeof(signed short);
		ssize_t read_bytes = 0;
		
		poll(&pollfd, 1, 1);
		//if (pollfd.revents) fprintf(stderr, "revents: 0x%x (in: 0x%x)\n", pollfd.revents, POLLIN);
		while (pollfd.revents & (POLLIN | POLLHUP)) {
			//fprintf(stderr, "POLLIN\n\n");
			pollfd.revents = 0;
			unsigned char msg[8]; // note_on: 3 bytes, note_off: 3 bytes
			int note, vel, channel;
			int ret = read(STDIN_FILENO, msg, 1);
			if (ret == 0) { // eof
				prog_ret = 0;
				goto finish;
			}
			if ((msg[0]&MSG_MASK) == NOTE_ON) {
				//fprintf(stderr, "NOTE_ON\n");
				channel = msg[0] & CHANNEL_MASK;
				ret = read(STDIN_FILENO, msg, 2);
				if (ret != 2) goto finish;
				note = msg[0]; // note number
				vel = msg[1]; // velocity
				for(int i = 0; i < NOTE_COUNT; ++i) {
					//fprintf(stderr, "%d: note #%d on %d at %d\n", __LINE__, i, freqs[i], amps[i]);
					if (vel == 0 && notes[channel][i] == note) {
						notes[channel][i] = freqs[channel][i] = amps[channel][i] = 0;
						break;
					}
					else if (vel > 0 && !notes[channel][i]) {
						if (channel == 9) { // percussion
							freqs[channel][i] = notes[channel][i] = note;
							amps[channel][i] = vel * (8192 / 0xff);
							percussion_sample_positions[i] = percussions[note];
						}
						else {
							//double freq = 440. * pow(2, (note - 69) / 12.);
							freqs[channel][i] = 440. * pow(2, (note - 69) / 12.);
							notes[channel][i] = note;
							amps[channel][i] = vel * (8192 / 0xff);
							//fprintf(stderr, "%d (%d) on %d\n", note, freqs[i], i);
						}
						break;
					}
				}
			}
			else if ((msg[0]&MSG_MASK) == NOTE_OFF) {
				//fprintf(stderr, "NOTE_OFF\n");
				channel = msg[0] & CHANNEL_MASK;
				ret = read(STDIN_FILENO, msg, 2);
				if (ret != 2) goto finish;
				int note = msg[0]; // note number
				//int freq = 440. * pow(2, (note - 69) / 12.);
				for(int i = 0; i < NOTE_COUNT; ++i) {
					//fprintf(stderr, "%d: note #%d on %d at %d (trying to turn off %d)\n", __LINE__, i, freqs[i], amps[i], freq);
					if (notes[channel][i] == note) {
						//fprintf(stderr, "%d (%d) off %d\n", note, freq, i);
						notes[channel][i] = freqs[channel][i] = amps[channel][i] = 0;
						break;
					}
				}
			}
			//else fprintf(stderr, "0x%x\n", msg[0]);
			poll(&pollfd, 1, 1);
		}

#if 0
		pa_usec_t latency;

		if ((latency = pa_simple_get_latency(s, &error)) == (pa_usec_t) -1) {
			fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
			goto finish;
		}

		fprintf(stderr, "%0.0f usec    \r", (float)latency);
#endif

		//NO /* Read some data ... */
		// make some data
		//for (int i = 0; i < BUFSIZE; ++i, (sample_index = (sample_index+1)%sample_count)) {
		int ampsum = 0;
		for (int i = 0; i < CHANNEL_COUNT; ++i)
			for (int j = 0; j < NOTE_COUNT; ++j)
				ampsum += amps[i][j];
		for (int i = 0; i < BUFSIZE; ++i, ++sample_index) {
			double wav = 0;
			for (int channel = 0; channel < CHANNEL_COUNT; ++channel) {
				if (channel != 9) { // not percussion
					for (int j = 0; j < NOTE_COUNT; ++j) {
						if (!amps[channel][j]) continue;
						double wava = 0;
						for (int k = 0; k < instruments[channel].count; ++k) {
							wava += instruments[channel].amps[k] * sin((k+1)*2.*M_PI*sample_dt*sample_index*freqs[channel][j]);
						}
						wav += wava * amps[channel][j] / instruments[channel].fullamp;
						//wav += amps[j] * sin(4.*M_PI*sample_dt*sample_index*freqs[j]) / 3.;
						//wav += amps[j] * sin(6.*M_PI*sample_dt*sample_index*freqs[j]) / 6.;
						//wav += amps[j] * sin(8.*M_PI*sample_dt*sample_index*freqs[j]) / 10.;
					}
				}
				else { // percussion
					for (int j = 0; j < NOTE_COUNT; ++j) {
						if (!percussion_sample_positions[j] || !amps[channel][j]) continue;
//							if (!percussion_sample_positions[j]) continue;
//						}
						wav += *percussion_sample_positions[j]++ * amps[channel][j];
						if (*percussion_sample_positions[j] > 1.0) { // greater than 1.0 sample indicates end of wave
							percussion_sample_positions[j] = NULL;
							amps[channel][j] = 0;
						}
					}
					// idk
				}
			}
			if (ampsum > 32767)
				buf[i] = wav * (32767. / ampsum);
			else
				buf[i] = wav;
		}
		
		// print notes
//		fprintf(stderr, "time: %lld\n", (long long int) time(NULL) - start_time);
		fprintf(stderr, "chnls:\x1b[0K");
		for (int i = 0; i < CHANNEL_COUNT; ++i)
			for (int j = 0; j < NOTE_COUNT; ++j)
				if (amps[i][j])
					fprintf(stderr, " (%2d,%1d)\x1b[0K", i, j);
		fprintf(stderr, "\n");
		fprintf(stderr, "freqs:\x1b[0K");
		for (int i = 0; i < CHANNEL_COUNT; ++i)
			for (int j = 0; j < NOTE_COUNT; ++j)
				if (amps[i][j])
					fprintf(stderr, freqs[i][j] <= 999. ? " %6.2f\x1b[0K" : " %6.1f\x1b[0K", freqs[i][j]);
		fprintf(stderr, "\n");
		fprintf(stderr, "amps: \x1b[0K");
		for (int i = 0; i < CHANNEL_COUNT; ++i)
			for (int j = 0; j < NOTE_COUNT; ++j)
				if (amps[i][j])
					fprintf(stderr, " %6d\x1b[0K", amps[i][j]);
		fprintf(stderr, "\n");
		fprintf(stderr, "amps: \x1b[0K");
		for (int i = 0; i < CHANNEL_COUNT; ++i)
			for (int j = 0; j < NOTE_COUNT; ++j)
				if (amps[i][j])
					fprintf(stderr, " %6d\x1b[0K", ampsum > 32767 ? (int)(amps[i][j] * (32767. / ampsum)) : amps[i][j]);
//		fprintf(stderr, "\n\x1b[A\x1b[A\x1b[A\x1b[A\x1b[A");
		fprintf(stderr, "\n\x1b[A\x1b[A\x1b[A\x1b[A");
		fflush(stderr);
		
		/* ... and play it */
		//fwrite(buf, 2, BUFSIZE, temp);
		//fflush(temp);
		if (pa_simple_write(s, buf, (size_t) buf_bytes, &error) < 0) {
			fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
			goto finish;
		}
	}

	/* Make sure that every single sample was played */
	if (pa_simple_drain(s, &error) < 0) {
		fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
		goto finish;
	}

	prog_ret = 0;

finish:
	fprintf(stderr, "\r\x1b[0K\n");
	fprintf(stderr, "\r\x1b[0K\n");
	fprintf(stderr, "\r\x1b[0K\n");
	fprintf(stderr, "\r\x1b[0K\n");
	fprintf(stderr, "\r\x1b[0K\x1b[A\x1b[A\x1b[A\x1b[A\x1b[A");
	fprintf(stderr, "\r\x1b[0K");
	if (s)
		pa_simple_free(s);

	return prog_ret;
}
