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

#include <QPainter>
#include <QWidget>
#include <QVarLengthArray>
#include <QtDebug>

#include <cmath>
#include <stdio.h>
#include "FX.h"

#ifdef BE_WS_X11
#if QT_VERSION >= 0x060200
#include <QGuiApplication>
#else
#include <QX11Info>
#endif
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "xproperty.h"
#include "fixx11h.h"
namespace BE { namespace FX { static Atom net_wm_cm; } }
#endif

using namespace BE;

static float ratioFor(int step, int maxSteps) {
#if 1
    static QVarLengthArray<float> table;
    const int n = maxSteps + 1;
    if (table.size() != n) {
        table.resize(n);
        const float accel = 0.5f;
        const float factor = pow(0.5f, accel);
        for (int i = (n+1)/2; i < n; ++i) {
            float ratio = i/float(maxSteps);
            if (ratio == 0.5f)
                table[i] = 0.f;
            else
                table[i] = (pow(ratio - 0.5f, accel) + factor) / (2.0f*factor);
        }
        for (int i = 0; i <= n/2; ++i) {
            table[i] = 1.0f - table[maxSteps-i];
        }
    }

    return table.at(step);
#else
    return step/float(MAX_STEPS);
#endif
}

QPixmap &
FX::scaledIcon(QPixmap &pix, int step, int maxSteps, int pad)
{
    const float ratio = ratioFor(step, maxSteps);
    static QPixmap scaledIcon[2], emptyIcon;
    static qint64 lastIconPix[2] = {0, 0};
    int idx = pix.cacheKey() == lastIconPix[0] ? 0 : (pix.cacheKey() == lastIconPix[1] ? 1 : -1);
    if (idx < 0) {
        idx = 1;
        if (lastIconPix[1]) {
            scaledIcon[0] = scaledIcon[1];
            lastIconPix[0] = lastIconPix[1];
        }
        scaledIcon[1] = pix.scaledToHeight(pix.height() + 2*pad, Qt::SmoothTransformation);
        lastIconPix[1] = pix.cacheKey();
    }

    if (step == maxSteps)
        return scaledIcon[idx];

    if (emptyIcon.size() != scaledIcon[idx].size()) {
        emptyIcon = QPixmap(scaledIcon[idx].size());
    }
    emptyIcon.fill(Qt::transparent);

    FX::blend(pix, emptyIcon, 1.0, pad, pad);
    FX::blend(scaledIcon[idx], emptyIcon, ratio);
    return emptyIcon;
}

QPixmap &
FX::tintedIcon(QPixmap &pix, int step, int maxSteps, QColor tint)
{
    const float ratio = ratioFor(step, maxSteps);
    static QPixmap tintedIcon[2], emptyIcon;
    static qint64 lastIconPix[2] = {0, 0};
    int idx = pix.cacheKey() == lastIconPix[0] ? 0 : (pix.cacheKey() == lastIconPix[1] ? 1 : -1);
    if (idx < 0) {
        idx = 1;
        if (lastIconPix[1]) {
            tintedIcon[0] = tintedIcon[1];
            lastIconPix[0] = lastIconPix[1];
        }
        QImage img = pix.toImage().convertToFormat(QImage::Format_ARGB32);
        int size = img.width() * img.height();
        QRgb *pixel = (QRgb*)img.bits();
        const int r = tint.red(), g = tint.green(), b = tint.blue();
        int minV = 255, maxV = 0;
        bool mono = true;
        for (int i = 0; i < size; ++i) {
            if (qAlpha(*pixel) > 24) {
                const int v = qGray(*pixel);
                maxV = qMax(v, maxV);
                minV = qMin(v, minV);
                if (maxV - minV > 10) {
                    mono = false;
                    break;
                }
            }
            ++pixel;
        }
        pixel = (QRgb*)img.bits();
        if (mono) {
            for (int i = 0; i < size; ++i) {
                if (int a = qAlpha(*pixel)) {
                    *pixel = qRgba(r, g, b, a);
                }
                ++pixel;
            }
        } else {
            for (int i = 0; i < size; ++i) {
                if (int a = qAlpha(*pixel)) {
                    const int v = qGray(*pixel);
                    // stretch alpha
                    a = 255 - v*a/255;
                    a = 255 - a*a/255;
                    *pixel = qRgba(r, g, b, a);
                }
                ++pixel;
            }
        }
        tintedIcon[1] = QPixmap::fromImage(img);
        lastIconPix[1] = pix.cacheKey();
    }

    if (step == maxSteps)
        return tintedIcon[idx];

    if (emptyIcon.size() != pix.size()) {
        emptyIcon = QPixmap(pix.size());
    }
    emptyIcon.fill(Qt::transparent);

    FX::blend(pix, emptyIcon, 1.0-ratio);
    FX::blend(tintedIcon[idx], emptyIcon, ratio);
    return emptyIcon;
}

