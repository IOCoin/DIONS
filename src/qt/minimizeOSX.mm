#include "iocoingui.h"
#include "minimizeOSX.h"
#include <Cocoa/Cocoa.h>

void MinimizeOSX::min(IocoinGUI* w)
{
  NSView* view = (NSView*)w->winId();
  NSWindow* wnd = [view window];
  [wnd setStyleMask:[wnd styleMask] | NSWindowStyleMaskMiniaturizable];
  w->showMinimized();
}
