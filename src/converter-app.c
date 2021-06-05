#include <gtk/gtk.h>
#include <stdio.h>

#include "converter-app.h"
#include "converter-app-win.h"
#include "converter-filelist.h"
#include "converter-emitter.h"

struct _ConverterApp
{
        GtkApplication parent;
        ConverterAppWindow *window;
};

G_DEFINE_TYPE(ConverterApp, converter_app, GTK_TYPE_APPLICATION);

static void converter_app_init(ConverterApp *app)
{
        init_file_list();
}

static void converter_app_activate(GApplication *app)
{
        CONVERTER_APP(app)->window =
                converter_app_window_new(CONVERTER_APP(app));

        /* finish & show */
        converter_app_window_open(CONVERTER_APP(app)->window);
        gtk_window_present(GTK_WINDOW(CONVERTER_APP(app)->window));
}

void converter_app_open(GApplication *app,
                        GFile        **files,
                        int          argc,
                        const char   *argv)
{
        GList *window;
        ConverterAppWindow *win;

        window = gtk_application_get_windows(GTK_APPLICATION(app));
        if (window) {
                win = CONVERTER_APP_WINDOW(window->data);
        } else {
                win = converter_app_window_new(CONVERTER_APP(app));
        }

        gtk_window_present(GTK_WINDOW(win));
}

ConverterApp *converter_app_new(void)
{
        return g_object_new(
                CONVERTER_APP_TYPE,
                "application-id",
                "me.starry-s.converter",
                "flags",
                G_APPLICATION_HANDLES_OPEN,
                NULL
        );
}

static void on_cancel_response(GtkNativeDialog *dialog, int response)
{
        if (response != GTK_RESPONSE_CANCEL) {
                return;
        }

        g_object_unref(dialog);
}

static void on_open_response(GtkNativeDialog *dialog,
        int response, gpointer window)
{
        if (response != GTK_RESPONSE_ACCEPT) {
                return;
        }

        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        GListModel* files = gtk_file_chooser_get_files(chooser);

        int i = 0;
        GFile *f = NULL;
        while ((f = g_list_model_get_item(files, i++)) != NULL) {
                char *basename = g_file_get_basename(f);
                char *pathname = g_file_get_path(f);
                // open_file(file);
                insert_sort(pathname, basename, default_str_is_larger);
        }

        calculate_file_id();

#ifndef NDEBUG
        print_file_list();
        printf("\n");
#endif

        converter_app_window_update_list(window);
        g_object_unref(dialog);
}

static void clear_list_activated(GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer      app)
{
        ConverterAppWindow *window = CONVERTER_APP(app)->window;
        release_file_list();
        init_file_list();
        converter_app_window_update_list(window);
}

