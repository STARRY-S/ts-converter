#include <stdio.h>

#include "converterapp.h"
#include "converterappwin.h"

struct _ConverterAppWindow
{
        GtkApplicationWindow parent;

        GtkWidget *gears;
        GtkTreeView *treeview;
};

G_DEFINE_TYPE(ConverterAppWindow,
        converter_app_window,
        GTK_TYPE_APPLICATION_WINDOW
);

// static struct selected_file flist[] = {
//         { 1, "mp4", "./bob.mp4" },
//         { 2, "mp4", "./hello.mp4" },
//         { 3, "mp4", "/home/Alice/Videos/game.mp4" },
//         { 4, "mp4", "/usr/share/nginx/videos/share.mp4" },
//         { 5, "mp4", "/usr/share/nginx/videos/share.mp4" },
//         { 6, "mp4", "/usr/share/nginx/videos/share.mp4" },
//         { 7, "mp4", "/usr/share/nginx/videos/share.mp4" },
//         { 8, "mp4", "/usr/share/nginx/videos/share.mp4" },
//         { 9, "mp4", "/usr/share/nginx/videos/share.mp4" },
//         { 10, "mp4", "/usr/share/nginx/videos/share.mp4" },
//         { 10, "mp4", "/usr/share/nginx/videos/share0.mp4" },
//         { 11, "mp4", "/usr/share/nginx/videos/share1.mp4" },
//         { 12, "mp4", "/usr/share/nginx/videos/share2.mp4" },
//         { 13, "mp4", "/usr/share/nginx/videos/share3.mp4" }
// };

static struct selected_file flist[] = {
        {0, "UNKNOWN", "Please select videos first. [Ctrl-F]" }
};

static GtkTreeModel *create_model()
{
        GtkListStore *store;
        GtkTreeIter iter;

        /* create list store */
        store = gtk_list_store_new(
                NUM_COLUMNS,
                G_TYPE_UINT,
                G_TYPE_STRING,
                G_TYPE_STRING
        );

        /* add data to the list store */
        for (int i = 0; i < G_N_ELEMENTS(flist); i++)
        {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(
                        store, &iter,
                        COLUMN_NUMBER, flist[i].num,
                        COLUMN_FORMAT, flist[i].format,
                        COLUMN_FILENAME, flist[i].path,
                        -1
                );
        }
        return GTK_TREE_MODEL(store);
}


// static void add_columns(GtkTreeView *treeview)
// {
//         GtkCellRenderer *renderer;
//         GtkTreeViewColumn *column;
//
//         /* column for fixed toggles */
//         renderer = gtk_cell_renderer_text_new();
//         column = gtk_tree_view_column_new_with_attributes(
//                 "ID",
//                 renderer,
//                 "text",
//                 COLUMN_NUMBER,
//                 NULL
//         );
//
//         gtk_tree_view_column_set_sizing(
//                 GTK_TREE_VIEW_COLUMN(column),
//                 GTK_TREE_VIEW_COLUMN_AUTOSIZE
//         );
//         gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
//                                          GTK_TREE_VIEW_COLUMN_FIXED);
//         gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 70);
//
//         gtk_tree_view_column_set_sort_column_id(column, COLUMN_NUMBER);
//         gtk_tree_view_append_column(treeview, column);
//
//         /* column for file paths */
//         renderer = gtk_cell_renderer_text_new();
//         column = gtk_tree_view_column_new_with_attributes(
//                 "Videos",
//                 renderer,
//                 "text",
//                 COLUMN_FILENAME,
//                 NULL
//         );
//         gtk_tree_view_column_set_sizing(
//                 GTK_TREE_VIEW_COLUMN(column),
//                 GTK_TREE_VIEW_COLUMN_GROW_ONLY
//         );
//         gtk_tree_view_column_set_sort_column_id(column, COLUMN_NUMBER);
//         gtk_tree_view_append_column(treeview, column);
// }

