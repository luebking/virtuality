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

#include <QApplication>
#include <QAbstractItemView>
#include <QListView>
#include <QPainterPath>
#include <QPushButton>
#include <QTreeView>
#include "draw.h"

#include <QtDebug>

void
Style::drawHeader(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(header, Header);

    if (appType == GTK)
        const_cast<QStyleOption*>(option)->palette = qApp->palette();

    drawHeaderSection(header, painter, widget);
    drawHeaderLabel(header, painter, widget);

    // sort Indicator on sorting or (inverted) on hovered headers
    if (header->sortIndicator != QStyleOptionHeader::None) {
        QStyleOptionHeader subopt = *header;
        subopt.rect = subElementRect(SE_HeaderArrow, option, widget);
        drawHeaderArrow(&subopt, painter, widget);
    }

//    painter->setClipRegion(clipRegion);
}

void
Style::drawHeaderSection(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    OPT_SUNKEN OPT_HOVER
    SAVE_PAINTER(Pen|Brush|Alias);
    const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option);
    const bool sorting = header && (header->sortIndicator != QStyleOptionHeader::None);
    char corners = 0;
    const bool l2r = option->direction == Qt::LeftToRight;
    if (header) {
        if (header->position == QStyleOptionHeader::Beginning || header->position == QStyleOptionHeader::OnlyOneSection)
            corners |= (l2r ? Corner::TopLeft : Corner::TopRight);
        if (header->position == QStyleOptionHeader::End || header->position == QStyleOptionHeader::OnlyOneSection) {
            if (header->orientation == Qt::Horizontal)
                corners |= (l2r ? Corner::TopRight : Corner::TopLeft);
            else
                corners |= (l2r ? Corner::BottomLeft : Corner::BottomRight);
        }
    }

    if (appType == GTK)
        sunken = option->state & State_HasFocus;

    QColor c =  widget ? PAL.color(widget->backgroundRole()) : (config.invert.headers ? FCOLOR(Text) : FCOLOR(Base));

    painter->setRenderHint(QPainter::Antialiasing, false);

    bool paintBg = config.invert.headers;
    if (!config.invert.headers) {
        if (sunken) {
            paintBg = true;
            c = FCOLOR(Highlight);
        } else if (hover) {
            paintBg = true;
            c = FX::blend(FCOLOR(Base), FCOLOR(Highlight), 3, 1);
        } else {
            painter->setPen(QPen(FRAME_COLOR, FRAME_STROKE_WIDTH));
            STROKED_RECT(r, RECT);
            if (header->orientation == Qt::Horizontal)
                painter->drawLine(r.bottomLeft(), r.bottomRight());
            else if (l2r)
                painter->drawLine(r.topRight(), r.bottomRight());
            else
                painter->drawLine(r.topLeft(), r.bottomLeft());
            RESTORE_PAINTER
        }
    } else {
        if (sorting || sunken) {
            if ((option->state & State_On) || !FX::haveContrast(c, FCOLOR(Highlight)))
                c = FCOLOR(Highlight);
        }
        if (!sunken && hover) {
            c = FX::blend(c, sorting ? FCOLOR(HighlightedText) : FCOLOR(Highlight),8,1);
        }
    }

    if (paintBg) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);

        QPainterPath path;
        const int rnd(config.frame.roundness);
        const int rnd_2 = 2*rnd;
        const QRect r(RECT.adjusted(0,0,1,1));
        if (corners & Corner::BottomRight) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            path.moveTo(r.right(), r.bottom() - rnd_2);
            path.arcTo(r.right() - rnd_2, r.bottom() - rnd_2, rnd_2, rnd_2, 0, -90);
        } else {
            path.moveTo(r.bottomRight());
        }
        if (corners & Corner::BottomLeft) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            path.lineTo(r.x() + rnd, r.bottom());
            path.arcTo(r.x(), r.bottom() - rnd_2, rnd_2, rnd_2, -90, -90);
        } else {
            path.lineTo(r.bottomLeft());
        }
        if (corners & Corner::TopLeft) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            path.lineTo(r.x(), r.y() + rnd);
            path.arcTo(r.x(), r.y(), rnd_2, rnd_2, 180, -90);
        } else {
            path.lineTo(r.topLeft());
        }
        if (corners & Corner::TopRight) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            path.lineTo(r.right() - rnd, r.y());
            path.arcTo(r.right() - rnd_2, r.y(), rnd_2, rnd_2, 90, -90);
        } else {
            path.lineTo(r.topRight());
        }
        path.closeSubpath();
        painter->drawPath(path);
    }

    if (!header || header->position < QStyleOptionHeader::End) {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(FX::blend(c, config.invert.headers ? FCOLOR(Base) : FCOLOR(Text), 6, 1));
        if (header && header->orientation == Qt::Vertical)
            painter->drawLine(RECT.bottomLeft(), RECT.bottomRight());
        else if (option->direction == Qt::LeftToRight)
            painter->drawLine(RECT.topRight(), RECT.bottomRight());
        else
            painter->drawLine(RECT.topLeft(), RECT.bottomLeft());
    }
    RESTORE_PAINTER
}