/*
// Exponential blur, Jani Huhtanen, 2006 ==========================
*  expblur(QImage &img, int radius)
*
*  In-place blur of image 'img' with kernel of approximate radius 'radius'.
*  Blurs with two sided exponential impulse response.
*
*  aprec = precision of alpha parameter in fixed-point format 0.aprec
*  zprec = precision of state parameters zR,zG,zB and zA in fp format 8.zprec
*/

template<int aprec, int zprec>
static inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
    int R,G,B,A;
    R = *bptr;
    G = *(bptr+1);
    B = *(bptr+2);
    A = *(bptr+3);

    zR += (alpha * ((R<<zprec)-zR))>>aprec;
    zG += (alpha * ((G<<zprec)-zG))>>aprec;
    zB += (alpha * ((B<<zprec)-zB))>>aprec;
    zA += (alpha * ((A<<zprec)-zA))>>aprec;

    *bptr =     zR>>zprec;
    *(bptr+1) = zG>>zprec;
    *(bptr+2) = zB>>zprec;
    *(bptr+3) = zA>>zprec;
}

template<int aprec,int zprec>
static inline void blurrow( QImage & im, int line, int alpha)
{
    int zR,zG,zB,zA;

    QRgb *ptr = (QRgb *)im.scanLine(line);

    zR = *((unsigned char *)ptr    )<<zprec;
    zG = *((unsigned char *)ptr + 1)<<zprec;
    zB = *((unsigned char *)ptr + 2)<<zprec;
    zA = *((unsigned char *)ptr + 3)<<zprec;

    for(int index=1; index<im.width(); index++)
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);

    for(int index=im.width()-2; index>=0; index--)
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
}

template<int aprec, int zprec>
static inline void blurcol( QImage & im, int col, int alpha)
{
    int zR,zG,zB,zA;

    QRgb *ptr = (QRgb *)im.bits();
    ptr+=col;

    zR = *((unsigned char *)ptr    )<<zprec;
    zG = *((unsigned char *)ptr + 1)<<zprec;
    zB = *((unsigned char *)ptr + 2)<<zprec;
    zA = *((unsigned char *)ptr + 3)<<zprec;

    for(int index=im.width(); index<(im.height()-1)*im.width(); index+=im.width())
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);

    for(int index=(im.height()-2)*im.width(); index>=0; index-=im.width())
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
}

void
FX::expblur(QImage &img, int radius, Qt::Orientations o)
{
    if(radius<1)
        return;

    static const int aprec = 16; static const int zprec = 7;

    // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends to infinity)
    int alpha = (int)((1<<aprec)*(1.0f-expf(-2.3f/(radius+1.f))));

    if (o & Qt::Horizontal) {
        for(int row=0;row<img.height();row++)
            blurrow<aprec,zprec>(img,row,alpha);
    }

    if (o & Qt::Vertical) {
        for(int col=0;col<img.width();col++)
            blurcol<aprec,zprec>(img,col,alpha);
    }
}
// ======================================================

// stolen from KWindowSystem
bool FX::compositingActive()
{
#ifdef BE_WS_X11
    if (!BE::isPlatformX11())
        return true; // we assume wayland - or any other compositing capable display server
    return XGetSelectionOwner( BE_X11_DISPLAY, BE::FX::net_wm_cm ) != None;
#else
    return true;
#endif
}

static bool useRaster = false;

void
FX::init()
{
#ifdef BE_WS_X11
    if (!BE::isPlatformX11())
        return;
    Display *dpy = BE_X11_DISPLAY;
    char string[ 100 ];
    sprintf(string, "_NET_WM_CM_S%d", DefaultScreen( dpy ));
    BE::FX::net_wm_cm = XInternAtom(dpy, string, False);
#endif
}

bool
FX::blend(const QPixmap &upper, QPixmap &lower, double opacity, int x, int y)
{
    if (opacity == 0.0)
        return false; // haha...

    {
        QPixmap tmp;
        if ( useRaster ) // raster engine is broken... :-(
        {
            tmp = QPixmap(upper.size());
            tmp.fill(Qt::transparent);
            QPainter p(&tmp);
            p.drawPixmap(0,0, upper);
            p.end();
        }
        else
            tmp = upper;

        QPainter p;
        if (opacity < 1.0)
        {
            p.begin(&tmp);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.fillRect(tmp.rect(), QColor(0,0,0, opacity*255.0));
            p.end();
        }
        p.begin(&lower);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawPixmap(x, y, tmp);
        p.end();
    }
   return true;
}

