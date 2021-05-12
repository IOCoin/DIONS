#include "iocoingui.h"
#include "OSXHideTitleBar.h"
#include <Cocoa/Cocoa.h>

void OSXHideTitleBar::HideTitleBar(long winid)
{
    NSView *nativeView = reinterpret_cast<NSView *>(winid);
    NSWindow* nativeWindow = [nativeView window];

    [nativeWindow setStyleMask:
        [nativeWindow styleMask] | NSFullSizeContentViewWindowMask | NSWindowTitleHidden];

    [nativeWindow setTitlebarAppearsTransparent:YES];
}
void OSXHideTitleBar::min(IocoinGUI* w)
{
  NSView* view = (NSView*)w->winId();
  NSWindow* wnd = [view window];
  [wnd setStyleMask:[wnd styleMask] | NSWindowStyleMaskMiniaturizable];
  w->showMinimized();
}
