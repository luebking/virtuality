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

#ifndef BE_SHADOWS_H
#define BE_SHADOWS_H

class QWidget;

namespace BE {
namespace Shadows {
    enum BLIB_EXPORT Type { None = 0, Small, Large };
    BLIB_EXPORT bool areSet(WId id);
    BLIB_EXPORT void cleanUp();
    BLIB_EXPORT void manage(QWidget *w);
    BLIB_EXPORT void set(WId id, Shadows::Type t, bool storeToRoot = false);
    BLIB_EXPORT void setColor(QColor c);
    BLIB_EXPORT void setHalo(bool halo);
    BLIB_EXPORT void setSize(int small, int big);
} }

#endif // BESPIN_SHADOWS_H
