/***
	This file is part of PulseAudio.

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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <math.h>
#include <poll.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFSIZE 128
#define RATE 44100
#define AMPLITUDE 32000
#define NOTE_COUNT 16

#define NOTE_ON 0
#define NOTE_OFF 1

int main(int argc, char*argv[]) {

	/* The Sample format to use */
	static const pa_sample_spec ss = {
		.format = PA_SAMPLE_S16LE,
		.rate = RATE,
		.channels = 1
	};
	FILE *temp = fopen("temp.raw", "wb");
	
	struct pollfd pollfd = {STDIN_FILENO, POLLIN, 0};

	pa_simple *s = NULL;
	int ret = 1;
	int error;
	int freqs[NOTE_COUNT] = {0};
	int amps[NOTE_COUNT] = {0};
	
	
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
		if (pollfd.revents) fprintf(stderr, "revents: 0x%x (in: 0x%x)\n", pollfd.revents, POLLIN);
		while (pollfd.revents & (POLLIN | POLLHUP)) {
			fprintf(stderr, "POLLIN\n");
			pollfd.revents = 0;
			char msg[8]; // note_on: 3, note_off: 2
			int note, vel;
			int ret = read(STDIN_FILENO, msg, 1);
			if (ret == 0) goto finish; // eof
			if (msg[0] == NOTE_ON) {
				ret = read(STDIN_FILENO, msg, 2);
				if (ret != 2) goto finish;
				note = msg[0]; // note number
				vel = msg[1]; // velocity
				for(int i = 0; i < NOTE_COUNT; ++i) {
					fprintf(stderr, "%d: note #%d on %d at %d\n", __LINE__, i, freqs[i], amps[i]);
					if (!freqs[i]) {
						freqs[i] = 440. * pow(2, (note - 69) / 12.);
						fprintf(stderr, "%d (%d) on %d\n", note, freqs[i], i);
						amps[i] = vel * (16384 / 0xff);
						break;
					}
				}
			}
			else if (msg[0] == NOTE_OFF) {
				ret = read(STDIN_FILENO, msg, 1);
				if (ret != 1) goto finish;
				int note = msg[0]; // note number
				int freq = 440. * pow(2, (note - 69) / 12.);
				for(int i = 0; i < NOTE_COUNT; ++i) {
					fprintf(stderr, "%d: note #%d on %d at %d (trying to turn off %d)\n", __LINE__, i, freqs[i], amps[i], freq);
					if (freqs[i] == freq) {
						fprintf(stderr, "%d (%d) off %d\n", note, freq, i);
						freqs[i] = amps[i] = 0;
						break;
					}
				}
			}
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
		for (int i = 0; i < BUFSIZE; ++i, ++sample_index) {
			double wav = 0;
			int ampsum = 0;
			for (int j = 0; j < NOTE_COUNT; ++j) {
				ampsum += amps[j];
				wav += amps[j] * sin(2.*M_PI*sample_dt*sample_index*freqs[j]);
			}
			if (ampsum > 32767)
				buf[i] = wav * (32767. / ampsum);
			else
				buf[i] = wav;
		}

		/* ... and play it */
		fwrite(buf, 2, BUFSIZE, temp);
		fflush(temp);
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

	ret = 0;

finish:

	if (s)
		pa_simple_free(s);

	return ret;
}