void
Style::drawHeaderLabel(const QStyleOption * option, QPainter * painter, const QWidget *widget) const
{
    ASSURE_OPTION(header, Header);
    OPT_ENABLED OPT_SUNKEN

    QRect rect = widget ? RECT.intersected(widget->rect()) : RECT;

    // iconos
    if ( !header->icon.isNull() ) {
        QPixmap pixmap = header->icon.pixmap( 22,22, isEnabled ? QIcon::Normal : QIcon::Disabled );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        // "pixh - 1" because of tricky integer division
        rect.setY( rect.center().y() - (pixh - 1) / 2 );
        drawItemPixmap(painter, rect, Qt::AlignCenter, pixmap);
        rect = RECT; rect.setLeft( rect.left() + pixw + 2 );
    }

    if (header->text.isEmpty())
        return;

    // textos ;)
    SAVE_PAINTER(Pen|(header->sortIndicator == QStyleOptionHeader::None)?0:Font);

    // this works around a possible Qt bug?!?
    QColor fg;
    if (sunken && !config.invert.headers) {
        fg = FCOLOR(HighlightedText);
    } else if (option->state & State_On) {
        fg = widget ? PAL.color(widget->backgroundRole()) : (config.invert.headers ? FCOLOR(Text) : FCOLOR(Base)); // abusing fg as bg cache
        fg = sunken || !FX::haveContrast(fg, FCOLOR(Highlight)) ? FCOLOR(HighlightedText) : FCOLOR(Highlight);
    } else {
        fg = widget ? PAL.color(widget->foregroundRole()) : (config.invert.headers ? FCOLOR(Base) : FCOLOR(Text));
    }

    fg.setAlpha(255);

    if (header->sortIndicator != QStyleOptionHeader::None) {
        QFont fnt(painter->font());
        fnt.setBold(true);
        painter->setFont(fnt);
    }

    painter->setPen(fg);
    Qt::Alignment align = Qt::AlignHCenter;
    if (header->sortIndicator == QStyleOptionHeader::SortUp)
        align |= Qt::AlignTop;
    else if (header->sortIndicator == QStyleOptionHeader::SortDown)
        align |= Qt::AlignBottom;
    else
        align |= Qt::AlignVCenter;
    drawItemText(painter, rect, align, PAL, isEnabled, header->text);
    RESTORE_PAINTER
}

void
Style::drawHeaderArrow(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
    Navi::Direction dir = Navi::S;
    if (const QStyleOptionHeader* hopt = qstyleoption_cast<const QStyleOptionHeader*>(option))
    {
        if (hopt->sortIndicator == QStyleOptionHeader::None)
            return;
        if (hopt->sortIndicator == QStyleOptionHeader::SortUp)
            dir = Navi::N;
    }
    SAVE_PAINTER(Pen|Brush);
    painter->setPen(Qt::NoPen);
    painter->setBrush(FX::blend(FCOLOR(Highlight), FCOLOR(HighlightedText)));
    drawArrow(dir, RECT, painter);
    RESTORE_PAINTER
}

// static const int gs_decoration_size = 9;

