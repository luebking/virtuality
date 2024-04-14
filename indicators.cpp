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


#define DRAW_SEGMENT(_R_, _START_, _SPAN_) sunken ? painter->drawEllipse(_R_) : painter->drawArc(_R_, _START_, _SPAN_)

void
Style::drawCheck(const QStyleOption *option, QPainter *painter, const QWidget *, bool exclusive, bool itemview) const
{
    Qt::CheckState state = (option->state & State_On) ? Qt::Checked : Qt::Unchecked;
    if (itemview) {
        if (const QStyleOptionViewItem *item = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
            if (!(item->features & QStyleOptionViewItem::HasCheckIndicator))
                return;
            state = item->checkState;
        }
    }

    OPT_SUNKEN OPT_HOVER

    SAVE_PAINTER(Pen|Brush|Alias);
    painter->setRenderHint(QPainter::Antialiasing);

    // rect -> square
    STROKED_RECT(r, RECT);
    if (r.width() > r.height())
        r.setWidth(r.height());
    else
        r.setHeight(r.width());
    r.adjust(F(1), F(1), -F(1), -F(1));
    if (option->direction == Qt::LeftToRight)
        r.moveRight(RECT.right() - (halfStroke + F(1)));
    else
        r.moveLeft(RECT.left() + (halfStroke + F(1)));
    r.moveTop(RECT.top() + 0.5*(RECT.height() - r.height()));

    if (itemview) { // itemViewCheck
        if (!exclusive)
            r.translate(0, -r.height()/4);
        if (option->state & State_Selected)
            painter->setPen(FCOLOR(HighlightedText));
        else if (hover) // not necessarily selectable...
            painter->setPen(FX::blend(FCOLOR(Highlight), FCOLOR(HighlightedText)));
        else
            painter->setPen(FX::blend(FCOLOR(Base), FCOLOR(Text)));
    }

    painter->setPen(QPen(painter->pen().color(), FRAME_STROKE));
    painter->setBrush(Qt::NoBrush);
    if (exclusive) {
        if (option->direction == Qt::LeftToRight)
            DRAW_SEGMENT(r, -16*96, 16*192);
        else
            DRAW_SEGMENT(r, 16*84, 16*192);
    } else {
        DRAW_SEGMENT(r, 16*6, -16*192);
    }

    if (state != Qt::Unchecked) { // the drop
        painter->setBrush(painter->pen().color());
        painter->setPen(Qt::NoPen);
        const int d = intMin(F(4), r.height()/3);
        r.adjust(d, d, -d, -d);
        if (state == Qt::PartiallyChecked) // tri-state
            painter->drawChord(r, -180*16, 180*16);
        else
            painter->drawEllipse(r);
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

    switch (dir) {
        case Navi::N: DRAW_ARROW(0); break;
        case Navi::E: DRAW_ARROW(-90); break;
        case Navi::S: DRAW_ARROW(-180); break;
        case Navi::W: DRAW_ARROW(90); break;
        case Navi::NW: DRAW_ARROW(45); break;
        case Navi::NE: DRAW_ARROW(-45); break;
        case Navi::SE: DRAW_ARROW(-135); break;
        case Navi::SW: DRAW_ARROW(135); break;
    }

    if (reset_pen)
        painter->setPen(Qt::NoPen);
    RESTORE_PAINTER
}

extern bool isUrlNaviButtonArrow;
/**static!*/ void
Style::drawSolidArrow(Navi::Direction dir, const QRect &rect, QPainter *painter, const QWidget *w)
{
    if (isUrlNaviButtonArrow) {
        if ( painter->brush() != Qt::NoBrush &&
             (!w || painter->brush().color().rgb() == w->palette().color(QPalette::HighlightedText).rgb()) &&
             painter->brush().color().alpha() < 255 )
            dir = (dir == Navi::W) ? Navi::SW : Navi::SE;
        if (w) {
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
