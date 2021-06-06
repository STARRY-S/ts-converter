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

        struct List *list = converter_filelist_get_list();
        if (list == NULL || list->size == 0) {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(
                        store, &iter,
                        COLUMN_NUMBER, 0,
                        COLUMN_FORMAT, "UNKNOW",
                        COLUMN_FILENAME, "Please import file first.",
                        -1
                );
                return GTK_TREE_MODEL(store);
        }

        /* add data to the list store */
        for (struct video *p = list->begin->next; p != NULL; p = p->next)
        {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(
                        store, &iter,
                        COLUMN_NUMBER, p->id,
                        COLUMN_FORMAT, p->format,
                        COLUMN_FILENAME, p->path,
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
        return g_object_new(CONVERTER_APP_WINDOW_TYPE,
                "application", app, NULL
        );
}

void converter_app_window_update_list(ConverterAppWindow *window)
{
        GtkTreeModel *model = init_model();
        gtk_tree_view_set_model(window->treeview, model);
        g_object_unref(model);

        // gtk_list_store_clear(GTK_LIST_STORE(window->treeview) );
}

void converter_app_window_open(ConverterAppWindow *window)
{
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_margin_start(vbox, 8);
        gtk_widget_set_margin_end(vbox, 8);
        gtk_widget_set_margin_top(vbox, 8);
        gtk_widget_set_margin_bottom(vbox, 8);
        gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(vbox));

        GtkWidget *scroll_view = gtk_scrolled_window_new();
        gtk_widget_set_hexpand(scroll_view, TRUE);
        gtk_widget_set_vexpand(scroll_view, TRUE);
        // gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroll_view), TRUE);
        // gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_view),
                                        // GTK_POLICY_NEVER,
                                        // GTK_POLICY_AUTOMATIC);
        gtk_box_append(GTK_BOX(vbox), scroll_view);

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
