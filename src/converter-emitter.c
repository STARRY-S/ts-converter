#include <gtk/gtk.h>

#include "converter-emitter.h"
/* refer: https://github.com/AlynxZhou/showmethekey */

#ifndef MAX_LINE
#define MAX_LINE 128
#endif

struct _ConverterEmitter {
	GObject parent_instance;

	GSubprocess *cli;
	GDataInputStream *cli_out;
	GDataInputStream *cli_err;
	GPtrArray *line_array;
	GThread *poller;
	gboolean running;
	GError *error;

	GtkTextBuffer *buffer;
};

G_DEFINE_TYPE(ConverterEmitter, converter_emitter, G_TYPE_OBJECT);

enum {
	CONVERTER_EMITTER_UPDATE_TEXT_VIEW,
	N_SIGNALS
};

static gboolean idle_function(gpointer user_data)
{
	ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	char *cliout = emitter->cli_out_line;
	char *clierr = emitter->cli_err_line;

	GtkTextIter end;
	gtk_text_buffer_get_iter_at_offset(emitter->buffer, &end, -1);
	if (cliout != NULL) {
		gtk_text_buffer_insert(
			emitter->buffer, &end, cliout, -1);
		gtk_text_buffer_insert(emitter->buffer, &end, "\n", -1);
		g_free(cliout);
	}

	if (clierr != NULL) {
		gtk_text_buffer_insert(
			emitter->buffer, &end, "[ERROR] ", -1);
		gtk_text_buffer_insert(
			emitter->buffer, &end, clierr, -1);
		gtk_text_buffer_insert(emitter->buffer, &end, "\n", -1);
		g_free(clierr);
	}
}

// See https://stackoverflow.com/questions/37780799/
// gtk-3-textview-application-crashes
// You can not access any part of GTK+ in a seperate process
static gpointer poller_function(gpointer user_data)
{
        ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);

        while (TRUE) {
		GError *read_out_line_error = NULL;
                GError *read_err_line_error = NULL;
                emitter->cli_out_line = g_data_input_stream_read_line(
                        emitter->cli_out,
                        NULL,
                        NULL,
                        &read_out_line_error
                );

		emitter->cli_err_line = g_data_input_stream_read_line(
			emitter->cli_err,
			NULL,
			NULL,
			&read_err_line_error
		);

                if (!emitter->cli_out_line && !emitter->cli_err_line) {
                        if (read_out_line_error != NULL) {
                                fprintf(stderr, "Read line error: %s\n",
                                                read_out_line_error->message);
                                g_error_free(read_out_line_error);
                        }

			if (read_err_line_error != NULL) {
                                fprintf(stderr, "Read line error: %s\n",
                                                read_err_line_error->message);
                                g_error_free(read_err_line_error);
                        }
			// When the subprocess finished and output buffer is
			// clean, stop polling thread.
			if (!emitter->running) {
				break;
			}
                        continue;
                }

		// g_idle_add();
		g_timeout_add_full(
			G_PRIORITY_DEFAULT,
			0,
			idle_function,
			emitter,
			NULL
		);
        }

        return NULL;
}

static void subprocess_finished(GObject *object,
                        	GAsyncResult *res,
                        	gpointer user_data)
{
	ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	emitter->running = false;

	printf("subprocess finished\n");
}

