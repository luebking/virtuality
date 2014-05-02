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

#ifndef BE_FX_H
#define BE_FX_H

class QWidget;
#include <QColor>
#include <QPalette>
#include <QPixmap>

namespace BE {
namespace FX {
    BLIB_EXPORT void init();
    BLIB_EXPORT bool compositingActive();
    BLIB_EXPORT bool blend(const QPixmap &upper, QPixmap &lower, double opacity = 0.5, int x = 0, int y = 0);
    BLIB_EXPORT void desaturate(QImage &img, const QColor &c);
    BLIB_EXPORT QImage newDitherImage(uint intensity = 6, uint size = 32);
    BLIB_EXPORT const QPixmap &dither();
    BLIB_EXPORT QPixmap fade(const QPixmap &pix, double percent);
    BLIB_EXPORT QPixmap tint(const QPixmap &mask, const QColor &color);
    BLIB_EXPORT QPixmap applyAlpha( const QPixmap &toThisPix, const QPixmap &fromThisPix, const QRect &rect = QRect(), const QRect &alphaRect = QRect());
    BLIB_EXPORT void expblur(QImage &img, int radius, Qt::Orientations o = Qt::Horizontal|Qt::Vertical );

    BLIB_EXPORT int contrastOf(const QColor &a, const QColor &b);
    BLIB_EXPORT QPalette::ColorRole counter(QPalette::ColorRole role);
    BLIB_EXPORT bool counter(QPalette::ColorRole &from, QPalette::ColorRole &to,
                             QPalette::ColorRole defFrom = QPalette::WindowText,
                             QPalette::ColorRole defTo = QPalette::Window);
    BLIB_EXPORT bool haveContrast(const QColor &a, const QColor &b);
    BLIB_EXPORT QColor blend(const QColor &oc1, const QColor &c2, int w1 = 1, int w2 = 1);
    BLIB_EXPORT int value(const QColor &c);
    BLIB_EXPORT void swap(QPalette &pal, QPalette::ColorGroup group, QPalette::ColorRole r1, QPalette::ColorRole r2, const QWidget *w = 0);
}
}

#endif //BE_FX_H
