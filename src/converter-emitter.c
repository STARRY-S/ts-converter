#include <gtk/gtk.h>

#include "converter-emitter.h"

#define BUFF_SIZE 512

/* refer: https://github.com/AlynxZhou/showmethekey */
struct _ConverterEmitter {
	GObject parent_instance;

	GSubprocess *cli;
	GInputStream *cliout;
	GInputStream *clierr;

	GThread *poller;
	gboolean running;
	GPtrArray *line_array;

	GtkWindow *window;
	GtkTextBuffer *buffer;
};

G_DEFINE_TYPE(ConverterEmitter, converter_emitter, G_TYPE_OBJECT);

enum {
	CONVERTER_EMITTER_UPDATE_TEXT_VIEW,
	N_SIGNALS
};

static guint obj_signals[N_SIGNALS] = { 0 };

static void idle_destroy_function(gpointer user_data)
{
	ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	g_object_unref(emitter);
}

static gboolean idle_function(gpointer user_data)
{
	ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	gchar *log_text = g_strjoinv(
		"",
		(gchar**) emitter->line_array->pdata
	);

	g_signal_emit_by_name(emitter, "update-log", log_text);
	g_free(log_text);

	return FALSE;
}

static gpointer poller_function(gpointer user_data)
{
        ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	char outbuff[BUFF_SIZE + 1];
	char errbuff[BUFF_SIZE + 1];

	// Wait until emitter text buffer is initialized
	while (emitter->buffer == NULL && emitter->running) {
		continue;
	}

        while (TRUE) {
		GError *read_out_line_error = NULL;
                GError *read_err_line_error = NULL;
		// printf("poll\n");
		int out_num = g_input_stream_read(
			emitter->cliout,
			outbuff,
			BUFF_SIZE,
			NULL,
			&read_out_line_error
		);
		outbuff[out_num] = '\0';

		int err_num = g_input_stream_read(
			emitter->clierr,
			errbuff,
			BUFF_SIZE,
			NULL,
			&read_err_line_error
		);
		errbuff[err_num] = '\0';

		if (out_num > 0) {
			printf("%s", outbuff);
			g_ptr_array_insert(
				emitter->line_array,
				emitter->line_array->len - 1,
				g_strdup(outbuff)
			);
		}

		if (err_num > 0) {
			printf("%s", errbuff);
			g_ptr_array_insert(
				emitter->line_array,
				emitter->line_array->len - 1,
				g_strdup(errbuff)
			);
		}

		if (out_num == 0 && err_num == 0) {
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
                } else {
			g_timeout_add_full(
				G_PRIORITY_DEFAULT,
				0,
				idle_function,
				g_object_ref(emitter),
				idle_destroy_function
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
	printf("subprocess finished!\n");
}

static void converter_emitter_init(ConverterEmitter *emitter)
{
	emitter->window = NULL;
	emitter->buffer = NULL;
	emitter->cliout = NULL;
	emitter->clierr = NULL;

	// g_strjoinv() accepts a NULL terminated char pointer array,
	// so we use a GPtrArray to store char pointer.
	// Append a NULL first and always insert elements before it.
	// So we can directly use the GPtrArray for g_strjoinv().
	emitter->line_array = g_ptr_array_new_with_free_func(g_free);
	g_ptr_array_add(emitter->line_array, NULL);
}

static void converter_emitter_finalize(GObject *object)
{
        ConverterEmitter *emitter = CONVERTER_EMITTER(object);

	if (emitter->line_array != NULL && emitter->line_array->len > 0) {
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
		"update-log",
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

static void converter_emitter_update_log(ConverterEmitter *emitter,
	char* log_text, gpointer data)
{
	// GtkTextIter end;
	// gtk_text_buffer_get_iter_at_offset(emitter->buffer, &end, -1);
	// gtk_text_buffer_insert(
	// 	emitter->buffer, &end, log_text, -1
	// );
	gtk_text_buffer_set_text(emitter->buffer, log_text, -1);
}

ConverterEmitter *converter_emitter_new(void)
{
        ConverterEmitter *emitter = g_object_new(CONVERTER_EMITTER_TYPE, NULL);
        return emitter;
}

void converter_emitter_start_async(ConverterEmitter *emitter, GError **error)
{
	g_return_if_fail(emitter != NULL);

	// refer: https://trac.ffmpeg.org/wiki/Concatenate
	// ffmpeg -i ./0010.mp4 -c:v libx264 -c:a aac test.mp4
	// spawn a subprocess, execute ffmpeg command.
	// ffmpeg -f concat -safe 0 -i temp.txt -c copy output.mp4
	emitter->cli = g_subprocess_new(
                // G_SUBPROCESS_FLAGS_STDIN_PIPE |
                G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                G_SUBPROCESS_FLAGS_STDERR_PIPE,
                error,
		// "stdbuf", "-o0",
		"ffmpeg", "-nostdin", "-y",
		"-f", "concat", "-safe", "0", "-i", "temp.txt", "-c", "copy",
		// "-i", "./0010.mp4", "-c:v", "libx264", "-c:a", "aac", "test.mp4",
		"output.mp4",
		// "./a.out",
		// "-version",
                NULL
        );

        if (emitter->cli == NULL) {
		fprintf(stderr, "Failed to run ffmpeg command.\n");
		fprintf(stderr, "Please make sure ffmpeg is installed.\n");
                return;
        }

        g_subprocess_wait_async(
		emitter->cli,
		NULL,				// cancellable
		subprocess_finished,		// callback
		emitter				// user_data
	);

	emitter->cliout = g_subprocess_get_stdout_pipe(emitter->cli);
	emitter->clierr = g_subprocess_get_stderr_pipe(emitter->cli);

        emitter->running = TRUE;
        emitter->poller = g_thread_try_new(
                "poller",
                poller_function,
                emitter,
                error
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
	gtk_text_buffer_set_text(
		emitter->buffer, "Loading...\nPlease wait.", -1
	);

	/* release emitter when window destroy */
	g_signal_connect(
		G_OBJECT(window),
		"destroy",
      		G_CALLBACK(converter_emitter_win_close),
		emitter
	);

	g_signal_connect(
		G_OBJECT(emitter),
		"update-log",
		G_CALLBACK(converter_emitter_update_log),
		NULL
	);

	gtk_window_present(GTK_WINDOW(window));
}
