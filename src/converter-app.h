#ifndef _EXAMPLEAPP_H_
#define _EXAMPLEAPP_H_

#include <gtk/gtk.h>

#define CONVERTER_APP_TYPE (converter_app_get_type ())
G_DECLARE_FINAL_TYPE(
        ConverterApp,
        converter_app,
        CONVERTER,
        APP,
        GtkApplication
)

struct selected_file {
        guint num;
        char *format;
        char *path;
};

ConverterApp *converter_app_new(void);


#endif
