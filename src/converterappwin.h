#ifndef _EXAMPLEAPPWIN_H_
#define _EXAMPLEAPPWIN_H_

#include <gtk/gtk.h>

#define CONVERTER_APP_WINDOW_TYPE (converter_app_window_get_type ())

G_DECLARE_FINAL_TYPE(
        ConverterAppWindow,
        converter_app_window,
        CONVERTER,
        APP_WINDOW,
        GtkApplicationWindow
)

ConverterAppWindow *converter_app_window_new(ConverterApp *app);
void converter_app_window_open(ConverterAppWindow *win, GFile *file);

#endif