QPixmap
FX::applyAlpha(const QPixmap &toThisPix, const QPixmap &fromThisPix, const QRect &rect, const QRect &alphaRect)
{
    QPixmap pix;
    int sx,sy,ax,ay,w,h;
    if (rect.isNull())
        { sx = sy = 0; w = toThisPix.width(); h = toThisPix.height(); }
    else
        rect.getRect(&sx,&sy,&w,&h);
    if (alphaRect.isNull())
        { ax = ay = 0; }
    else
    {
        ax = alphaRect.x(); ay = alphaRect.y();
        w = qMin(alphaRect.width(),w); h = qMin(alphaRect.height(),h);
    }

    if (w > fromThisPix.width() || h > fromThisPix.height())
        pix = QPixmap(w, h);
    else
        pix = fromThisPix.copy(0,0,w,h); // cause slow depth conversion...
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.drawPixmap(0, 0, toThisPix, sx, sy, w, h);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawPixmap(0, 0, fromThisPix, ax, ay, w, h);
    p.end();
    return pix;
}

#if 1
// taken from QCommonStyle generatedIconPixmap - why oh why cannot KDE apps take this function into account...
static inline uint qt_intensity(uint r, uint g, uint b)
{
    // 30% red, 59% green, 11% blue
    return (77 * r + 150 * g + 28 * b) / 255;
}

void
FX::desaturate(QImage &img, const QColor &c)
{
    int r,g,b; c.getRgb(&r, &g, &b);
    uchar reds[256], greens[256], blues[256];

    for (int i=0; i<128; ++i)
    {
        reds[i]   = uchar((r * (i<<1)) >> 8);
        greens[i] = uchar((g * (i<<1)) >> 8);
        blues[i]  = uchar((b * (i<<1)) >> 8);
    }
    for (int i=0; i<128; ++i)
    {
        reds[i+128]   = uchar(qMin(r   + (i << 1), 255));
        greens[i+128] = uchar(qMin(g + (i << 1), 255));
        blues[i+128]  = uchar(qMin(b + (i << 1), 255));
    }

    int intensity = qt_intensity(r, g, b);
    const int f = 191;

    if ((r - f > g && r - f > b) || (g - f > r && g - f > b) || (b - f > r && b - f > g))
        intensity = qMin(255, intensity + 91);
    else if (intensity <= 128)
        intensity -= 51;

    for (int y = 0; y < img.height(); ++y)
    {
        QRgb *scanLine = (QRgb*)img.scanLine(y);
        for (int x = 0; x < img.width(); ++x)
        {
            QRgb pixel = *scanLine;
            uint ci = uint(qGray(pixel)/3 + (130 - intensity / 3));
            *scanLine = qRgba(reds[ci], greens[ci], blues[ci], qAlpha(pixel));
            ++scanLine;
        }
    }
}
#endif


QImage
FX::newDitherImage(uint intensity, uint size)
{
    QImage img(size,size, QImage::Format_ARGB32);
    size = size*size;
    QRgb *pixel = (QRgb*)img.bits();
    int a, v;
    for (uint i = 0; i < size; ++i) // 32*32...
    {
        a = (rand() % intensity)/2;
        v = (a%2)*255;
        *pixel = qRgba(v,v,v,a);
        ++pixel;
    }
    return img;
}
const QPixmap &
FX::dither()
{
    static QPixmap _dither;
    if (_dither.isNull())
        _dither = QPixmap::fromImage(newDitherImage());
    return _dither;
}

QPixmap
FX::tint(const QPixmap &mask, const QColor &color)
{
    QPixmap pix = mask.copy();
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setPen(Qt::NoPen); p.setBrush(color);
    p.drawRect(pix.rect());
    p.end();
    pix = FX::applyAlpha(pix, mask);

    return pix;
}


QPixmap
FX::fade(const QPixmap &pix, double percent)
{
    QPixmap newPix(pix.size());
    newPix.fill(Qt::transparent);
    blend(pix, newPix, percent);
    return newPix;
}


#define CLAMP(x,l,u) (x) < (l) ? (l) :\
(x) > (u) ? (u) :\
(x)


int
FX::contrastOf(const QColor &a, const QColor &b)
{
    int ar,ag,ab,br,bg,bb;
    a.getRgb(&ar,&ag,&ab);
    b.getRgb(&br,&bg,&bb);

    int diff = 299*(ar-br) + 587*(ag-bg) + 114*(ab-bb);
    diff = (diff < 0) ? -diff : 90*diff/100;
    int perc = diff / 2550;

    diff = qMax(ar,br) + qMax(ag,bg) + qMax(ab,bb)
        - (qMin(ar,br) + qMin(ag,bg) + qMin(ab,bb));

    perc += diff/765;
    perc /= 2;

    return perc;
}

