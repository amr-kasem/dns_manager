#include "my_application.h"

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif

int main(int argc, char** argv) {
#ifdef GDK_WINDOWING_X11
  // Initialize X11 threading to prevent XCB errors
  XInitThreads();
#endif
  
  g_autoptr(MyApplication) app = my_application_new();
  return g_application_run(G_APPLICATION(app), argc, argv);
}
