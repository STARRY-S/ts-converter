#include <gtk/gtk.h>
#include "converterapp.h"

int main(int argc, char **argv)
{
        return g_application_run(G_APPLICATION(converter_app_new()), argc, argv);
}
