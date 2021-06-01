#include <gtk/gtk.h>

#include "converter-emitter.h"
/* refer: https://github.com/AlynxZhou/showmethekey */

struct _ConverterEmitter {
	GObject parent_instance;
	GSubprocess *cli;
	GDataInputStream *cli_out;
	GThread *poller;
	gboolean polling;
	GError *error;
};

G_DEFINE_TYPE(ConverterEmitter, converter_emitter, G_TYPE_OBJECT);

static gpointer poller_function(gpointer user_data)
{
        ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
        while (emitter->polling) {
                // printf("pooling.\n");
                GError *read_line_error = NULL;
                char *line = g_data_input_stream_read_line(
                        emitter->cli_out,
                        NULL,
                        NULL,
                        &read_line_error
                );

                if (line == NULL) {
                        if (read_line_error != NULL) {
                                fprintf(stderr, "Read line error: %s\n",
                                                read_line_error->message);
                                g_error_free(read_line_error);
                        }
                        continue;
                }

                // 还没想好咋处理接受到的消息，先丢到stdout再说
                printf("%s\n", line);
                g_free(line);
        }

        return NULL;
}

static void command_async_ready(GObject *object,
                        	GAsyncResult *res,
                        	gpointer user_data)
{
	ConverterEmitter *emitter = CONVERTER_EMITTER(user_data);
	GError *read_line_error = NULL;
	char *line = g_data_input_stream_read_line(
		emitter->cli_out,
		NULL,
		NULL,
		&read_line_error
	);

	printf("Command output: %s\n", line);
}

static void converter_emitter_init(ConverterEmitter *emitter)
{
        emitter->error = NULL;
        emitter->cli = g_subprocess_new(
                G_SUBPROCESS_FLAGS_STDIN_PIPE |
                G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                G_SUBPROCESS_FLAGS_STDERR_PIPE,
                &emitter->error,
                // "pkexec",
		"ffmpeg",
                "-version",
                NULL
        );

        if (emitter->cli == NULL) {
                return;
        }

        // spawn a subprocess, execute ffmpeg command.
        g_subprocess_wait_async(emitter->cli,
		NULL,				// cancellable
		NULL,		// callback
		NULL);				// user_data
        emitter->cli_out = g_data_input_stream_new(
                g_subprocess_get_stdout_pipe(emitter->cli)
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
}

// static void converter_emitter_finalize(GObject *object)
// {
//         ConverterEmitter *emitter = CONVERTER_EMITTER_TYPE(user_data);
//
//         G_OBJECT_CLASS(converter_emitter_parent_class)->finalize(object);
// }

static void converter_emitter_class_init(ConverterEmitterClass *emitter)
{
        printf("merge class init\n");
        // GObjectClass *object_class = G_OBJECT_CLASS(emitter);
        // object_class->finalize = converter_emitter_finalize;
}

ConverterEmitter *converter_emitter_new(void)
{
        ConverterEmitter *emitter = g_object_new(CONVERTER_EMITTER_TYPE, NULL);
        return emitter;
}
