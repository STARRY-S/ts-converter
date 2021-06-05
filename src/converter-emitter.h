#ifndef __CONVERTER_EMITTER_H__
#define __CONVERTER_EMITTER_H__

#include <gtk/gtk.h>

#define CONVERTER_EMITTER_TYPE (converter_emitter_get_type())

G_DECLARE_FINAL_TYPE(
        ConverterEmitter,
        converter_emitter,
        CONVERTER,
        EMITTER,
        GObject
)

ConverterEmitter *converter_emitter_new(void);

#endif
