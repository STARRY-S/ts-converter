#include <gtk/gtk.h>

#include "converterapp.h"
#include "converterappwin.h"

enum {
        COLUMN_NUMBER,
        COLUMN_FILENAME,
        NUM_COLUMNS
};

struct selected_file {
        guint num;
        char *format;
        char *path;
};

static struct selected_file flist[] = {
        { 1, "mp4", "./bob.mp4" },
        { 2, "mp4", "./hello.mp4" },
        { 3, "mp4", "/home/Alice/Videos/game.mp4" },
        { 4, "mp4", "/usr/share/nginx/videos/share.mp4" },
        { 5, "mp4", "/usr/share/nginx/videos/share.mp4" },
        { 6, "mp4", "/usr/share/nginx/videos/share.mp4" },
        { 7, "mp4", "/usr/share/nginx/videos/share.mp4" },
        { 8, "mp4", "/usr/share/nginx/videos/share.mp4" },
        { 9, "mp4", "/usr/share/nginx/videos/share.mp4" },
        { 10, "mp4", "/usr/share/nginx/videos/share.mp4" },
        { 10, "mp4", "/usr/share/nginx/videos/share0.mp4" },
        { 11, "mp4", "/usr/share/nginx/videos/share1.mp4" },
        { 12, "mp4", "/usr/share/nginx/videos/share2.mp4" },
        { 13, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 14, "mp4", "/usr/share/nginx/videos/a/very/large/long/file/name1111111122222222\
/aaaaaabbbbbbbccccccccc.mp4" },
        { 15, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 16, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 17, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 18, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 19, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 20, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 21, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 22, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 23, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 24, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 25, "mp4", "/usr/share/nginx/videos/share3.mp4" },
        { 26, "mp4", "/usr/share/nginx/videos/share3.mp4" }
};

struct _ConverterApp
{
        GtkApplication parent;
};

G_DEFINE_TYPE(ConverterApp, converter_app, GTK_TYPE_APPLICATION);

static GtkTreeModel *create_model(void)
{
        GtkListStore *store;
        GtkTreeIter iter;

        /* create list store */
        store = gtk_list_store_new(
                NUM_COLUMNS,
                G_TYPE_UINT,
                G_TYPE_STRING
        );

        /* add data to the list store */
        for (int i = 0; i < G_N_ELEMENTS(flist); i++)
        {
                // const char *icon_name;
                // gboolean sensitive;

                gtk_list_store_append(store, &iter);
                gtk_list_store_set(
                        store, &iter,
                        COLUMN_NUMBER, flist[i].num,
                        COLUMN_FILENAME, flist[i].path,
                        -1
                );
        }
        return GTK_TREE_MODEL(store);
}

static void add_columns(GtkTreeView *treeview)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        /* column for fixed toggles */
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(
                "ID",
                renderer,
                "text",
                COLUMN_NUMBER,
                NULL
        );

        gtk_tree_view_column_set_sizing(
                GTK_TREE_VIEW_COLUMN(column),
                GTK_TREE_VIEW_COLUMN_AUTOSIZE
        );
        gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                         GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 70);

        gtk_tree_view_column_set_sort_column_id(column, COLUMN_NUMBER);
        gtk_tree_view_append_column(treeview, column);

        /* column for file paths */
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(
                "File path",
                renderer,
                "text",
                COLUMN_FILENAME,
                NULL
        );
        gtk_tree_view_column_set_sizing(
                GTK_TREE_VIEW_COLUMN(column),
                GTK_TREE_VIEW_COLUMN_GROW_ONLY
        );
        gtk_tree_view_column_set_sort_column_id(column, COLUMN_NUMBER);
        gtk_tree_view_append_column(treeview, column);
}

static void converter_app_init(ConverterApp *app)
{
        ;
}

static void converter_app_activate(GApplication *app)
{
        ConverterAppWindow *window;

        window = converter_app_window_new(CONVERTER_APP(app));

        /* create file list store */
        GtkGrid *grid_view;

        /* create window, etc */
        GtkTreeModel *model = NULL;
        // g_object_add_weak_pointer(G_OBJECT(window), (gpointer *)&window);

        grid_view = (GtkGrid*) gtk_grid_new();
        gtk_window_set_child(GTK_WINDOW(window), (GtkWidget*) grid_view);

        GtkWidget *scroll_view = gtk_scrolled_window_new ();
        gtk_widget_set_hexpand(scroll_view, TRUE);
        gtk_widget_set_vexpand(scroll_view, TRUE);
        gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroll_view), TRUE);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_view),
                                        GTK_POLICY_NEVER,
                                        GTK_POLICY_AUTOMATIC);
        gtk_grid_attach(grid_view, scroll_view, 0, 0, 1, 1);

        /* create tree model */
        model = create_model();

        /* create tree view */
        GtkWidget *treeview = gtk_tree_view_new_with_model(model);
        gtk_widget_set_vexpand(treeview, TRUE);
        gtk_tree_view_set_search_column(
                GTK_TREE_VIEW (treeview),
                COLUMN_FILENAME
        );

        g_object_unref(model);

        gtk_scrolled_window_set_child(
                GTK_SCROLLED_WINDOW(scroll_view),
                treeview
        );

        /* Demo */
        add_columns(GTK_TREE_VIEW(treeview));

        /* finish & show */
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

        converter_app_window_open(NULL, NULL);

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

static void preferences_activated(GSimpleAction *action,
                                  GVariant      *parameter,
                                  gpointer      app)
{
        /* file picker */
        // GtkWidget *dialog;
        // GtkFileChooserAction open_action = GTK_FILE_CHOOSER_ACTION_OPEN;
        //
        // dialog = gtk_file_chooser_dialog_new ("Open File",
        //                               gtk_application_get_windows(GTK_APPLICATION(app)),
        //                               open_action,
        //                               "_Cancel",
        //                               GTK_RESPONSE_CANCEL,
        //                               "_Open",
        //                               GTK_RESPONSE_ACCEPT,
        //                               NULL);
        // GtkFileChooser *chooser;
        // chooser = GTK_FILE_CHOOSER(dialog);
        // gtk_widget_show(dialog);
}

static void quit_activated(GSimpleAction *action,
                           GVariant      *parameter,
                           gpointer      app)
{
        g_application_quit(G_APPLICATION(app));
}

static GActionEntry app_entries[] = {
        { "preference", preferences_activated, NULL, NULL, NULL },
        { "quit", quit_activated, NULL, NULL, NULL }
};

static void converter_app_startup(GApplication *app)
{
        const char *quit_accels[2] = { "<Ctrl>Q", NULL };

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
}

static void converter_app_class_init(ConverterAppClass *class)
{
        // override the activate vfunc
        G_APPLICATION_CLASS(class)->activate = converter_app_activate;
        G_APPLICATION_CLASS(class)->open = converter_app_open;
        G_APPLICATION_CLASS(class)->startup = converter_app_startup;
}
