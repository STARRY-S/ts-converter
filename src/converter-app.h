#ifndef _CONVERTER_APP_H_
#define _CONVERTER_APP_H_

#include <gtk/gtk.h>

#define CONVERTER_APP_TYPE (converter_app_get_type ())
G_DECLARE_FINAL_TYPE(
        ConverterApp,
        converter_app,
        CONVERTER,
        APP,
        GtkApplication
)

ConverterApp *converter_app_new(void);


#endif
