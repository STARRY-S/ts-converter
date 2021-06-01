#include <gtk/gtk.h>
#include "converter-app.h"

int main(int argc, char **argv)
{
        return g_application_run(
                G_APPLICATION(converter_app_new()),
                argc,
                argv
        );
}
