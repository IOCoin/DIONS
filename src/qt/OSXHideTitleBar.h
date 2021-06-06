#ifndef OSXHIDETITLEBAR_H
#define OSXHIDETITLEBAR_H

#include "iocoingui.h"

class IocoinGUI;

class OSXHideTitleBar
{
public:
    static void HideTitleBar(long winid);
    static void min(IocoinGUI*);
};

#endif
