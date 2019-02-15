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
	int freq = 0;
	
	
	//int sample_count; //  = RATE / freq;

	
	//int sample_count = 2*RATE / freq;
	double sample_dt = 1 / (double)RATE; // delta time
	int sample_index = 0;

	/* Create a new playback stream */
	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		goto finish;
	}

	for (;;) {
		signed short buf[BUFSIZE];
		ssize_t r = 2*BUFSIZE;
		
		poll(&pollfd, 1, 1);
		if (pollfd.revents && POLLIN) {
			pollfd.revents = 0;
			int midinum = getchar();
			if (midinum == EOF) goto finish;
			//if (midinum == NOTE_ON)
			freq = 440. * pow(2, (midinum - 69) / 12.);
			fprintf(stderr, "%d\n", midinum);
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
			buf[i] = AMPLITUDE * sin(2.*M_PI*sample_dt*sample_index*freq);
		}

		/* ... and play it */
		fwrite(buf, 2, BUFSIZE, temp);
		fflush(temp);
		if (pa_simple_write(s, buf, (size_t) r, &error) < 0) {
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
