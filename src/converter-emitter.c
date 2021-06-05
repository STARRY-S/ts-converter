#include <gtk/gtk.h>

#include "converter-emitter.h"

#define MAX_LINE 128

/* refer: https://github.com/AlynxZhou/showmethekey */
struct _ConverterEmitter {
	GObject parent_instance;

	GSubprocess *cli;
	GDataInputStream *cliout;
	GDataInputStream *clierr;
	GPtrArray *line_array;
	GThread *poller;
	gboolean running;
	GError *error;

	GtkWindow *window;
	GtkTextBuffer *buffer;
};

G_DEFINE_TYPE(ConverterEmitter, converter_emitter, G_TYPE_OBJECT);

enum {
	CONVERTER_EMITTER_UPDATE_TEXT_VIEW,
	N_SIGNALS
};

static guint obj_signals[N_SIGNALS] = { 0 };

static gboolean idle_function(gpointer user_data)
{
	ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	gchar *log_text = g_strjoinv(
		"\n",
		(gchar**) emitter->line_array->pdata
	);
	// remove the lines which is inserted in the log.
	g_ptr_array_remove_range(
		emitter->line_array,
		0,
		emitter->line_array->len - 1
	);

	GtkTextIter end;
	gtk_text_buffer_get_iter_at_offset(emitter->buffer, &end, -1);
	gtk_text_buffer_insert(
		emitter->buffer, &end, log_text, -1
	);
	g_free(log_text);

	return FALSE;
}

static gpointer poller_function(gpointer user_data)
{
        ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	while (emitter->buffer == NULL) {
		continue;
	}

        while (TRUE) {
		GError *read_out_line_error = NULL;
                GError *read_err_line_error = NULL;

                char *cliout = g_data_input_stream_read_line(
                        emitter->cliout,
                        NULL,
                        NULL,
                        &read_out_line_error
                );

		char *clierr = g_data_input_stream_read_line(
			emitter->clierr,
			NULL,
			NULL,
			&read_err_line_error
		);

                if (cliout == NULL && clierr == NULL) {
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

		if (cliout != NULL) {
			printf("%s\n", cliout);
			g_ptr_array_insert(
				emitter->line_array,
				emitter->line_array->len - 1,
				cliout
			);
		}

		if (clierr != NULL) {
			printf("%s\n", clierr);
			g_ptr_array_insert(
				emitter->line_array,
				emitter->line_array->len - 1,
				clierr
			);
		}

		// Do not access any part of GTK+ in a seperate process
		if (cliout != NULL || clierr != NULL) {
			// if (emitter->line_array->len - 1 > MAX_LINE) {
			// 	g_ptr_array_remove_range(
			// 		emitter->line_array,
			// 		0,
			// 		emitter->line_array->len - 1 - MAX_LINE
			// 	);
			// }

			g_timeout_add_full(
				G_PRIORITY_DEFAULT,
				0,
				idle_function,
				g_object_ref(emitter),
				NULL
			);
		}
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
	emitter->window = NULL;
	emitter->buffer = NULL;

	// g_strjoinv() accepts a NULL terminated char pointer array,
	// so we use a GPtrArray to store char pointer.
	// Append a NULL first and always insert elements before it.
	// So we can directly use the GPtrArray for g_strjoinv().
	emitter->line_array = g_ptr_array_new_full(MAX_LINE + 1, g_free);
	g_ptr_array_add(emitter->line_array, NULL);

	emitter->error = NULL;
	// refer: https://trac.ffmpeg.org/wiki/Concatenate
        emitter->cli = g_subprocess_new(
                G_SUBPROCESS_FLAGS_STDIN_PIPE |
                G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                G_SUBPROCESS_FLAGS_STDERR_PIPE,
                &emitter->error,
		"ffmpeg",
                "-f", "concat", "-safe", "0", "-i", "temp.txt", "-c", "copy",
		"output.mp4",
                NULL
        );

        if (emitter->cli == NULL) {
		fprintf(stderr, "Failed to run ffmpeg command.\n");
		fprintf(stderr, "Please make sure ffmpeg is installed.\n");
                return;
        }

        // spawn a subprocess, execute ffmpeg command.
        g_subprocess_wait_async(
		emitter->cli,
		NULL,				// cancellable
		subprocess_finished,		// callback
		emitter				// user_data
	);

        emitter->cliout = g_data_input_stream_new(
                g_subprocess_get_stdout_pipe(emitter->cli)
        );

	emitter->clierr = g_data_input_stream_new(
                g_subprocess_get_stderr_pipe(emitter->cli)
        );

        emitter->running = TRUE;
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

static void converter_emitter_finalize(GObject *object)
{
        ConverterEmitter *emitter = CONVERTER_EMITTER(object);

	if (emitter->line_array != NULL) {
		g_ptr_array_free(emitter->line_array, TRUE);
		emitter->line_array = NULL;
	}
	G_OBJECT_CLASS(converter_emitter_parent_class)->finalize(object);
}

static void converter_emitter_class_init(ConverterEmitterClass *emitter)
{
        GObjectClass *object_class = G_OBJECT_CLASS(emitter);
        object_class->finalize = converter_emitter_finalize;

	obj_signals[CONVERTER_EMITTER_UPDATE_TEXT_VIEW] = g_signal_new(
		"update-textview",
		CONVERTER_EMITTER_TYPE,
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

}

void converter_emitter_win_init(ConverterEmitter* emitter, GtkWindow *parent)
{
	/* init emitter window */
	GtkWindow *window = GTK_WINDOW(gtk_window_new());
	if (window == NULL) {
		return;
	}
	emitter->window = window;
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
