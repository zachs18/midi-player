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
#include "instrument.h"

#define BUFSIZE 128
#define RATE 44100
#define AMPLITUDE 32000
#define NOTE_COUNT 8
#define CHANNEL_COUNT 16

#define CHANNEL_MASK 0x0f
#define MSG_MASK 0xf0
#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define PROGRAM_CHANGE 0xC0


#ifndef STARTING_INSTRUMENT
#define STARTING_INSTRUMENT instruments[40]
#endif

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
	int error = 0;
	int notes[CHANNEL_COUNT][NOTE_COUNT] = {0};
	double freqs[CHANNEL_COUNT][NOTE_COUNT] = {0.};
	int amps[CHANNEL_COUNT][NOTE_COUNT] = {0};
	double (*envp)[NOTE_COUNT] = calloc(CHANNEL_COUNT, sizeof(*envp));
	int sample_times[CHANNEL_COUNT][NOTE_COUNT] = {0}; // for envelope
	struct percussion *percussion_positions = calloc(NOTE_COUNT, sizeof(*percussion_positions));
	struct instrument *current_instruments = calloc(CHANNEL_COUNT, sizeof(*current_instruments));
	for (int i = 0; i < CHANNEL_COUNT; ++i) current_instruments[i] = STARTING_INSTRUMENT;
	current_instruments[9] = (struct instrument) {
		0,
		NULL,
		1.0,
		default_envelope,
	};
	

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
	while (1) {
		signed short buf[BUFSIZE];
		ssize_t buf_bytes = BUFSIZE*sizeof(signed short);
		ssize_t read_bytes = 0;
		
		poll(&pollfd, 1, 1);
		//if (pollfd.revents) fprintf(stderr, "revents: 0x%x (in: 0x%x)\n", pollfd.revents, POLLIN);
		while (pollfd.revents & (POLLIN | POLLHUP)) {
			//fprintf(stderr, "POLLIN\n\n");
			pollfd.revents = 0;
			unsigned char msg[8]; // note_on: 3 bytes, note_off: 3 bytes, program_change: 2 bytes
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
				int i;
				for(i = 0; i < NOTE_COUNT; ++i) {
					//fprintf(stderr, "%d: note #%d on %d at %d\n", __LINE__, i, freqs[i], amps[i]);
					if (vel == 0 && notes[channel][i] == note && sample_times[channel][i] > 0) {
						sample_times[channel][i] = -current_instruments[channel].envelope.release;
						amps[channel][i] *= envp[channel][i];
						//notes[channel][i] = freqs[channel][i] = amps[channel][i] = 0;
						break;
					}
					else if (vel > 0 && !notes[channel][i] && sample_times[channel][i] == 0) {
						sample_times[channel][i] = 1;
						notes[channel][i] = note;
						amps[channel][i] = vel * (8192 / 0xff);
						if (channel == 9) { // percussion
							freqs[channel][i] = note;
							percussion_positions[i] = percussions[note];
						}
						else {
							freqs[channel][i] = 440. * pow(2, (note - 69) / 12.);
						}
						break;
					}
				}
				if (i == NOTE_COUNT) { // no free note found
					
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
					if (notes[channel][i] == note && sample_times[channel][i] > 0) {
						sample_times[channel][i] = -current_instruments[channel].envelope.release;
						amps[channel][i] *= envp[channel][i];
						//fprintf(stderr, "%d (%d) off %d\n", note, freq, i);
						//notes[channel][i] = freqs[channel][i] = amps[channel][i] = 0;
						break;
					}
				}
			}
			else if ((msg[0]&MSG_MASK) == PROGRAM_CHANGE) {
				channel = msg[0] & CHANNEL_MASK;
				ret = read(STDIN_FILENO, msg, 1);
				if (ret != 1) goto finish;
				//fprintf(stderr, "program change on channel %d to %d\n\n\n\n", channel, msg[0]);
				if (channel != 9) { // Ignore program changes on percussion channel
					int inst = msg[0]; // program number
					if (instruments[inst].amplitudes != NULL) {
						current_instruments[channel] = instruments[inst];
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
				for (int j = 0; j < NOTE_COUNT; ++j) {
					if (!sample_times[channel][j]) {
						notes[channel][j] = freqs[channel][j] = amps[channel][j] = 0;
						continue;
					}
					// envelope
					#define current_envelope current_instruments[channel].envelope
					if (sample_times[channel][j] < 0) { // release
						//envp[channel][j] = current_envelope.sustain * sample_times[channel][j] / -current_envelope.release;
						envp[channel][j] = sample_times[channel][j] / (double) -current_envelope.release;
						// when note is released, amps is multiplied by envp to allow smoothness even if released before sustain
					} else if (sample_times[channel][j] <= current_envelope.attack) { // attack
						envp[channel][j] = sample_times[channel][j] / (double)current_envelope.attack;
					} else if (sample_times[channel][j] - current_envelope.attack < current_envelope.decay) { // decay
						double progress = (sample_times[channel][j] - current_envelope.attack) / (double) current_envelope.decay;
						envp[channel][j] = 1 - (1-current_envelope.sustain) * progress;
					} else {
						envp[channel][j] = current_envelope.sustain;
					}
					#undef current_envelope
					
					++sample_times[channel][j];
					if (!sample_times[channel][j]) {
						notes[channel][j] = freqs[channel][j] = amps[channel][j] = 0;
					}
					
					if (channel != 9) { // not percussion
						
						double wava = 0;
						for (int k = 0; k < current_instruments[channel].count; ++k) {
							wava += current_instruments[channel].amplitudes[k] * sin((k+1)*2.*M_PI*sample_dt*sample_index*freqs[channel][j]);
						}
						wava *= amps[channel][j] / current_instruments[channel].fullamplitude;
						
						wava *= envp[channel][j];
						
						wav += wava;
						//if (i%10 == 0) amps[channel][j] *= 0.9999999; // envelope
						//wav += amps[j] * sin(4.*M_PI*sample_dt*sample_index*freqs[j]) / 3.;
						//wav += amps[j] * sin(6.*M_PI*sample_dt*sample_index*freqs[j]) / 6.;
						//wav += amps[j] * sin(8.*M_PI*sample_dt*sample_index*freqs[j]) / 10.;
					}
					else { // percussion
						if (percussion_positions[j].end - percussion_positions[j].start < 1) {
							percussion_positions[j] = (struct percussion){NULL, NULL};
							notes[channel][j] = freqs[channel][j] = amps[channel][j] = 0;
							sample_times[channel][i] = -1;
						} else {
							wav += (*percussion_positions[j].start * amps[channel][j] / 32768) * envp[channel][j];
							percussion_positions[j].start += 2;
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
		
		if (1) {
			// print notes
			fprintf(stderr, "time: %lld\n", (long long int) time(NULL) - start_time);
			fprintf(stderr, "chnls:\x1b[0K");
			for (int i = 0; i < CHANNEL_COUNT; ++i)
				for (int j = 0; j < NOTE_COUNT; ++j)
					if (sample_times[i][j] > 0)
						fprintf(stderr, " (%2d,%1d)\x1b[0K", i, j);
			fprintf(stderr, "\n");
			fprintf(stderr, "freqs:\x1b[0K");
			for (int i = 0; i < CHANNEL_COUNT; ++i)
				for (int j = 0; j < NOTE_COUNT; ++j)
					if (sample_times[i][j] > 0)
						fprintf(stderr, freqs[i][j] <= 999. ? " %6.2f\x1b[0K" : " %6.1f\x1b[0K", freqs[i][j]);
			fprintf(stderr, "\n");
			fprintf(stderr, "amps: \x1b[0K");
			for (int i = 0; i < CHANNEL_COUNT; ++i)
				for (int j = 0; j < NOTE_COUNT; ++j)
					if (sample_times[i][j] > 0)
						fprintf(stderr, " %6d\x1b[0K", amps[i][j]);
			fprintf(stderr, "\n");
			fprintf(stderr, "envp: \x1b[0K");
			for (int i = 0; i < CHANNEL_COUNT; ++i)
				for (int j = 0; j < NOTE_COUNT; ++j)
					if (sample_times[i][j] > 0)
						fprintf(stderr,  " %6.4f\x1b[0K", envp[i][j]);
					//	fprintf(stderr, " %6d\x1b[0K", ampsum > 32767 ? (int)(amps[i][j] * (32767. / ampsum)) : amps[i][j]);
			fprintf(stderr, "\n\x1b[A\x1b[A\x1b[A\x1b[A\x1b[A");
	//		fprintf(stderr, "\n\x1b[A\x1b[A\x1b[A\x1b[A");
			fflush(stderr);
		}
		
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
