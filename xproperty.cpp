/*
 *   Bespin library for Qt style, KWin decoration and everythng else
 *   Copyright 2009-2014 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtDebug> // gets us Qt version also ;)
#include "xproperty.h"


using namespace BE;

#include <X11/Xatom.h>
#include "fixx11h.h"

BLIB_EXPORT Atom XProperty::winData;
BLIB_EXPORT Atom XProperty::bgPics;
BLIB_EXPORT Atom XProperty::decoDim;
BLIB_EXPORT Atom XProperty::pid;
BLIB_EXPORT Atom XProperty::blurRegion;
BLIB_EXPORT Atom XProperty::forceShadows;
BLIB_EXPORT Atom XProperty::kwinShadow;
//     const char* const ShadowHelper::netWMForceShadowPropertyName( "_KDE_NET_WM_FORCE_SHADOW" );
//     const char* const ShadowHelper::netWMSkipShadowPropertyName( "_KDE_NET_WM_SKIP_SHADOW" );
BLIB_EXPORT Atom XProperty::bespinShadow[2];
BLIB_EXPORT Atom XProperty::netSupported;
BLIB_EXPORT Atom XProperty::blockCompositing;

void
XProperty::init()
{
    static bool initialized = false;
    if (initialized)
        return;
    winData = XInternAtom(BE_X11_DISPLAY, "BESPIN_WIN_DATA", False);
    bgPics = XInternAtom(BE_X11_DISPLAY, "BESPIN_BG_PICS", False);
    decoDim = XInternAtom(BE_X11_DISPLAY, "BESPIN_DECO_DIM", False);
    pid = XInternAtom(BE_X11_DISPLAY, "_NET_WM_PID", False);
    blurRegion = XInternAtom(BE_X11_DISPLAY, "_KDE_NET_WM_BLUR_BEHIND_REGION", False);
    forceShadows = XInternAtom( BE_X11_DISPLAY, "_KDE_SHADOW_FORCE", False );
    kwinShadow = XInternAtom( BE_X11_DISPLAY, "_KDE_NET_WM_SHADOW", False );
    bespinShadow[0] = XInternAtom( BE_X11_DISPLAY, "BESPIN_SHADOW_SMALL", False );
    bespinShadow[1] = XInternAtom( BE_X11_DISPLAY, "BESPIN_SHADOW_LARGE", False );
    netSupported = XInternAtom( BE_X11_DISPLAY, "_NET_SUPPORTED", False );
    blockCompositing = XInternAtom( BE_X11_DISPLAY, "_KDE_NET_WM_BLOCK_COMPOSITING", False );
    initialized = true;
}

void
XProperty::setAtom(WId window, Atom atom)
{
    const char *data = "1";
    XChangeProperty(BE_X11_DISPLAY, window, atom, XA_ATOM, 32, PropModeReplace, (uchar*)data, 1 );
}

unsigned long
XProperty::handleProperty(WId window, Atom atom, uchar **data, Type type, unsigned long n)
{
    int format = (type == LONG ? 32 : type);
    Atom xtype = (type == ATOM ? XA_ATOM : XA_CARDINAL);
    if (*data) // this is ok, internally used only
    {
        XChangeProperty(BE_X11_DISPLAY, window, atom, xtype, format, PropModeReplace, *data, n );
        XSync(BE_X11_DISPLAY, False);
        return 0;
    }
    int result, de; //dead end
    unsigned long nn, de2;
    int nmax = n ? n : 0xffffffff;
    result = XGetWindowProperty(BE_X11_DISPLAY, window, atom, 0L, nmax, False, xtype, &de2, &de, &nn, &de2, data);
    if (result != Success || *data == NULL || (n > 0 && n != nn))
        *data = NULL; // superflous?!?
    return nn;
}

void
XProperty::remove(WId window, Atom atom)
{
    XDeleteProperty(BE_X11_DISPLAY, window, atom);
}

#if 0

/* The below functions mangle 2 rbg (24bit) colors and a 2 bit hint into
a 32bit integer to be set as X11 property
Of course this is convulsive, but doesn't hurt for our purposes
::encode() is a bit trickier as it needs to decide whether the color values
should be rounded up or down like
x = qMin(qRround(x/8.0),31) IS WRONG! as it would impact the hue and while
value manipulations are acceptable, hue values are NOT (this is a 8v stepping
per channel and as we're gonna create gradients out of the colors, black could
turn some kind of very dark red...)
Just trust and don't touch ;) (Yes future Thomas, this means YOU!)
======================================================================*/

#include <QtDebug>
uint
XProperty::encode(const QColor &bg, const QColor &fg, uint hint)
{
    int r,g,b; bg.getRgb(&r,&g,&b);
    int d = r%8 + g%8 + b%8;
    if (d > 10)
    {
        r = qMin(r+8, 255);
        g = qMin(g+8, 255);
        b = qMin(b+8, 255);
    }
    uint info = (((r >> 3) & 0x1f) << 27) | (((g >> 3) & 0x1f) << 22) | (((b >> 3) & 0x1f) << 17);

    fg.getRgb(&r,&g,&b);
    d = r%8 + g%8 + b%8;
    if (d > 10)
    {
        r = qMin(r+8, 255);
        g = qMin(g+8, 255);
        b = qMin(b+8, 255);
    }
    info |= (((r >> 3) & 0x1f) << 12) | (((g >> 3) & 0x1f) << 7) | (((b >> 3) & 0x1f) << 2) | hint & 3;
    return info;
}

void
XProperty::decode(uint info, QColor &bg, QColor &fg, uint &hint)
{
    bg.setRgb(  ((info >> 27) & 0x1f) << 3,
                ((info >> 22) & 0x1f) << 3,
                ((info >> 17) & 0x1f) << 3 );
    fg.setRgb(  ((info >> 12) & 0x1f) << 3,
                ((info >> 7) & 0x1f) << 3,
                ((info >> 2) & 0x1f) << 3 );
    hint = info & 3;
}
#endif