static void open_file_activated(GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer      app)
{
        ConverterAppWindow *window = CONVERTER_APP(app)->window;
        GtkFileChooserAction open_action = GTK_FILE_CHOOSER_ACTION_OPEN;
        GtkFileChooserNative *native;

        native = gtk_file_chooser_native_new(
                "Open File",
                GTK_WINDOW(window),
                open_action,
                "_Open",
                "_Cancel"
        );
        g_object_set_data_full(
                G_OBJECT(native),
                "app",
                g_object_ref(app),
                g_object_unref
        );

        g_signal_connect(
                native,
                "response",
                G_CALLBACK(on_cancel_response),
                NULL
        );

        g_signal_connect(
                native,
                "response",
                G_CALLBACK(on_open_response),
                window
        );

        gtk_file_chooser_set_select_multiple(
                GTK_FILE_CHOOSER(native),
                TRUE
        );

        gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

static void on_save_response(GtkFileChooserNative *dialog, int response)
{
        GFile *file = NULL;
        if (response == GTK_RESPONSE_ACCEPT)
        {
                GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
                file = gtk_file_chooser_get_file(chooser);
                gtk_window_destroy(GTK_WINDOW (dialog));
        } else {
                gtk_window_destroy(GTK_WINDOW (dialog));
        }

        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_margin_start(vbox, 8);
        gtk_widget_set_margin_end(vbox, 8);
        gtk_widget_set_margin_top(vbox, 8);
        gtk_widget_set_margin_bottom(vbox, 8);

        GtkWidget *text_scroll = gtk_scrolled_window_new();
        GtkWidget *text_view = gtk_text_view_new();
        gtk_widget_set_hexpand(text_view, TRUE);
        gtk_widget_set_vexpand(text_view, TRUE);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
        gtk_scrolled_window_set_child(
                GTK_SCROLLED_WINDOW(text_scroll),
                text_view
        );
        gtk_box_append(GTK_BOX(vbox), text_scroll);

        GtkWindow *mwindow;
        mwindow = GTK_WINDOW(gtk_window_new());
        gtk_window_set_title(GTK_WINDOW(mwindow), "Merge Videos");
        gtk_window_set_child(GTK_WINDOW(mwindow), vbox);
        gtk_window_set_default_size(GTK_WINDOW(mwindow), 800, 600);
        gtk_window_set_transient_for(mwindow, NULL);
        gtk_window_present(GTK_WINDOW(mwindow));

        GtkTextBuffer *buffer;
        char *path = g_file_get_path(file);
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_set_text(buffer, path, strlen(path));

        ConverterEmitter *emitter = converter_emitter_new();
}

static void merge_activated(GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer      app)
{
        /* file picker */
        ConverterAppWindow *window = CONVERTER_APP(app)->window;
        GtkFileChooserAction save_action = GTK_FILE_CHOOSER_ACTION_SAVE;
        GtkFileChooserNative *native = gtk_file_chooser_native_new(
                "Merge Video",
                GTK_WINDOW(window),
                save_action,
                "_Save",
                "_Cancel"
        );
        g_object_set_data_full(
                G_OBJECT(native),
                "app",
                g_object_ref(app),
                g_object_unref
        );

        g_signal_connect(
                native,
                "response",
                G_CALLBACK(on_cancel_response),
                NULL
        );

        g_signal_connect(
                native,
                "response",
                G_CALLBACK(on_save_response),
                window
        );

        gtk_file_chooser_set_current_name(
                GTK_FILE_CHOOSER(native),
                "output.mp4"
        );

        gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

static void quit_activated(GSimpleAction *action,
                           GVariant      *parameter,
                           gpointer      app)
{
        release_file_list();
        g_application_quit(G_APPLICATION(app));
}

static GActionEntry app_entries[] = {
        { "merge", merge_activated, NULL, NULL, NULL },
        { "open", open_file_activated, NULL, NULL, NULL },
        { "clear_list", clear_list_activated, NULL, NULL, NULL },
        { "quit", quit_activated, NULL, NULL, NULL }
};

static void converter_app_startup(GApplication *app)
{
        const char *quit_accels[2] = { "<Ctrl>Q", NULL };
        const char *merge_accels[2] = { "<Ctrl>S", NULL };
        const char *open_accels[2] = { "<Ctrl>F", NULL };

        G_APPLICATION_CLASS(converter_app_parent_class)->startup(app);
        g_action_map_add_action_entries(
                G_ACTION_MAP(app),
                app_entries,
                G_N_ELEMENTS(app_entries),
                app
        );
        gtk_application_set_accels_for_action(
                GTK_APPLICATION(app),
                "app.quit",
                quit_accels
        );
        gtk_application_set_accels_for_action(
                GTK_APPLICATION(app),
                "app.merge",
                merge_accels
        );
        gtk_application_set_accels_for_action(
                GTK_APPLICATION(app),
                "app.open",
                open_accels
        );
}

static void converter_app_class_init(ConverterAppClass *class)
{
        // override the activate vfunc
        G_APPLICATION_CLASS(class)->activate = converter_app_activate;
        G_APPLICATION_CLASS(class)->open = converter_app_open;
        G_APPLICATION_CLASS(class)->startup = converter_app_startup;
}
