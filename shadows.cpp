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

#include <QEvent>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#ifdef BE_WS_X11
#if QT_VERSION >= 0x060200
#include <QGuiApplication>
#else
#include <QX11Info>
#endif
#endif
#include "fixx11h.h"
#include <QtDebug>

#include "FX.h"
#include "shadows.h"

using namespace BE;

#ifdef BE_WS_X11
#include "xproperty.h"
#include "fixx11h.h"
class ShadowManager : public QObject {
public:
    ShadowManager() : QObject() {}
protected:
    bool eventFilter(QObject *o, QEvent *e)
    {
        if (e->type() == QEvent::Show)
        if (QWidget *w = qobject_cast<QWidget*>(o))
        if (w->isWindow() && w->testAttribute(Qt::WA_WState_Created) && w->internalWinId())
            Shadows::set(w->winId(), Shadows::Small);
        return false;
    }
};

static ShadowManager *shadowManager = 0;
static uint size[2] = { 12, 64 };
static QColor color(0,0,0,0);
static bool halo(false);

static Pixmap (*pixmaps[2])[8] = {0,0};
static unsigned long globalShadowData[2][12];
#endif

static Pixmap nativePixmap(const QImage qtImg)
{
#ifdef BE_WS_X11
    if (qtImg.isNull())
        return 0;

    static QImage dither;
    if (dither.isNull())
        dither = FX::newDitherImage();
    QPainter pd(const_cast<QImage*>(&qtImg));
    for (int x = 0; x < qtImg.width() - dither.width(); x += dither.width()) {
        for (int y = 0; y < qtImg.height() - dither.height(); y += dither.height()) {
            pd.drawImage(x, y, dither);
        }
    }
    pd.end();

    XImage ximage;
    ximage.width            = qtImg.width();
    ximage.height           = qtImg.height();
    ximage.xoffset          = 0;
    ximage.format           = ZPixmap;
    // This is a hack to prevent the image data from detaching
    ximage.data             = (char*) const_cast<const QImage*>(&qtImg)->bits();
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    ximage.byte_order       = MSBFirst;
#else
    ximage.byte_order       = LSBFirst;
#endif
    ximage.bitmap_unit      = 32;
    ximage.bitmap_bit_order = ximage.byte_order;
    ximage.bitmap_pad       = 32;
    ximage.depth            = 32;
    ximage.bytes_per_line   = qtImg.bytesPerLine();
    ximage.bits_per_pixel   = 32;
    ximage.red_mask     = 0x00ff0000;
    ximage.green_mask   = 0x0000ff00;
    ximage.blue_mask    = 0x000000ff;
    ximage.obdata           = 0;
    if (!XInitImage(&ximage)) {
        qDebug() << "error on init ximage";
    }

    Pixmap xPix = XCreatePixmap(BE_X11_DISPLAY, DefaultRootWindow(BE_X11_DISPLAY), qtImg.width(), qtImg.height(), 32);
    GC context = XCreateGC(BE_X11_DISPLAY, xPix, 0, NULL);
    XPutImage(BE_X11_DISPLAY, xPix, context, &ximage, 0, 0, 0, 0, qtImg.width(), qtImg.height());
    XFreeGC(BE_X11_DISPLAY, context);
    return xPix;
#else
    return 0; // just for GCC - makes no sense at all anyway
#endif
}

static void tile(const QImage &src, QImage &dst, bool h)
{
    const QRgb* srcPixel = (const QRgb*)src.constBits();
    QRgb* dstPixel = (QRgb*)dst.bits();
    if (h) {
        for (int i = 0; i < dst.width(); ++i)
        for (int j = 0; j < src.height(); ++j)
            dstPixel[dst.width()*j + i] = srcPixel[j];
    } else {
        for (int i = 0; i < dst.height(); ++i)
        for (int j = 0; j < src.width(); ++j)
            dstPixel[src.width()*i + j] = srcPixel[j];
    }
}

