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

#ifndef BESPIN_DRAW_H
#define BESPIN_DRAW_H

#include <cmath>
#include <QPainter>

#include "FX.h"
#include "virtuality.h"
#include "makros.h"

using namespace BE;

#define OPT_SUNKEN bool sunken = option->state & State_Sunken;
#define OPT_ENABLED bool isEnabled = option->state & State_Enabled;
#define OPT_HOVER bool hover = (option->state & State_Enabled) && (option->state & State_MouseOver);
#define OPT_FOCUS bool hasFocus = option->state & State_HasFocus;

#define B_STATES OPT_SUNKEN OPT_ENABLED OPT_HOVER OPT_FOCUS

#define BESPIN_MNEMONIC config.mnemonic

#define FOCUS_FRAME_PEN QPen(FX::blend(FCOLOR(Window), FCOLOR(Highlight), 1, 2), F(2) & ~1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)
#define FRAME_PEN QPen(FX::blend(FCOLOR(Window), FCOLOR(WindowText), 8, 1), F(2) & ~1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)

#define THERMOMETER_COLOR FX::blend(FCOLOR(Window), FCOLOR(WindowText), 1, 4)
#define GROOVE_COLOR FX::blend(FCOLOR(Window), FCOLOR(WindowText), 8, 1)

static inline void
setBold(QPainter *p, const QString &text = QString(), int w = -1)
{
    if (p->font().pointSize() < 1)  // ignore pixelsize fonts for reports of bad visual quality (#2787384)
        return;

    QFont fnt = p->font();
    fnt.setBold(true);
    if (!text.isEmpty())
    {
        int wb = QFontMetrics(fnt).size(Qt::TextShowMnemonic, text).width();
        if (wb > w)
        {
            int wd = qMax(w-F(4), QFontMetrics(p->font()).size(Qt::TextShowMnemonic, text).width());
            fnt.setStretch(qRound(wd*100.0/wb));
        }
    }
    p->setFont(fnt);
}

static inline void
setTitleFont(QPainter *p, const QString &text = QString(), int w = -1 )
{
    if (p->font().pointSize() < 1) // ignore pixelsize fonts for reports of bad visual quality (#2787384)
        return;

    QFont fnt = p->font();
    fnt.setWeight(QFont::Black);
//         fnt.setUnderline(true);
//         fnt.setStretch(QFont::SemiExpanded);
//     fnt.setCapitalization(QFont::AllUppercase/*SmallCaps*/);
//     fnt.setCapitalization(QFont::SmallCaps);
    if (text.isEmpty())
        fnt.setPointSize(9*fnt.pointSize()/10);
    else
    {
        int wb = QFontMetrics(fnt).size(Qt::TextShowMnemonic, text).width();
        if (wb > w)
        {
            int wd = qMax(w-F(4), QFontMetrics(p->font()).size(Qt::TextShowMnemonic, text).width());
            fnt.setStretch(qRound(wd*100.0/wb));
        }
    }

    p->setFont(fnt);
}

#endif // BESPIN_DRAW_H