static void converter_emitter_init(ConverterEmitter *emitter)
{
	/* lock the poller thread before we initialize the window */
	emitter->buffer = NULL;
	// 好麻烦啊
	// g_strjoinv() accepts a NULL terminated char pointer array,
	// so we use a GPtrArray to store char pointer.
	// Append a NULL first and always insert elements before it.
	// So we can directly use the GPtrArray for g_strjoinv().
	emitter->line_array = g_ptr_array_new_full(MAX_LINE + 1, g_free);

	emitter->error = NULL;
        emitter->cli = g_subprocess_new(
                G_SUBPROCESS_FLAGS_STDIN_PIPE |
                G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                G_SUBPROCESS_FLAGS_STDERR_PIPE,
                &emitter->error,
		"ffmpeg",
                "-version",
                NULL
        );

        if (emitter->cli == NULL) {
		fprintf(stderr, "Failed to run ffmpeg command.\n");
		fprintf(stderr, "Please make sure ffmpeg is installed.\n");
                return;
        }

        // spawn a subprocess, execute ffmpeg command.
        g_subprocess_wait_async(emitter->cli,
		NULL,				// cancellable
		subprocess_finished,		// callback
		emitter				// user_data
	);

        emitter->cli_out = g_data_input_stream_new(
                g_subprocess_get_stdout_pipe(emitter->cli)
        );

	emitter->cli_err = g_data_input_stream_new(
                g_subprocess_get_stderr_pipe(emitter->cli)
        );

        emitter->running = TRUE;

}

static void converter_emitter_finalize(GObject *object)
{
        // ConverterEmitter *emitter = CONVERTER_EMITTER_TYPE(object);

        G_OBJECT_CLASS(converter_emitter_parent_class)->finalize(object);
	printf("emitter finalized!\n");
	fflush(stdout);
}

static void converter_emitter_class_init(ConverterEmitterClass *emitter)
{
        GObjectClass *object_class = G_OBJECT_CLASS(emitter);
        object_class->finalize = converter_emitter_finalize;

	obj_signals[CONVERTER_EMITTER_UPDATE_TEXT_VIEW] = g_signal_new(
		"update-textview",
		CONVERTER_EMITTER,
		G_SIGNAL_RUN_LAST,
		0, NULL, NULL,
		g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING
	);
}

static void converter_emitter_win_close(GtkWidget *object, gpointer emitter)
{
	g_object_unref(emitter);
}

ConverterEmitter *converter_emitter_new(void)
{
        ConverterEmitter *emitter = g_object_new(CONVERTER_EMITTER_TYPE, NULL);
        return emitter;
}

void converter_emitter_poller_init(ConverterEmitter *emitter)
{
	emitter->poller = NULL;
        emitter->poller = g_thread_try_new(
                "poller",
                poller_function,
                emitter,
                &emitter->error
        );

        if (emitter->poller == NULL) {
                return;
        }
}

void converter_emitter_win_init(ConverterEmitter* emitter, GtkWindow *parent)
{
	/* init emitter window */
	GtkWindow *window = GTK_WINDOW(gtk_window_new());
	if (window == NULL) {
		return;
	}
	gtk_window_set_title(GTK_WINDOW(window), "Merge Videos");
        gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
	gtk_window_set_transient_for(window, parent);

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_margin_start(vbox, 8);
        gtk_widget_set_margin_end(vbox, 8);
        gtk_widget_set_margin_top(vbox, 8);
        gtk_widget_set_margin_bottom(vbox, 8);
	gtk_window_set_child(GTK_WINDOW(window), vbox);

	GtkWidget *text_view = gtk_text_view_new();
        GtkWidget *text_scroll = gtk_scrolled_window_new();
        gtk_widget_set_hexpand(text_view, TRUE);
        gtk_widget_set_vexpand(text_view, TRUE);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
        gtk_text_view_set_cursor_visible(
		GTK_TEXT_VIEW(text_view),
		FALSE
	);
        gtk_scrolled_window_set_child(
                GTK_SCROLLED_WINDOW(text_scroll),
                text_view
        );
        gtk_box_append(GTK_BOX(vbox), text_scroll);

	emitter->buffer = gtk_text_view_get_buffer(
		GTK_TEXT_VIEW(text_view)
	);

	/* release emitter when window destroy */
	g_signal_connect(
		G_OBJECT(window),
		"destroy",
      		G_CALLBACK(converter_emitter_win_close),
		emitter
	);

	gtk_window_present(GTK_WINDOW(window));
}