static unsigned long*
shadowData(Shadows::Type t, bool storeToRoot)
{
#ifdef BE_WS_X11
    XProperty::init();
    unsigned long _12 = 12;
    unsigned long *data = XProperty::get<unsigned long>(DefaultRootWindow(BE_X11_DISPLAY), XProperty::bespinShadow[t-1], XProperty::LONG, &_12);
    if (!data)
    {
        const int sz = size[t == Shadows::Large];
        float focus = 5.0f/sz;
        if (halo)
        {
            focus = 9.0f/sz;
            globalShadowData[t-1][8] = globalShadowData[t-1][9] = // yes, next line
            globalShadowData[t-1][10] = globalShadowData[t-1][11] = 3*(sz-4)/4;
        }
        else
        {
            focus = 5.0f/sz;
            globalShadowData[t-1][8] = (sz-4)/2;
            globalShadowData[t-1][9] = 2*(sz-4)/3;
            globalShadowData[t-1][10] = sz-4;
            globalShadowData[t-1][11] = 2*(sz-4)/3;
        }

        if (!pixmaps[t-1])
        {
            Pixmap *store = new Pixmap[8];
            pixmaps[t-1] = (Pixmap (*)[8])store;

            // radial gradient requires the raster engine anyway and we need *working* ... -> QImage
            QImage shadow(2*sz+1, 2*sz+1, QImage::Format_ARGB32);
            shadow.fill(Qt::transparent);
            QRadialGradient rg(QPoint(sz+1,sz+1),sz);
            const QRect shadowRect(shadow.rect());
            QPainter p(&shadow);
            p.setPen(Qt::NoPen);

            int weaken = sz/2;
            if (halo)
            {
                weaken = 0;
                rg.setColorAt(focus, QColor(255,255,255,192-sz)); rg.setColorAt(focus + (1.0f-focus)/2.0f, QColor(255,255,255,0));
                p.setBrush(rg);
                p.drawRect(shadowRect);
            }

            QColor transparent = color; transparent.setAlpha(0);
            color.setAlpha(96-weaken);
            rg.setColorAt(focus, color); rg.setColorAt(0.98, transparent);
            p.setBrush(rg);
            p.drawRect(shadowRect);

            rg.setStops(QGradientStops());

            color.setAlpha(80-weaken);
            rg.setColorAt(focus, color); rg.setColorAt(focus + (1.0f-focus)/1.25f, transparent);
            p.setBrush(rg);
            p.drawRect(shadowRect);

            rg.setStops(QGradientStops());

            color.setAlpha(64-weaken);
            rg.setColorAt(focus, color); rg.setColorAt(focus + (1.0f-focus)/1.52f, transparent);
            p.setBrush(rg);
            p.drawRect(shadowRect);

            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(Qt::transparent);
            p.drawRoundedRect(shadow.rect().adjusted(globalShadowData[t-1][9]+1, globalShadowData[t-1][8]+1,
                                                     -(1+globalShadowData[t-1][11]), -(1+globalShadowData[t-1][10])), 8,8);

            p.end();

            QImage vc(sz, 32, QImage::Format_ARGB32);
            QImage hc(32, sz, QImage::Format_ARGB32);
            QImage buffer = shadow.copy(sz,0,1,sz); // TopMid
            tile(buffer, hc, true);
            store[0] = nativePixmap(hc);
            store[1] = nativePixmap(shadow.copy(sz+1,0,sz,sz)); // TopRight
            buffer = shadow.copy(sz+1,sz,sz,1); // MidRight
            tile(buffer, vc, false);
            store[2] = nativePixmap(vc);
            store[3] = nativePixmap(shadow.copy(sz+1,sz+1,sz,sz)); // BtmRight
            buffer = shadow.copy(sz,sz+1,1,sz); // BtmMid
            tile(buffer, hc, true);
            store[4] = nativePixmap(hc);
            store[5] = nativePixmap(shadow.copy(0,sz+1,sz,sz)); // BtmLeft
            buffer = shadow.copy(0,sz,sz,1); // MidLeft
            tile(buffer, vc, false);
            store[6] = nativePixmap(vc);
            store[7] = nativePixmap(shadow.copy(0,0,sz,sz)); // TopLeft
        }
        for (int i = 0; i < 8; ++i)
            globalShadowData[t-1][i] = (*pixmaps[t-1])[i];

        data = &globalShadowData[t-1][0];
        if (storeToRoot)
            XProperty::set(DefaultRootWindow(BE_X11_DISPLAY), XProperty::bespinShadow[t-1], data, XProperty::LONG, 12);
    }
    return data;
#else
    return 0;  // just for GCC - makes no sense at all anyway
#endif
}

bool
Shadows::areSet(WId id)
{
    if (!BE::isPlatformX11())
        return false; // TODO: port XProperty for wayland
#ifdef BE_WS_X11
    XProperty::init();
    unsigned long _12 = 12;
    return XProperty::get<unsigned long>(id, XProperty::kwinShadow, XProperty::LONG, &_12);
#endif
    return false;
}

void
Shadows::cleanUp()
{
#ifdef BE_WS_X11
    delete shadowManager; shadowManager = 0;
    for (int i = 0; i < 2; ++i)
    {
        if (pixmaps[i])
        {
            for (int j = 0; j < 8; ++j)
                XFreePixmap(BE_X11_DISPLAY, (*pixmaps[i])[j]);
            delete [] pixmaps[i];
            pixmaps[i] = 0L;
        }
    }
#endif
}

void
Shadows::manage(QWidget *w)
{
#ifdef BE_WS_X11
    if (!BE::isPlatformX11())
        return; // TODO: port XProperty for wayland
    if (!shadowManager)
        shadowManager = new ShadowManager;
    w->removeEventFilter(shadowManager);
    w->installEventFilter(shadowManager);
#endif
}

void
Shadows::set(WId id, Shadows::Type t, bool storeToRoot)
{
#ifdef BE_WS_X11
    if (!BE::isPlatformX11())
        return; // TODO: port XProperty for wayland
    if (id == DefaultRootWindow(BE_X11_DISPLAY)) {
        qWarning("BESPIN WARNING! Setting shadow to ROOT window is NOT supported");
        return;
    }
    XProperty::init();
    switch(t)
    {
    case Shadows::None:
        XProperty::remove(id, XProperty::kwinShadow);
        break;
    case Shadows::Large:
    case Shadows::Small:
        XProperty::set(id, XProperty::kwinShadow, shadowData(t, storeToRoot), XProperty::LONG, 12);
    default:
        break;
    }
#endif
}

void Shadows::setColor(QColor c)
{
#ifdef BE_WS_X11
    color = c;
#endif
}

void Shadows::setHalo(bool h)
{
#ifdef BE_WS_X11
    halo = h;
#endif
}

void
Shadows::setSize(int small, int big)
{
#ifdef BE_WS_X11
    size[0] = qMin(72, qMax(8, small));
    size[1] = qMin(72, qMax(8, big));
#endif
}