QPalette::ColorRole
FX::counter(QPalette::ColorRole role)
{
    switch (role) {
    case QPalette::ButtonText: //8
        return QPalette::Button;
    case QPalette::WindowText: //0
        return QPalette::Window;
    case QPalette::HighlightedText: //13
        return QPalette::Highlight;
    case QPalette::Window: //10
        return QPalette::WindowText;
    case QPalette::Base: //9
        return QPalette::Text;
    case QPalette::Text: //6
        return QPalette::Base;
    case QPalette::Highlight: //12
        return QPalette::HighlightedText;
    case QPalette::Button: //1
        return QPalette::ButtonText;
    default:
        return QPalette::Window;
    }
}

bool
FX::counter(QPalette::ColorRole &from, QPalette::ColorRole &to, QPalette::ColorRole defFrom, QPalette::ColorRole defTo)
{
    switch (from) {
    case QPalette::WindowText: //0
        to = QPalette::Window; break;
    case QPalette::Window: //10
        to = QPalette::WindowText; break;
    case QPalette::Base: //9
        to = QPalette::Text; break;
    case QPalette::Text: //6
        to = QPalette::Base; break;
    case QPalette::Button: //1
        to = QPalette::ButtonText; break;
    case QPalette::ButtonText: //8
        to = QPalette::Button; break;
    case QPalette::Highlight: //12
        to = QPalette::HighlightedText; break;
    case QPalette::HighlightedText: //13
        to = QPalette::Highlight; break;
    default:
        from = defFrom;
        to = defTo;
        return false;
    }
    return true;
}

bool
FX::haveContrast(const QColor &a, const QColor &b)
{
    int ar,ag,ab,br,bg,bb;
    a.getRgb(&ar,&ag,&ab);
    b.getRgb(&br,&bg,&bb);

    /*int diff = (299*(ar-br) + 587*(ag-bg) + 114*(ab-bb));

    if (qAbs(diff) < 91001)
        return false;*/

    int diff = (299*qAbs(ar - br) + 587*qAbs(ag - bg) + 114*qAbs(ab - bb)) / 300;

    return (diff > 250);
}

QColor
FX::blend(const QColor &c1, const QColor &c2, int w1, int w2)
{
    int sum = (w1+w2);
    if (!sum)
       return Qt::black;

    int r,g,b,a;
#if 0
    QColor c1 = oc1;
    b = value(c1);
    if (b < 70)
    {
        c1.getHsv(&r,&g,&b,&a);
        c1.setHsv(r,g,70,a);
    }
#endif
    r = (w1*c1.red() + w2*c2.red())/sum; r = CLAMP(r,0,255);
    g = (w1*c1.green() + w2*c2.green())/sum; g = CLAMP(g,0,255);
    b = (w1*c1.blue() + w2*c2.blue())/sum; b = CLAMP(b,0,255);
    a = (w1*c1.alpha() + w2*c2.alpha())/sum; a = CLAMP(a,0,255);
    return QColor(r,g,b,a);
}

int
FX::value(const QColor &c)
{
    int v = c.red();
    if (c.green() > v) v = c.green();
    if (c.blue() > v) v = c.blue();
    return v;
}

void 
FX::swap(QPalette &pal, QPalette::ColorGroup group, QPalette::ColorRole r1, QPalette::ColorRole r2, const QWidget *w)
{
    QColor c1 = pal.color(group, r1);
    if (w && c1 == Qt::transparent && r1 == w->backgroundRole()) {
        qDebug() << "Qt::transparent abused as ::setAutoFillBackground(false)" << w->parentWidget() << ">" << w;
        const QWidget *runner = w;
        while ((runner = runner->parentWidget())) {
            if (runner->autoFillBackground() || runner->testAttribute(Qt::WA_OpaquePaintEvent) || runner->isWindow()) {
                c1 = runner->palette().color(runner->backgroundRole());
                if (c1.alpha()) {
                    break;
                }
            }
        }
    }

    QColor c2 = pal.color(group, r2);
    const int alpha = c1.alpha();
    c1.setAlpha(c2.alpha());
    c2.setAlpha(alpha);
    if (r1 == QPalette::Highlight || r2 == QPalette::Highlight) {
        int h,s,v;
        int r,g,b;
        c1.getHsv(&h, &s, &v);
        c1.getRgb(&r, &g, &b);
        bool swapOther = v < 128;
        v = qMax(qMax(255-r,255-g),255-b);
        swapOther = (swapOther != (v < 128));
        c1.setHsv(h,s,v,c1.alpha());
        pal.setColor(group, r1, c1);

        if (swapOther) {
            c2.getHsv(&h, &s, &v);
            c2.getRgb(&r, &g, &b);
            v = qMax(qMax(255-r,255-g),255-b);
            c2.setHsv(h,s,v,c2.alpha());
            pal.setColor(group, r2, c2);
        }
    } else {
        pal.setColor(group, r1, c2);
        pal.setColor(group, r2, c1);
    }
}
