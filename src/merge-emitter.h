#ifndef __MERGE_EMITTER_H__
#define __MERGE_EMITTER_H__

#include <gtk/gtk.h>

#define MERGE_EMITTER_TYPE (merge_emitter_get_type())

G_DECLARE_FINAL_TYPE(
        MergeEmitter,
        merge_emitter,
        MERGE,
        EMITTER,
        GObject
)

MergeEmitter *merge_emitter_new(void);


#endif
