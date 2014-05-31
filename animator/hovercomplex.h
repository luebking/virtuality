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

#ifndef HOVER_COMPLEX_ANIMATOR_H
#define HOVER_COMPLEX_ANIMATOR_H

#include <QStyle>
#include "hoverindex.h"

namespace Animator {

class ComplexInfo
{
    public:
        ComplexInfo() {
            active = fades[In] = fades[Out] = QStyle::SC_None;
        }
        QStyle::SubControls active, fades[2];
        inline int step(QStyle::SubControl sc) const {return steps.value(sc);}
    private:
        friend class HoverComplex;
        QMap<QStyle::SubControl, int> steps;
};

class HoverComplex : public HoverIndex
{
    public:
        static const ComplexInfo *info(const QWidget *widget, QStyle::SubControls active);
        static void setDuration(uint ms);
        static void setFPS(uint fps);
    protected:
        void timerEvent(QTimerEvent * event);
        typedef QMap<WidgetPtr, ComplexInfo> Items;
        Items items;
    private:
        const ComplexInfo *_info(const QWidget *widget, QStyle::SubControls active) const;
        Q_DISABLE_COPY(HoverComplex)
        HoverComplex() {}
};

} // namespace

#endif // HOVER_COMPLEX_ANIMATOR_H