void
Style::drawBranch(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{

    if ( !RECT.isValid() )
        return;

    const int gs_decoration_size = ((RECT.height()>>2) + 2) | 3;

    SAVE_PAINTER(Pen|Brush);
    int mid_h = RECT.x() + RECT.width() / 2;
    int mid_v = RECT.y() + RECT.height() / 2;
    int bef_h = mid_h;
    int bef_v = mid_v;
    int aft_h = mid_h;
    int aft_v = mid_v;


    QPalette::ColorRole bg = QPalette::Base, fg = QPalette::Text;
    if (widget)
        { bg = widget->backgroundRole(); fg = widget->foregroundRole(); }

    bool firstCol = ( RECT.x() < 1 );
    if HAVE_OPTION(item, ViewItem)
        firstCol = item->viewItemPosition == QStyleOptionViewItem::Beginning ||
                   item->viewItemPosition == QStyleOptionViewItem::OnlyOne;

    if (option->state & State_Children) {
        int delta = gs_decoration_size / 2 + 2;
        bef_h -= delta;
        bef_v -= delta;
        aft_h += delta;
        aft_v += delta;
        painter->setPen(Qt::NoPen);
        QRect rect = QRect(bef_h+2, bef_v+2, gs_decoration_size, gs_decoration_size);
//         if (firstCol)
//             rect.moveRight(RECT.right()-F(1));
        Navi::Direction dir;
        QColor c;
        if (option->state & State_Open) {
            c = (option->state & State_Selected) ? FCOLOR(HighlightedText) : FX::blend( COLOR(bg), COLOR(fg));
            rect.translate(0,-gs_decoration_size/6);
            dir = (option->direction == Qt::RightToLeft) ? Navi::SW : Navi::SE;
        }
        else {
            c = (option->state & State_Selected) ? FCOLOR(HighlightedText) : FX::blend( COLOR(bg), COLOR(fg), 1, 4);
            dir = (option->direction == Qt::RightToLeft) ? Navi::W : Navi::E;
        }
        c.setAlpha(255);
        painter->setPen(c);
        painter->setBrush(Qt::NoBrush);
        drawArrow(dir, rect, painter);
    }

    // no line on the first column!
    if (firstCol) {
        RESTORE_PAINTER
        return;
    }

    painter->setPen(FX::blend( COLOR(bg), COLOR(fg), 10, 1));

    if (option->state & (State_Item | State_Sibling))
        painter->drawLine(mid_h, RECT.y(), mid_h, bef_v);
    if (option->state & State_Sibling)
        painter->drawLine(mid_h, aft_v, mid_h, RECT.bottom());
    if (option->state & State_Item) {
        if (option->direction == Qt::RightToLeft)
            painter->drawLine(RECT.left(), mid_v, bef_h, mid_v);
        else
            painter->drawLine(aft_h, mid_v, RECT.right(), mid_v);
    }
    RESTORE_PAINTER
}


void
Style::drawRubberBand(const QStyleOption *option, QPainter *painter, const QWidget*) const
{
    SAVE_PAINTER(Pen|Brush|Alias);

    static QPixmap scanlines;
    static QColor scanlineColor;

    QColor c = FCOLOR(Highlight);
    painter->setPen(QPen(c, 2));
    if (scanlines.isNull() || scanlineColor != c) {
        if (scanlines.isNull())
            scanlines = QPixmap(64,64);
        c.setAlpha(48);
        scanlines.fill(c);
        c.setAlpha(96);
        QPainter p(&scanlines);
        p.setPen(c);
        p.setBrush(Qt::NoBrush);
        for (int i = 3; i < 64; i+=4)
            p.drawLine(0,i,64,i);
        p.end();
    }
    painter->setBrush(scanlines);
    painter->setBrushOrigin(RECT.topLeft());
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::Antialiasing, true);
//     painter->drawRect(RECT.adjusted(0,0,-1,-1));
    painter->drawRoundedRect(RECT, config.frame.roundness, config.frame.roundness);
    RESTORE_PAINTER
}

enum IVI_Flags { Crumb = 1, DolphinDetail = 2 };

static const QWidget *last_widget = 0;
static int last_flags = 0;

static void updateLastWidget( const QWidget *widget, QPainter */*p*/ )
{
    if (widget != last_widget)
    {
        last_widget = widget;
        last_flags = 0;
        if (qobject_cast<const QPushButton*>(widget))
        {
            if (widget->inherits("KUrlButton") && !widget->inherits("KFilePlacesSelector"))
                last_flags |= Crumb;
            else if (widget->inherits("BreadcrumbItemButton"))
                last_flags |= Crumb;
        }
    }
}

