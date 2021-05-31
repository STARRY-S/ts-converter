#include <gtk/gtk.h>
#include <stdio.h>

#include "converterapp.h"
#include "converterappwin.h"
#include "filelist.h"

struct _ConverterApp
{
        GtkApplication parent;
};

G_DEFINE_TYPE(ConverterApp, converter_app, GTK_TYPE_APPLICATION);

static void converter_app_init(ConverterApp *app)
{
        init_file_list();
}

static void converter_app_activate(GApplication *app)
{
        ConverterAppWindow *window;

        window = converter_app_window_new(CONVERTER_APP(app));

        /* finish & show */
        converter_app_window_open(window, NULL);
        gtk_window_present(GTK_WINDOW(window));
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

static void on_cancel_response(GtkDialog *dialog, int response)
{
        if (response != GTK_RESPONSE_CANCEL) {
                return;
        }

        gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_open_response(GtkDialog *dialog, int response)
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
                printf("select file: %s\n", pathname);
                insert_sort(pathname, basename, default_str_is_larger);
                printf("Insert %s: \n", basename);
                print_file_list();
                printf("\n");
        }
        calculate_file_id();
        print_file_list();

        gtk_window_destroy(GTK_WINDOW(dialog));
}

static void open_file_activated(GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer      app)
{
        /* file picker */
        GList *wins = gtk_application_get_windows(GTK_APPLICATION(app));
        ConverterAppWindow *window = CONVERTER_APP_WINDOW(wins->data);

        GtkFileChooserAction open_action = GTK_FILE_CHOOSER_ACTION_OPEN;
        GtkWidget *dialog = gtk_file_chooser_dialog_new(
                "Open File",
                GTK_WINDOW(window),
                open_action,
                "_Cancel",
                GTK_RESPONSE_CANCEL,
                "_Open",
                GTK_RESPONSE_ACCEPT,
                NULL
        );

        g_signal_connect(
                dialog,
                "response",
                G_CALLBACK(on_open_response),
                NULL
        );

        g_signal_connect(
                dialog,
                "response",
                G_CALLBACK(on_cancel_response),
                NULL
        );

        gtk_file_chooser_set_select_multiple(
                GTK_FILE_CHOOSER(dialog),
                TRUE
        );

        // gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
        gtk_widget_show(dialog);
}

static void merge_activated(GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer      app)
{
        GList *wins = gtk_application_get_windows(GTK_APPLICATION(app));
        ConverterAppWindow *window = CONVERTER_APP_WINDOW(wins->data);

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
        gtk_window_set_transient_for(mwindow, GTK_WINDOW(window));
        gtk_window_present(GTK_WINDOW(mwindow));

        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_set_text(buffer, "Test Log info here!", 12);
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

static void converter_app_finalize(ConverterAppClass *class)
{
}

static void converter_app_class_init(ConverterAppClass *class)
{
        // override the activate vfunc
        G_APPLICATION_CLASS(class)->activate = converter_app_activate;
        G_APPLICATION_CLASS(class)->open = converter_app_open;
        G_APPLICATION_CLASS(class)->startup = converter_app_startup;
}
