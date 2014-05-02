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
    const bool beginning = header && (header->position == QStyleOptionHeader::Beginning ||
                                      header->position == QStyleOptionHeader::OnlyOneSection);
    const bool ending = header && (header->position == QStyleOptionHeader::End ||
                                   header->position == QStyleOptionHeader::OnlyOneSection);


    if (appType == GTK)
        sunken = option->state & State_HasFocus;

    QColor c =  widget ? PAL.color(widget->backgroundRole()) : FCOLOR(Text);
    if (sorting || sunken) {
        if ((option->state & State_On) || !FX::haveContrast(c, FCOLOR(Highlight)))
            c = FCOLOR(Highlight);
    }
    if (!sunken && hover) {
        c = FX::blend(c, sorting ? FCOLOR(HighlightedText) : FCOLOR(Highlight),8,1);
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(Qt::NoPen);
    painter->setBrush(c);

    QPainterPath path;
    const int rnd(config.frame.roundness);
    const int rnd_2 = 2*rnd;
    const QRect r(RECT.adjusted(0,0,1,1));
    path.moveTo(r.bottomRight());
    if (ending && header->orientation == Qt::Vertical) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        path.lineTo(r.x() + rnd, r.bottom());
        path.arcTo(r.x(), r.bottom() - rnd_2, rnd_2, rnd_2, -90, -90);
    } else {
        path.lineTo(r.bottomLeft());
    }
    if (beginning) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        path.lineTo(r.x(), r.y() + rnd);
        path.arcTo(r.x(), r.y(), rnd_2, rnd_2, 180, -90);
    } else {
        path.lineTo(r.topLeft());
    }
    if (ending && header->orientation == Qt::Horizontal) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        path.lineTo(r.right() - rnd, r.y());
        path.arcTo(r.right() - rnd_2, r.y(), rnd_2, rnd_2, 90, -90);
    } else {
        path.lineTo(r.topRight());
    }
    path.closeSubpath();
    painter->drawPath(path);

    if (!header || header->position < QStyleOptionHeader::End) {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(FX::blend(c, FCOLOR(Base), 6, 1));
        if (header && header->orientation == Qt::Vertical)
            painter->drawLine(RECT.bottomLeft(), RECT.bottomRight());
        else
            painter->drawLine(RECT.topRight(), RECT.bottomRight());
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
    SAVE_PAINTER(Pen|Font);

    // this works around a possible Qt bug?!?
    QColor fg;
    const bool isOn(option->state & State_On);
    if ((header->sortIndicator != QStyleOptionHeader::None) || sunken) {
        fg = widget ? PAL.color(widget->backgroundRole()) : FCOLOR(Text);
        fg = isOn || !FX::haveContrast(fg, FCOLOR(Highlight)) ? FCOLOR(HighlightedText) : FCOLOR(Highlight);
    } else {
        fg = isOn ? FCOLOR(Highlight) : (widget ? PAL.color(widget->foregroundRole()) : FCOLOR(Base));
    }

    fg.setAlpha(255);
    painter->setPen(fg);
    drawItemText ( painter, rect, Qt::AlignCenter, PAL, isEnabled, header->text);
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
    if HAVE_OPTION(item, ViewItemV4)
        firstCol = item->viewItemPosition == QStyleOptionViewItemV4::Beginning ||
                   item->viewItemPosition == QStyleOptionViewItemV4::OnlyOne;

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

    ASSURE_OPTION(item, ViewItemV4);

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
            QPaintDevice *dev = QPainter::redirected(painter->device());
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
        bool round = item->viewItemPosition == QStyleOptionViewItemV4::OnlyOne;
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
            painter->setBrush(high);
//             if (item->viewItemPosition == QStyleOptionViewItemV4::Invalid && widget->inherits("QCalendarView")) {
//                 const int r = (qMin(RECT.width(), RECT.height()) - 1 ) / 2;
//                 painter->setBrush(Qt::red);
//                 painter->drawRoundedRect(RECT, r,r);
//             } else
                painter->drawRect(RECT);
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
        } else if (item->features & QStyleOptionViewItemV2::Alternate) {
            painter->fillRect(RECT, PAL.brush(QPalette::AlternateBase));
        }
        // reset the painter for normal items. our above workaround otherwise might kill things...
        painter->setPen(COLOR(fg));
    }
    RESTORE_PAINTER
}