void
Style::drawItem(const QStyleOption *option, QPainter *painter, const QWidget *widget, bool isItem) const
{
    // kwin tabbox, painting plasma and animation - this looks really lousy :-(
    if ( appType == KWin &&
         widget && widget->parentWidget() &&
         widget->parentWidget()->inherits("KWin::TabBox::TabBoxView") )
        return;

    ASSURE_OPTION(item, ViewItem);

    updateLastWidget( widget, painter );

    if (widget && (last_flags & Crumb))
        return;

    OPT_HOVER
    SAVE_PAINTER(Brush|Alias); // SIC! - "sigh" - we try to trick the widget into a painter

    const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(widget);
    hover = hover && (!view || view->selectionMode() != QAbstractItemView::NoSelection);
    bool selected = item->state & QStyle::State_Selected;
    const QWidget *viewport = 0;
    if (view)
        viewport = view->viewport();
    else if (!widget && painter->device()) {   // search the widget from the painter =P
        if (painter->device()->devType() == QInternal::Widget)
            widget = static_cast<QWidget*>(painter->device());
        else {
            QPaintDevice *dev = 
            /**** @todo this got canned, is the plain device actually correct?
                                QPainter::redirected(painter->device());
            ****/
                                painter->device();
            if (dev && dev->devType() == QInternal::Widget)
                widget = static_cast<QWidget*>(dev);
        }
        if (widget && widget->objectName() == "qt_scrollarea_viewport")
            viewport = widget;
    }

    QPalette::ColorRole bg = QPalette::Base, fg = QPalette::Text;
    if (viewport) {
        if (viewport->autoFillBackground())
            { bg = viewport->backgroundRole(); fg = viewport->foregroundRole(); }
        else
            { bg = QPalette::Window; fg = QPalette::WindowText; }
    }
    else if (widget)
        { bg = widget->backgroundRole(); fg = widget->foregroundRole(); }

//     if (bg == QPalette::Window) {
//         if (config.bg.modal.invert && widget && widget->window()->isModal())
//             { bg = QPalette::WindowText; fg = QPalette::Window; }
//     }

    if (!isItem && (hover || selected)) {   // dolphin constrains selection to the text and really REALLY hates a non winblows look :-(
        // TODO: write a better finder clone...
        if (view && !qstrcmp(view->metaObject()->className(), "DolphinDetailsView"))
            hover = selected = false;
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
    if (hover || selected) {
        bool isPopup(false);
        if (widget)
        if (const QWidget *window = widget->window()) {
            isPopup = (window->windowFlags() & Qt::Popup) == Qt::Popup;
        }

//         const QTreeView *tree = ;
        bool round = item->viewItemPosition == QStyleOptionViewItem::OnlyOne;
        if (!round && view && view->selectionMode() == QAbstractItemView::SingleSelection) {
            round = qobject_cast<const QListView*>(view);
        }

        QColor high = isPopup ? PAL.color(QPalette::Active, QPalette::Highlight) : FCOLOR(Highlight);
        if (!selected) {
            const int contrast = qMax(1, FX::contrastOf(high, COLOR(fg)));
            high = FX::blend(COLOR(bg), high, 100/contrast, 20);
//         } else if (round) {
//             high = FX::blend(COLOR(bg), high, 1, 3);
        }
        if (round) {
            const int rnd = qMin(config.frame.roundness, qMin(RECT.width(), RECT.height()/2));
            painter->setRenderHint(QPainter::Antialiasing, true);
//             painter->setPen(FCOLOR(Highlight));
            painter->setBrush(high);
//             painter->drawRect(RECT.adjusted(0,0,-1,-1));
            painter->setPen(Qt::NoPen);
            QRect r(RECT);
            if (qobject_cast<const QTreeView*>(view)) {
                r.setLeft(0);
                if (widget->contentsRect().width() - r.width() > rnd)
                    r.setRight(r.right() + rnd + 1);
                const QRegion clip(painter->clipRegion());
                const bool hadClip(painter->hasClipping());
                painter->setClipRect(RECT);
                painter->drawRoundedRect(r, rnd, rnd);
                painter->setClipRegion(clip);
                painter->setClipping(hadClip);
            } else
                painter->drawRoundedRect(r, rnd, rnd);
        } else {
            painter->setPen(Qt::NoPen);
            if ((selected || hover) && item->viewItemPosition == QStyleOptionViewItem::Invalid && widget &&
#if QT_VERSION >= 0x060000
                widget->inherits("QtPrivate::QCalendarView")) {
#else
                widget->inherits("QCalendarView")) {
#endif
                if (item->index.isValid() && item->index.row() && item->index.column()) {
                    if (!high.alpha() || !selected) { // this is the color we cheated to transparent in polish.cpp
                        SAVE_PAINTER(Alias);
                        const int s = qMin(RECT.width(), RECT.height());
                        QRect r(0,0,s,s);
                        r.moveCenter(RECT.center());
                        painter->setRenderHint(QPainter::Antialiasing);
                        if (selected)
                            high.setAlpha(255);
                        painter->setBrush(high);
                        painter->drawEllipse(r);
                        RESTORE_PAINTER
                    }
                } else {
                    painter->fillRect(RECT, item->backgroundBrush);
                }
            } else {
                painter->setBrush(high);
                painter->drawRect(RECT);
            }
        }
        // try to convince the itemview to use the proper fg color, WORKAROUND (kcategorizedview, mainly)
        if (selected)
            painter->setPen(FCOLOR(HighlightedText));
        else
            painter->setPen(COLOR(fg));
    } else {
        if (item->backgroundBrush.style() != Qt::NoBrush) {
            QPoint oldBO = painter->brushOrigin();
            painter->setBrushOrigin(RECT.topLeft());
            painter->fillRect(RECT, item->backgroundBrush);
            painter->setBrushOrigin(oldBO);
        } else if (item->features & QStyleOptionViewItem::Alternate) {
            painter->fillRect(RECT, PAL.brush(QPalette::AlternateBase));
        }
        // reset the painter for normal items. our above workaround otherwise might kill things...
        painter->setPen(COLOR(fg));
    }
    RESTORE_PAINTER
}

//NOTICE this is a hell lot of code in QCommonStyle, so for the moment, it's not customized
void Style::drawViewItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->fillRect(RECT, Qt::green);
}
