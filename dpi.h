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

namespace BE {
namespace Dpi {

typedef struct BLIB_EXPORT
{
    int f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f12, f13, f16, f17, f32, f18, f20, f80, f22, f64;
} Target;
extern BLIB_EXPORT Target target;

}
}
