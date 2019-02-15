#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "keytodouble.h"



#define NOTE_COUNT 16

#define debug(d) fprintf(stderr, "%s:%s:%d: %d\n", __FILE__, __func__, __LINE__, d)

static void activate(GtkApplication *app, gpointer user_data);
static gboolean keypress(GtkWindow *window, GdkEventKey *event, gpointer user_data);

void print_notes(volatile double *notes);

int main(int argc, char **argv) {
	GtkApplication *app;
	GtkWindow *window;
//	GdkWindow *gdk_window;
	int status;

	volatile double notes[NOTE_COUNT] = {0};
	app = gtk_application_new("com.github.zachs18.midi_keyboard", G_APPLICATION_FLAGS_NONE);
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
			for (int i = 0; i < NOTE_COUNT; ++i) {
				if (notes[i] == note_values[keyval])
					return FALSE;
			}
			for (int i = 0; i < NOTE_COUNT; ++i) {
				if (notes[i] == 0.0) {
					notes[i] = note_values[keyval];
					putchar(0x90); // MIDI Note On Channel 1
					putchar(note_midi_values[keyval]); // MIDI Note value
					putchar(0x31); // Velocity
					fflush(stdout);
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
					putchar(0x90); // MIDI Note On Channel 1
					putchar(note_midi_values[keyval]); // MIDI Note value
					putchar(0); // Velocity off
					fflush(stdout);
					break;
				}
			}
			print_notes(notes);
			return FALSE;
			//fprintf(stderr, "Released: ");
		}
	}
	else {
		if (type == GDK_KEY_PRESS) {
			if (keyval == GDK_KEY_Escape) {
				fprintf(stderr, "Escape: Quitting\n");
				exit(0);
			}
		}
	}
	//printf("%s: ", gdk_keyval_name(keyval));
	//fprintf(stderr, "%f\n", (0 <= keyval && keyval <= max_key) ? note_values[keyval] : -1.0);
}

void print_notes(volatile double *notes) {
	
	fprintf(stderr, "\x1b[A\x1b[2K\x1b[A\x1b[2K"); // Erase lines
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
	fprintf(stderr, "\n");
}