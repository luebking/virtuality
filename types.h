/*
 *   Virtuality Style for Qt4 and Qt5
 *   Copyright 2009-2014 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef BESPIN_TYPES
#define BESPIN_TYPES

namespace BE {

namespace Check {
    enum Type {X = 0, V, O};
}

namespace Navi {
enum Direction {
   N = Qt::UpArrow, S = Qt::DownArrow,
   E = Qt::RightArrow, W = Qt::LeftArrow,
   NW = 5, NE, SE, SW
   };
}

enum BGMode { Plain = 0, Scanlines, BevelV, BevelH };

enum Orientation3D {Raised = 0, Relief, Sunken, Inlay};

enum AppType
{
    Unknown, GTK, QtDesigner, Plasma, KDM, KRunner, Dolphin, Opera, BEshell, Arora, KWin,
    KDevelop, Konversation, Amarok, OpenOffice, VLC, KMail, Konqueror, Gwenview, QBittorrent, Falkon
};

namespace Groove {
enum Mode { Line = 0, Groove, Sunken, SunkenGroove, None };
}
}

#endif // BESPIN_TYPES
