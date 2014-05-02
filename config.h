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

#ifndef BESPIN_CONFIG
#define BESPIN_CONFIG

#include "types.h"

namespace BE
{

typedef struct Config
{
    struct invert {
        bool docks, menubars, menus, modals, titlebars, toolbars;
    } invert;

    struct {
        bool buttons;
        int thickness;
    } slider;
    
    struct {
        int roundness;
    } frame;

    struct bg
    {
        bool ringOverlay;
        int structure;
        struct {
            int opacity;
        } modal;
    } bg;

    struct btn
    {
        int minHeight;
        struct tool {
            int disabledStyle;
        } tool;
    } btn;

    int  dialogBtnLayout;
    bool fadeInactive;
    int fontExtent;
    int fontOffset[2];

    struct input
    {
        ushort pwEchoChar;
    } input;

    Qt::LayoutDirection leftHanded;

    bool macStyle;

    struct menu
    {
        bool showIcons;
        int delay;
    } menu;

    int mnemonic;

    int winBtnStyle;

    float scale;

} Config;

} // namespace

#endif // BESPIN_CONFIG
