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

#include "draw.h"

void
Style::drawCheckMark(const QStyleOption *option, QPainter *painter, Check::Type type) const
{
    // the checkmark (using brush)
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);
    bool isOn = option->state & QStyle::State_On;
    switch (type)
    {
    case Check::X:
    {
        const int   d = RECT.height()/8,
                    c = RECT.height()/2,
                    s = RECT.width(),
                    x = RECT.x(), y = RECT.y();
        if (isOn)
        {
            const QPoint points[8] =
            {
                QPoint(x+c,y+c-d), QPoint(x,y),
                QPoint(x+c-d,y+c), QPoint(x,y+s),
                QPoint(x+c,y+c+d), QPoint(x+s,y+s),
                QPoint(x+c+d,y+c), QPoint(x+s,y)
            };
            painter->drawPolygon(points, 8);
        }
        else
        {   // tristate
            const QPoint points[5] =
            {
                QPoint(x+c,y+c-d), QPoint(x,y), QPoint(x+c-d,y+c),
                QPoint(x+s,y+s), QPoint(x+c+d,y+c),
            };
            painter->drawPolygon(points, 5);
        }
        break;
    }
    default:
    case Check::V:
    {
        if (isOn)
        {
            const QPoint points[4] =
            {
                QPoint(RECT.right(), RECT.top()),
                QPoint(RECT.x()+RECT.width()/4, RECT.bottom()),
                QPoint(RECT.x(), RECT.bottom()-RECT.height()/2),
                QPoint(RECT.x()+RECT.width()/4, RECT.bottom()-RECT.height()/4)
            };
            painter->drawPolygon(points, 4);
        }
        else
        {   // tristate
            const int d = 2*RECT.height()/5;
            QRect r = RECT.adjusted(F(2),d,-F(2),-d);
            painter->drawRect(r);
        }
        break;
    }
    case Check::O:
    {
        const int d = RECT.height()/8;
        QRect r = RECT.adjusted(d,d,-d,-d);
        if (!isOn)
            r.adjust(0,r.height()/4,0,-r.height()/4);
        painter->drawRoundRect(r,70,70);
    }
    }
}

void
Style::drawCheck(const QStyleOption *option, QPainter *painter, const QWidget*, bool itemview) const
{
    if (const QStyleOptionViewItemV2 *item = qstyleoption_cast<const QStyleOptionViewItemV2 *>(option))
    if (!(item->features & QStyleOptionViewItemV2::HasCheckIndicator))
        return;

//       if (option->state & State_NoChange)
//          break;
    QStyleOption copy = *option;

    const int f2 = F(2);

    // storage
    SAVE_PAINTER(Pen|Brush|Alias);
    QBrush oldBrush = painter->brush();
    painter->setRenderHint(QPainter::Antialiasing);

    // rect -> square
    QRect r = RECT;
    if (r.width() > r.height())
        r.setWidth(r.height());
    else
        r.setHeight(r.width());

    // box (requires set pen for PE_IndicatorMenuCheckMark)
    painter->setBrush(Qt::NoBrush);
    QPalette::ColorRole fg = QPalette::Text, bg = QPalette::Base;

    if (itemview)
    {   // itemViewCheck
        r.adjust(f2, f2, -f2, -f2);
        if (!(option->state & State_Off))
            copy.state |= State_On;
        if (option->state & State_Selected)
            { fg = QPalette::HighlightedText; bg = QPalette::Highlight; }
        painter->setPen(FX::blend(COLOR(bg), COLOR(fg)));
    }

    if (appType != GTK)
    {
        if (painter->pen() != Qt::NoPen)
            { r.adjust(f2, f2, -f2, -f2); painter->drawRoundRect(r); }

        if (option->state & State_Off) // not checked, get out
        { RESTORE_PAINTER; return; }
    }
    else
    {
        copy.rect.adjust(F(1), F(5), -F(6), -F(2));
        oldBrush = painter->pen().brush();
        copy.state |= State_On;
    }

    // checkmark
    if (itemview)
        painter->setBrush(COLOR(fg));
    else
    {
        painter->setBrush(oldBrush);
        painter->setBrushOrigin(r.topLeft());
    }
    copy.rect.adjust(F(3),0,0,-F(3));
    drawCheckMark(&copy, painter, Check::V);
    RESTORE_PAINTER
}

/**static!*/ void
Style::drawExclusiveCheck(const QStyleOption *option, QPainter *painter, const QWidget *)
{
    SAVE_PAINTER(Brush|Alias);
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint ( QPainter::Antialiasing );
    painter->drawEllipse ( RECT );
    if (option->state & State_On)
    {
        painter->setBrush ( painter->pen().color() );
        const int dx = 3*RECT.width()/8, dy = 3*RECT.height()/8;
        painter->drawEllipse ( RECT.adjusted(dx, dy, -dx, -dy) );
    }
    RESTORE_PAINTER
}

#define DRAW_ARROW(_A_) if (painter->brush() == Qt::NoBrush) painter->drawArc(r, _A_*16, 180*16); else painter->drawChord(r, _A_*16, 180*16)

/**static!*/ void
Style::drawArrow(Navi::Direction dir, const QRect &rect, QPainter *painter, const QWidget*)
{
    // create an appropriate rect and move it to center of desired rect
    int s = qMin(rect.width(), rect.height());
//     if (!(s&1))
//         --s;
    QRect r( 0, 0, s, s );
    r.moveCenter(rect.center());
    SAVE_PAINTER(Alias);
    painter->setRenderHint(QPainter::Antialiasing, true);
    bool reset_pen = (painter->pen() == Qt::NoPen);
    if (reset_pen)
        painter->setPen(QPen(painter->brush(), 1));
    switch (dir)
    {
    case Navi::N:
        DRAW_ARROW(0);
        break;
    case Navi::E:
        DRAW_ARROW(-90);
        break;
    case Navi::S:
        DRAW_ARROW(-180);
        break;
    case Navi::W:
        DRAW_ARROW(90);
        break;
    case Navi::NW:
        DRAW_ARROW(45);
        break;
    case Navi::NE:
        DRAW_ARROW(-45);
        break;
    case Navi::SE:
        DRAW_ARROW(-135);
        break;
    case Navi::SW:
        DRAW_ARROW(135);
        break;
    }

    if (reset_pen)
        painter->setPen(Qt::NoPen);
    RESTORE_PAINTER
}

extern bool isUrlNaviButtonArrow;
/**static!*/ void
Style::drawSolidArrow(Navi::Direction dir, const QRect &rect, QPainter *painter, const QWidget *w)
{
    if (isUrlNaviButtonArrow)
    {
        if ( painter->brush() != Qt::NoBrush &&
             (!w || painter->brush().color().rgb() == w->palette().color(QPalette::HighlightedText).rgb()) &&
             painter->brush().color().alpha() < 255 )
            dir = (dir == Navi::W) ? Navi::SW : Navi::SE;
        if (w)
        {
            painter->setBrush(w->palette().color(w->foregroundRole()));
            painter->setPen(w->palette().color(w->foregroundRole()));
        }
    }
    bool hadNoBrush = painter->brush() == Qt::NoBrush;
    if (hadNoBrush)
        painter->setBrush(painter->pen().brush());
    drawArrow(dir, rect, painter, w);
    if (hadNoBrush)
        painter->setBrush(Qt::NoBrush);
}
