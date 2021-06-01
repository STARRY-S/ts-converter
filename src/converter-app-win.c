#include <stdio.h>

#include "converter-app.h"
#include "converter-app-win.h"
#include "converter-filelist.h"

struct _ConverterAppWindow
{
        GtkApplicationWindow parent;

        GtkWidget *gears;
        GtkTreeView *treeview;
        GtkTreeModel *model;
};

G_DEFINE_TYPE(ConverterAppWindow,
        converter_app_window,
        GTK_TYPE_APPLICATION_WINDOW
);

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
        { 13, "mp4", "/usr/share/nginx/videos/share3.mp4" }
};

enum {
        COLUMN_NUMBER,
        COLUMN_FORMAT,
        COLUMN_FILENAME,
        NUM_COLUMNS
};

static GtkTreeModel *init_model()
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

        win->model = init_model();
        if (win->model == NULL) {
                return;
        }

        win->treeview = GTK_TREE_VIEW(
                gtk_builder_get_object(builder, "file-treeview")
        );
        gtk_tree_view_set_model(win->treeview, win->model);
        g_object_unref(win->model);
        win->model = NULL;
}

static void converter_app_window_class_init(ConverterAppWindowClass *class)
{
        gtk_widget_class_set_template_from_resource(
                GTK_WIDGET_CLASS(class),
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

void converter_app_window_open(ConverterAppWindow *window)
{
        GtkWidget *grid_view;

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

        /* create tree view */
        gtk_widget_set_vexpand(GTK_WIDGET(window->treeview), TRUE);
        // gtk_tree_view_set_search_column(
        //         GTK_TREE_VIEW (treeview),
        //         COLUMN_FILENAME
        // );

        gtk_scrolled_window_set_child(
                GTK_SCROLLED_WINDOW(scroll_view),
                GTK_WIDGET(window->treeview)
        );
}
