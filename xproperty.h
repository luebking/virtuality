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

#ifndef XPROPERTY_H
#define XPROPERTY_H

//#include <QWidget>
//#include <QtDebug>

#if QT_VERSION >= 0x060200
#include <QGuiApplication>
#define BE_X11_DISPLAY qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display()
#define BE_X11_ROOT DefaultRootWindow(BE_X11_DISPLAY)
#define BE_XCB_CONN qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->connection()
#else
#include <QX11Info>
#define BE_X11_DISPLAY QX11Info::display()
#define BE_X11_ROOT QX11Info::appRootWindow()
#define BE_XCB_CONN QX11Info::connection()
#endif
#include <QWidget>
//#include <QtDebug>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

namespace BE {


typedef struct _WindowData
{
    QRgb inactiveWindow, activeWindow, inactiveDeco, activeDeco,
         inactiveText, activeText, inactiveButton, activeButton;
    int style;
} WindowData;

typedef struct _WindowPics
{
    Picture topTile, btmTile, cnrTile, lCorner, rCorner;
} WindowPics;

class BLIB_EXPORT XProperty
{
public:
    enum Type { LONG = 1, BYTE = 8, WORD = 16, ATOM = 32 };
    static Atom winData, bgPics, decoDim, pid, blurRegion,
                forceShadows, kwinShadow, bespinShadow[2],
                netSupported, blockCompositing;
    static void init();

    template <typename T> inline static T *get(WId window, Atom atom, Type type, unsigned long *n = 0)
    {
        unsigned long nn = n ? *n : 1;
        T *data = 0;
        T **data_p = &data;
        nn = handleProperty(window, atom, (uchar**)data_p, type, _n<T>(type, nn));
        if (n)
            *n = nn;
        return data;
    }

    template <typename T> inline static void set(WId window, Atom atom, T *data, Type type, unsigned long n = 1)
    {
        if (!data) return;
        T **data_p = &data;
        handleProperty(window, atom, (uchar**)data_p, type, _n<T>(type, n));
    }

    static void remove(WId window, Atom atom);
    static void setAtom(WId window, Atom atom);
private:
    static unsigned long handleProperty(WId window, Atom atom, uchar **data, Type type, unsigned long n);
    template <typename T> inline static long _n(Type type, unsigned long n)
    {
        if (n < 1)
            return 0;
        unsigned long _n = n*sizeof(T)*8;
        _n /= (type == XProperty::LONG) ? 8*sizeof(long int) : (uint)type;
        return _n ? _n : 1L;
    }
};

inline bool isPlatformX11() {
#if QT_VERSION < 0x050000
    return true;
#elif QT_VERSION >= 0x060200
    return bool(qGuiApp->nativeInterface<QNativeInterface::QX11Application>());
#else
    return QX11Info::isPlatformX11();
#endif
}
}
#endif // XPROPERTY_H
