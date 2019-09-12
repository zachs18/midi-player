#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#include <pthread.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "keytodouble.h"



#define NOTE_COUNT 32
#define CHANNEL_COUNT 16

#define debug(d) fprintf(stderr, "%s:%s:%d: %d\n", __FILE__, __func__, __LINE__, d)


static void activate(GtkApplication *app, gpointer user_data);
static gboolean keypress(GtkWindow *window, GdkEventKey *event, gpointer user_data);

void print_notes(volatile double *notes);

struct simple_midi_event {
	struct timespec wait;
	struct simple_midi_event *next;
	uint_fast8_t note;
	uint_fast8_t channel;
	uint_fast8_t velocity;
	bool first;
};


/*
 * If =0, do not record, else, record for that channel
 */
int record_channel = 0;
struct simple_midi_event loop_events[CHANNEL_COUNT]; // The first event for each channel (starts at 1)
struct simple_midi_event *last_loop_event[CHANNEL_COUNT];

pthread_mutex_t note_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {
	GtkApplication *app;
	GtkWindow *window;
//	GdkWindow *gdk_window;
	int status;
	
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		last_loop_event[i] = loop_events + i;
	}

	volatile double notes[NOTE_COUNT] = {0};
	app = gtk_application_new("com.github.zachs18.midi_keyboard_loop", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), (gpointer) notes);
	fprintf(stderr, "\n\n");

//	window = gtk_application_window_new(app);
//	window = gtk_application_get_active_window(app);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	fprintf(stderr, "\n");

	return status;
}

static void activate(GtkApplication *app, gpointer user_data) {
	GtkWidget *window;

	window = gtk_application_window_new(app);

	g_signal_connect(window, "key-press-event", G_CALLBACK(keypress), user_data);
	g_signal_connect(window, "key-release-event", G_CALLBACK(keypress), user_data);
	gtk_window_set_title(GTK_WINDOW(window), "Window");
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_widget_show_all(window);
}

static gboolean keypress(GtkWindow *window, GdkEventKey *event, gpointer notes_) {
	volatile double *notes = (volatile double*)notes_;
	GdkEventType type = event->type;
	guint keyval = event->keyval;
	//fprintf(stderr, "%u: %f\n", keyval, 0 <= keyval && keyval <= max_key ? note_values[keyval] : -1.0);
	//fprintf(stderr, "%u:", keyval);//, 0 <= keyval && keyval <= max_key ? note_values[keyval] : -1.0);
	fflush(stdout);
	if (0 <= keyval && keyval <= max_key && note_values[keyval] != 0.0) {
		if (type == GDK_KEY_PRESS) {
			pthread_mutex_lock(&note_mutex);
			for (int i = 0; i < NOTE_COUNT; ++i) {
				if (notes[i] == note_values[keyval]) {
					pthread_mutex_unlock(&note_mutex);
					return FALSE;
				}
			}
			for (int i = 0; i < NOTE_COUNT; ++i) {
				if (notes[i] == 0.0) {
					notes[i] = note_values[keyval];
					putchar(0x90); // MIDI Note On Channel 1
					putchar(note_midi_values[keyval]); // MIDI Note value
					putchar(0x31); // Velocity
					fflush(stdout);
					pthread_mutex_unlock(&note_mutex);
					break;
				}
			}
			print_notes(notes);
			return FALSE;
			//fprintf(stderr, "Pressed: ");
		}
		else {
			for (int i = 0; i < NOTE_COUNT; ++i) {
				if (notes[i] == note_values[keyval]) {
					notes[i] = 0.0;
					pthread_mutex_lock(&note_mutex);
					putchar(0x90); // MIDI Note On Channel 1
					putchar(note_midi_values[keyval]); // MIDI Note value
					putchar(0); // Velocity off
					pthread_mutex_unlock(&note_mutex);
					fflush(stdout);
					break;
				}
			}
			print_notes(notes);
			return FALSE;
			//fprintf(stderr, "Released: ");
		}
	}
	else if (keyval == GDK_KEY_Escape) {
		if (type == GDK_KEY_PRESS) {
			fprintf(stderr, "Escape: Quitting\n");
			exit(0);
		}
	}
	else if (GDK_KEY_KP_1 <= keyval && keyval <= GDK_KEY_KP_9) { // set record_channel
		int channel = keyval - GDK_KEY_KP_1 + 1;
		if (type == GDK_KEY_PRESS) {
			if (record_channel == 0) record_channel = channel;
		}
		else {
			if (record_channel == channel) record_channel = 0;
		}
	}
	return FALSE;
	//printf("%s: ", gdk_keyval_name(keyval));
	//fprintf(stderr, "%f\n", (0 <= keyval && keyval <= max_key) ? note_values[keyval] : -1.0);
}

void print_notes(volatile double *notes) {
	
	fprintf(stderr, "\x1b[A\x1b[2K"); // Erase lines
	fprintf(stderr, "\x1b[A\x1b[2K"); // Erase lines
	fprintf(stderr, "\x1b[A\x1b[2K"); // Erase lines
	fprintf(stderr, "Notes playing: ");
	for (int i = 0; i < NOTE_COUNT; ++i) {
		if (notes[i] != 0.0) {
			fprintf(stderr, notes[i] >= 999.994 ? "%6.1f " : "%6.2f ", notes[i]);
		}
	}
	fprintf(stderr, "\nIndex        : ");
	for (int i = 0; i < NOTE_COUNT; ++i) {
		if (notes[i] != 0.0) {
			fprintf(stderr, "%6d ", i);
		}
	}
	fprintf(stderr, "\nRecording to channel: %d", record_channel);
	fprintf(stderr, "\n");
}
	
void start_recording(void) {
	if (!record_channel) return;
}
	

void add_event(int midi_value, bool on) {
	if (!record_channel) return;
}