static void converter_app_window_init(ConverterAppWindow *win)
{
        GtkBuilder *builder;
        GMenuModel *menu;

        gtk_widget_init_template(GTK_WIDGET(win));

        builder = gtk_builder_new_from_resource(
                "/me/starry-s/converter/menu.ui"
        );
        menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
        gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(win->gears), menu);
        g_object_unref(builder);

        builder = gtk_builder_new_from_resource(
                "/me/starry-s/converter/list.ui"
        );
        // GtkTreeView *tree;
        win->treeview = GTK_TREE_VIEW(
                gtk_builder_get_object(builder, "file-treeview")
        );
        // gtk_tree_view_set_model(GTK_TREE_VIEW(win->treeview), tree);

}

static void converter_app_window_class_init(ConverterAppWindowClass *class)
{
        gtk_widget_class_set_template_from_resource(
                GTK_WIDGET_CLASS(class),
                /* me.starry-s.converter */
                "/me/starry-s/converter/window.ui"
        );

        gtk_widget_class_bind_template_child(
                GTK_WIDGET_CLASS(class),
                ConverterAppWindow,
                gears
        );
}

ConverterAppWindow *converter_app_window_new(ConverterApp *app)
{
        return g_object_new(CONVERTER_APP_WINDOW_TYPE, "application", app, NULL);
}

void converter_app_window_open(ConverterAppWindow *window, GFile *file)
{
        // char *basename;
        // GtkWidget *scrolled, *view;
        // char *contents;
        // gsize length;
        //
        // basename = g_file_get_basename(file);
        //
        // scrolled = gtk_scrolled_window_new();
        // gtk_widget_set_hexpand(scrolled, TRUE);
        // gtk_widget_set_vexpand(scrolled, TRUE);
        // view = gtk_text_view_new();
        // gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
        // gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
        // gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
        // gtk_stack_add_titled(
        //         GTK_STACK(win->stack), scrolled, basename, basename
        // );
        //
        // if (g_file_load_contents(file, NULL, &contents, &length, NULL, NULL))
        // {
        //         GtkTextBuffer *buffer;
        //
        //         buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
        //         gtk_text_buffer_set_text(buffer, contents, length);
        //         g_free(contents);
        // } else {
        //         printf("failed to open file.\n");
        // }
        //

        /* create file list store */
        GtkWidget *grid_view;

        /* create window, etc */
        GtkTreeModel *model = NULL;
        // g_object_add_weak_pointer(G_OBJECT(window), (gpointer *)&window);

        grid_view = gtk_grid_new();
        gtk_widget_set_margin_start(grid_view, 8);
        gtk_widget_set_margin_end(grid_view, 8);
        gtk_widget_set_margin_top(grid_view, 8);
        gtk_widget_set_margin_bottom(grid_view, 8);
        gtk_window_set_child(GTK_WINDOW(window), (GtkWidget*) grid_view);

        GtkWidget *scroll_view = gtk_scrolled_window_new();
        gtk_widget_set_hexpand(scroll_view, TRUE);
        gtk_widget_set_vexpand(scroll_view, TRUE);
        gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroll_view), TRUE);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_view),
                                        GTK_POLICY_NEVER,
                                        GTK_POLICY_AUTOMATIC);
        gtk_grid_attach(GTK_GRID(grid_view), scroll_view, 0, 0, 3, 1);

        /* create tree model */
        model = create_model();

        /* create tree view */
        // GtkWidget *treeview = gtk_tree_view_new_with_model(model);
        gtk_tree_view_set_model(window->treeview, model);
        gtk_widget_set_vexpand(GTK_WIDGET(window->treeview), TRUE);
        // gtk_tree_view_set_search_column(
        //         GTK_TREE_VIEW (treeview),
        //         COLUMN_FILENAME
        // );

        g_object_unref(model);

        gtk_scrolled_window_set_child(
                GTK_SCROLLED_WINDOW(scroll_view),
                GTK_WIDGET(window->treeview)
        );

        /* Demo */
        // add_columns(GTK_TREE_VIEW(treeview));
}
