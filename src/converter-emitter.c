#include <gtk/gtk.h>

#include "converter-emitter.h"
/* refer: https://github.com/AlynxZhou/showmethekey */

struct _ConverterEmitter {
	GObject parent_instance;

	GSubprocess *cli;
	GDataInputStream *cli_out;
	GDataInputStream *cli_err;
	GThread *poller;
	gboolean polling;
	GError *error;

	GtkWindow *window;
	GtkTextBuffer *buffer;
};

G_DEFINE_TYPE(ConverterEmitter, converter_emitter, G_TYPE_OBJECT);

static gpointer poller_function(gpointer user_data)
{
        ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
        while (emitter->polling) {
		GError *read_out_line_error = NULL;
                GError *read_err_line_error = NULL;
                char *out_line = g_data_input_stream_read_line(
                        emitter->cli_out,
                        NULL,
                        NULL,
                        &read_out_line_error
                );

		char *err_line = g_data_input_stream_read_line(
			emitter->cli_err,
			NULL,
			NULL,
			&read_err_line_error
		);


                if (out_line == NULL && err_line == NULL) {
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
                        continue;
                }

		GtkTextIter end;
		gtk_text_buffer_get_iter_at_offset(emitter->buffer, &end, -1);
		if (out_line != NULL) {
			gtk_text_buffer_insert(
				emitter->buffer, &end, out_line, -1);
			gtk_text_buffer_insert(emitter->buffer, &end, "\n", -1);
			g_free(out_line);
			out_line = NULL;
		}

		if (err_line != NULL) {
			gtk_text_buffer_insert(
				emitter->buffer, &end, err_line, -1);
			gtk_text_buffer_insert(emitter->buffer, &end, "\n", -1);
			g_free(err_line);
			err_line = NULL;
		}
        }

        return NULL;
}

static void subprocess_finished(GObject *object,
                        	GAsyncResult *res,
                        	gpointer user_data)
{
	ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	emitter->polling = false;

	printf("subprocess finished\n");
}

static void converter_emitter_init(ConverterEmitter *emitter)
{
        emitter->error = NULL;
        emitter->cli = g_subprocess_new(
                G_SUBPROCESS_FLAGS_STDIN_PIPE |
                G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                G_SUBPROCESS_FLAGS_STDERR_PIPE,
                &emitter->error,
		"ffmpeg",
                "-aaa",
                NULL
        );

        if (emitter->cli == NULL) {
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

        emitter->polling = TRUE;
        emitter->poller = g_thread_try_new(
                "poller",
                poller_function,
                emitter,
                &emitter->error
        );

        if (emitter->poller == NULL) {
                return;
        }

	/* init emitter window */
	emitter->window = GTK_WINDOW(gtk_window_new());
	if (emitter->window == NULL) {
		return;
	}
	gtk_window_set_title(GTK_WINDOW(emitter->window), "Merge Videos");
        gtk_window_set_default_size(GTK_WINDOW(emitter->window), 800, 600);
        gtk_window_set_transient_for(emitter->window, NULL);
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

void converter_emitter_win_init(ConverterEmitter* emitter)
{
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_margin_start(vbox, 8);
        gtk_widget_set_margin_end(vbox, 8);
        gtk_widget_set_margin_top(vbox, 8);
        gtk_widget_set_margin_bottom(vbox, 8);
	gtk_window_set_child(GTK_WINDOW(emitter->window), vbox);

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
		G_OBJECT(emitter->window),
		"destroy",
      		G_CALLBACK(converter_emitter_win_close),
		emitter
	);

	gtk_window_present(GTK_WINDOW(emitter->window));
}
