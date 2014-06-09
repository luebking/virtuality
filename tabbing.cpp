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

#include <QToolButton>
#include "draw.h"
#include "animator/hoverindex.h"

#include <QtDebug>

inline static bool
verticalTabs(QTabBar::Shape shape)
{
    return  shape == QTabBar::RoundedEast ||
            shape == QTabBar::TriangularEast ||
            shape == QTabBar::RoundedWest ||
            shape == QTabBar::TriangularWest;
}

void
Style::drawTabWidget(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(twf, TabWidgetFrame);
    SAVE_PAINTER(Pen);
    QStyleOptionTabBarBaseV2 tbb;
    if (widget)
        tbb.initFrom(widget);
    else
        tbb.QStyleOption::operator=(*twf);
    tbb.shape = twf->shape; tbb.rect = twf->rect;
    if HAVE_OPTION(twf2, TabWidgetFrameV2) {
        tbb.selectedTabRect = twf2->selectedTabRect;
    }

#define SET_BASE_HEIGHT(_o_) \
baseHeight = twf->tabBarSize._o_(); \
if (!baseHeight) return; /*  no base -> no tabbing -> no bottom border either. period.*/\
if (baseHeight < 0) \
    baseHeight = pixelMetric( PM_TabBarBaseHeight, option, widget )

    int baseHeight;
    painter->setPen(FRAME_PEN);
    switch (twf->shape) {
    case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
        SET_BASE_HEIGHT(height);
        tbb.rect.setHeight(baseHeight);
        painter->drawLine(RECT.bottomLeft(), RECT.bottomRight());
        break;
    case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
        SET_BASE_HEIGHT(height);
        tbb.rect.setTop(tbb.rect.bottom()-baseHeight);
        painter->drawLine(RECT.topLeft(), RECT.topRight());
        break;
    case QTabBar::RoundedEast: case QTabBar::TriangularEast:
        SET_BASE_HEIGHT(width);
        tbb.rect.setLeft(tbb.rect.right()-baseHeight);
        painter->drawLine(RECT.topLeft(), RECT.bottomLeft());
        break;
    case QTabBar::RoundedWest: case QTabBar::TriangularWest:
        SET_BASE_HEIGHT(width);
        tbb.rect.setWidth(baseHeight);
        painter->drawLine(RECT.topRight(), RECT.bottomRight());
        break;
    }
#undef SET_BASE_HEIGHT

    // the "frame"
    // the bar
    drawTabBar(&tbb, painter, widget);
    RESTORE_PAINTER
}

void
Style::drawTabBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(tbb, TabBarBase);

    if (!config.invert.headers) {
        if (tbb->selectedTabRect.isEmpty())
            return; // only paint tab shapes
        SAVE_PAINTER(Pen|Alias);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(QPen(FRAME_COLOR, FRAME_STROKE_WIDTH));
        switch (tbb->shape) {
        case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
            painter->drawLine(RECT.x(), RECT.bottom(), tbb->selectedTabRect.x() + tbb->selectedTabRect.width()/3, RECT.bottom());
            painter->drawLine(tbb->selectedTabRect.right(), RECT.bottom(), RECT.right(), RECT.bottom());
            break;
        case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
            painter->drawLine(RECT.x(), RECT.top(), tbb->selectedTabRect.x(), RECT.top());
            painter->drawLine(tbb->selectedTabRect.right() - tbb->selectedTabRect.width()/3, RECT.top(), RECT.right(), RECT.top());
            break;
        case QTabBar::RoundedEast: case QTabBar::TriangularEast:
            painter->drawLine(RECT.x(), RECT.y(), RECT.x(), tbb->selectedTabRect.y());
            painter->drawLine(RECT.x(), tbb->selectedTabRect.bottom() - tbb->selectedTabRect.height()/3, RECT.x(), RECT.bottom());
            break;
        case QTabBar::RoundedWest: case QTabBar::TriangularWest:
            painter->drawLine(RECT.right(), RECT.y(), RECT.right(), tbb->selectedTabRect.y() + tbb->selectedTabRect.height()/3);
            painter->drawLine(RECT.right(), tbb->selectedTabRect.bottom(), RECT.right(), RECT.bottom());
            break;
        }
        RESTORE_PAINTER
        return;
    }

    QWidget *win = 0;

    if (widget) {
        if (widget->parentWidget() && qobject_cast<QTabWidget*>(widget->parentWidget())) {
            if (widget->parentWidget()->style() == this) {
                return; // otherwise it's a proxystyle like on konqueror / kdevelop...
            }
        } else if (qobject_cast<const QTabBar*>(widget) || (appType == KDevelop && widget->inherits("QLabel"))) {
            return; // usually we alter the paintevent by eventfiltering
        }
        win = widget->window();
    } else {
        if (painter->device()->devType() == QInternal::Widget)
            widget = static_cast<QWidget*>(painter->device());
        else {
            QPaintDevice *dev = QPainter::redirected(painter->device());
            if (dev && dev->devType() == QInternal::Widget)
                widget = static_cast<QWidget*>(dev);
        }
        if ( widget )
            win = widget->window();
    }

    QRect winRect;
    if (win) {
        winRect = win->rect();
        winRect.moveTopLeft(widget->mapFrom(win, winRect.topLeft()));
    }
//     else
//         winRect = tbb->tabBarRect; // we set this from the eventfilter QEvent::Paint

    SAVE_PAINTER(Pen|Brush|Alias);
    painter->setBrush(PAL.color(QPalette::Active, QPalette::WindowText));
    painter->setPen(Qt::NoPen);

    // TODO: half rounded rect?
    if (RECT.x() == winRect.x() || RECT.y() == winRect.y() || RECT.right() == winRect.right() || RECT.bottom() == winRect.bottom()) {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(RECT);
    } else {
        painter->setRenderHint(QPainter::Antialiasing, true);
        const int rnd = qMin(config.frame.roundness, verticalTabs(tbb->shape) ? RECT.width()/2 : RECT.height()/2);
        painter->drawRoundedRect(RECT, rnd, rnd);
    }
    RESTORE_PAINTER
}

static int animStep = -1;
static bool customColor = false;

void
Style::calcAnimStep(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{   // animation stuff
    OPT_ENABLED OPT_HOVER OPT_SUNKEN
    sunken = sunken || (option->state & State_Selected);
    animStep = 0;
    if ( isEnabled && !sunken )
    {
        Animator::IndexInfo *info = 0;
        int index = -1, hoveredIndex = -1;
        if (widget)
        if (const QTabBar* tbar = qobject_cast<const QTabBar*>(widget))
        {
            // NOTICE: the index increment is IMPORTANT to make sure it's not "0"
            index = tbar->tabAt(RECT.topLeft()) + 1; // is the action for this item!

            // sometimes... MANY times devs just set the tabTextColor to QPalette::WindowText,
            // because it's defined that it has to be this. Qt provides all these color roles just
            // to waste space and time... ...
            const QColor &fgColor = tbar->tabTextColor(index - 1);
            const QColor &stdFgColor = tbar->palette().color(QPalette::Window);
            if (fgColor.isValid() && fgColor != stdFgColor)
            {
                if (fgColor == tbar->palette().color(QPalette::WindowText))
                    const_cast<QTabBar*>(tbar)->setTabTextColor(index - 1, stdFgColor); // fixed
                else // nope, this is really a custom color that will likley contrast just enough with QPalette::Window...
                {
                    customColor = true;
                    if (FX::haveContrast(tbar->palette().color(QPalette::WindowText), fgColor))
                        painter->setPen(fgColor);
                }
            }
            if (!tbar->documentMode() || tbar->drawBase()) {
                if (hover)
                    hoveredIndex = index;
                else if (widget->underMouse())
                    hoveredIndex = tbar->tabAt(tbar->mapFromGlobal(QCursor::pos())) + 1;
                info = const_cast<Animator::IndexInfo*>(Animator::HoverIndex::info(widget, hoveredIndex));
            }

        }
        if (info)
            animStep = info->step(index);
        if (hover && !animStep)
            animStep = 6;
    }
}

void
Style::drawTab(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(tab, Tab);

    // do we have to exclude the scrollers?
    bool needRestore = false;
    if (widget && qobject_cast<const QTabBar*>(widget))
    {
        QRegion clipRgn = painter->clipRegion();
        if (clipRgn.isEmpty())
            clipRgn = RECT;

        QList<QToolButton*> buttons = widget->findChildren<QToolButton*>();
        foreach (QToolButton* button, buttons)
        {
            if (button->isVisible())
                clipRgn -= QRect(button->pos(), button->size());
        }
        if (!clipRgn.isEmpty())
        {
            painter->save();
            needRestore = true;
            painter->setClipRegion(clipRgn);
        }
    }

    // paint shape and label
    QStyleOptionTab copy = *tab;
    // NOTICE: workaround for e.g. konsole,
    // which sets the tabs bg, but not the fg color to the palette, but just
    // presets the painter and hopes for the best... tststs
    // TODO: bug Konsole/Konqueror authors
    if (widget)
        copy.palette = widget->palette();

    if (!config.invert.headers) {
        drawTabShape(&copy, painter, widget);
    }
    if HAVE_OPTION(tabV3, TabV3) {
        int d[4] = {0,1,0,0};
        if (verticalTabs(tab->shape)) {
            if ( tabV3->leftButtonSize.isValid() ) d[1] = tabV3->leftButtonSize.height() + F(2);
            if ( tabV3->rightButtonSize.isValid() ) d[3] = -tabV3->rightButtonSize.height() - F(2);
        } else {
            if ( tabV3->leftButtonSize.isValid() ) d[0] = tabV3->leftButtonSize.width() + F(2);
            if ( tabV3->rightButtonSize.isValid() ) d[2] = -tabV3->rightButtonSize.width() - F(2);
        }
        copy.rect.adjust( d[0], d[1], d[2], d[3] );
    }
    drawTabLabel(&copy, painter, widget);
    customColor = false;
    if (needRestore)
        painter->restore();
}

void
Style::drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    if (config.invert.headers) {
        SAVE_PAINTER(Pen|Brush);
        painter->setPen(Qt::NoPen);
        painter->setBrush(FCOLOR(WindowText));
        painter->drawRect(RECT);
        RESTORE_PAINTER
    } else {
        ASSURE_OPTION(tab, Tab);
        SAVE_PAINTER(Pen);
        OPT_SELECTED
        if (selected) {
            painter->setPen(FRAME_PEN);
            SAVE_PAINTER(Alias);
            painter->setRenderHint(QPainter::Antialiasing);
            bool docMode = false;
            if HAVE_OPTION(tab3, TabV3) {
                docMode = tab3->documentMode;
            }
            const int rnd = qMin(config.frame.roundness*2, RECT.height());
            QRectF r(0,0,rnd,rnd);
            QPainterPath path;
            switch (tab->shape) {
            case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
                r.moveTopRight(RECT.topRight() + QPointF(-halfStroke, halfStroke));
                path.moveTo(RECT.right() - (docMode ? RECT.height()/2 : RECT.width()/3), r.y());
                path.lineTo(r.x() + r.width()/2, r.y());
                path.arcTo(r, 90, -90);
                path.lineTo(r.right(), RECT.y() + RECT.height()/2);
                break;
            case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
                r.moveBottomLeft(RECT.bottomLeft() + QPointF(halfStroke, -halfStroke));
                path.moveTo(RECT.left() + (docMode ? RECT.height()/2 : RECT.width()/3), r.bottom() + 1);
                path.lineTo(r.x() + r.width()/2, r.bottom() + 1);
                path.arcTo(r, -90, -90);
                path.lineTo(r.x(), RECT.y() + RECT.height()/2);
                break;
            case QTabBar::RoundedEast: case QTabBar::TriangularEast:
                r.moveTopRight(RECT.topRight() + QPointF(-halfStroke, halfStroke));
                path.moveTo(RECT.right() - RECT.width()/2, r.y());
                path.lineTo(r.x() + r.width()/2, r.y());
                path.arcTo(r, 90, -90);
                path.lineTo(r.right(), RECT.y() + (docMode ? RECT.width()/2 : RECT.height()/3));
                break;
            case QTabBar::RoundedWest: case QTabBar::TriangularWest:
                r.moveBottomLeft(RECT.bottomLeft() + QPointF(halfStroke, -halfStroke));
                path.moveTo(RECT.left() + RECT.width()/2, r.bottom() + 1);
                path.lineTo(r.x() + r.width()/2, r.bottom() + 1);
                path.arcTo(r, -90, -90);
                path.lineTo(r.x(), RECT.y() + (docMode ? RECT.width()/2 : RECT.height()/3));
                break;
            }
            painter->drawPath(path);
            RESTORE_PAINTER
        } else {
            painter->setPen(QPen(FRAME_COLOR, FRAME_STROKE_WIDTH));
            SAVE_PAINTER(Alias);
            painter->setRenderHint(QPainter::Antialiasing, false);
            switch (tab->shape) {
            case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
                painter->drawLine(RECT.bottomLeft(), RECT.bottomRight());
                break;
            case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
                painter->drawLine(RECT.topLeft(), RECT.topRight());
                break;
            case QTabBar::RoundedEast: case QTabBar::TriangularEast:
                painter->drawLine(RECT.topLeft(), RECT.bottomLeft());
                break;
            case QTabBar::RoundedWest: case QTabBar::TriangularWest:
                painter->drawLine(RECT.topRight(), RECT.bottomRight());
                break;
            }
            RESTORE_PAINTER
        }
        RESTORE_PAINTER
    }
}

void
Style::drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(tab, Tab);
    OPT_SUNKEN OPT_ENABLED OPT_SELECTED OPT_FOCUS

    calcAnimStep( option, painter, widget );

    if (tab->position == QStyleOptionTab::OnlyOneTab)
        { sunken = false; /*hover = false;*/ }
    else
        sunken = sunken || selected;
//     if (sunken) hover = false;

    painter->save();
    QRect tr = RECT;

    bool vertical = false;
    bool east = false;
    int alignment = BESPIN_MNEMONIC | Qt::AlignHCenter | Qt::AlignBottom;
    switch (tab->shape) {
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
            alignment &= ~Qt::AlignBottom;
            alignment |= Qt::AlignTop;
            break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
            east = true; // fall through
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
            vertical = true;
            break;
        default:
            break;
    }


    if (vertical) {
        int newX, newY, newRot;
        if (east)
            { newX = tr.width(); newY = tr.y(); newRot = 90; }
        else
            { newX = 0; newY = tr.y() + tr.height(); newRot = -90; }
        tr.setRect(0, 0, tr.height(), tr.width());
        QMatrix m; m.translate(newX, newY); m.rotate(newRot);
        painter->setMatrix(m, true);
    }

    if ( !tab->icon.isNull() ) {
        QSize iconSize;
        if (const QStyleOptionTabV2 *tabV2 = qstyleoption_cast<const QStyleOptionTabV2*>(tab)) {
            iconSize = tabV2->iconSize;
        }
        if ( !iconSize.isValid() )
        {
            if (const QTabBar* tabbar = qobject_cast<const QTabBar*>(widget)) {
                iconSize = tabbar->iconSize();
            } else {
                const int iconExtent = pixelMetric(PM_TabBarIconSize);
                iconSize = QSize(iconExtent, iconExtent);
            }
        }
        QPixmap tabIcon = tab->icon.pixmap(iconSize, (isEnabled) ? QIcon::Normal : QIcon::Disabled);

        painter->drawPixmap(tr.left() + F(9), tr.center().y() - tabIcon.height() / 2, tabIcon);
        tr.setLeft(tr.left() + iconSize.width() + F(12));
        alignment = (alignment & ~Qt::AlignHCenter) |  Qt::AlignLeft;
    }

    if HAVE_OPTION(tabV3, TabV3) {
        if (vertical) {
            tr.setLeft(tr.left() + tabV3->leftButtonSize.height() + F(4));
            tr.setRight(tr.right() - (tabV3->rightButtonSize.height() + F(4)));
        } else {
            tr.setLeft(tr.left() + tabV3->leftButtonSize.width() + F(4));
            tr.setRight(tr.right() - (tabV3->rightButtonSize.width()+F(4)));
        }
    }

    // color adjustment
    bool elide(false);
    QColor cF, cB;
    if (config.invert.headers) {
        cF = hasFocus && selected &&
             FX::haveContrast(FCOLOR(Highlight), FCOLOR(WindowText)) ? FCOLOR(Highlight) : FCOLOR(Window);
        cB = FCOLOR(WindowText);
    } else {
        cF = hasFocus && selected &&
             FX::haveContrast(FCOLOR(Highlight), FCOLOR(Window)) ? FCOLOR(Highlight) : FCOLOR(WindowText);
        cB = FCOLOR(Window);
    }
    if (selected) {
        if (vertical) {
            setBold(painter, tab->text, tr.width());
        } else {
            QFont fnt(painter->font());
//             fnt.setBold(true);
            if (fnt.pointSize() > 0) {
                fnt.setPointSize(16*fnt.pointSize()/10);
                float w = QFontMetrics(fnt).width(tab->text);
                w = qMin(1.0f, tr.width()/w);
                if (w < 0.8) {
                    elide = true;
                    w = 0.8;
                }
                fnt.setPointSize(w*fnt.pointSize());
            }
            painter->setFont(fnt);
        }
    } else if (animStep) {
        cF = FX::blend(cB, cF, 1, animStep);
        if (!sunken) {
            QFont fnt(painter->font());
            if (fnt.pointSize() > 0)
                fnt.setPointSize(13*fnt.pointSize()/10);
            painter->setFont(fnt);
        }
    } else if (customColor) {
        if (FX::haveContrast(FCOLOR(WindowText), painter->pen().color()))
            cF = painter->pen().color();
        QFont fnt = painter->font();
        fnt.setUnderline(true);
        painter->setFont(fnt);
    } else {
        cF = FX::blend(cB, cF, 1, 1);
    }

    painter->setPen(cF);
    if (elide) {
        drawItemText(painter, tr, alignment, PAL, isEnabled, painter->fontMetrics().elidedText(tab->text, Qt::ElideRight, tr.width()));
    } else
        drawItemText(painter, tr, alignment, PAL, isEnabled, tab->text);

    painter->restore();
    animStep = -1;
}

void
Style::drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget*) const
{
    OPT_SUNKEN OPT_HOVER
    SAVE_PAINTER(Pen|Brush|Alias);

    if (sunken) hover = false;

    QRect rect = RECT;
    sunken ? rect.adjust(F(5),F(4),-F(4),-F(5)) :
    hover ?  rect.adjust(F(3),F(2),-F(2),-F(3)) :
             rect.adjust(F(4),F(3),-F(3),-F(4));

    painter->setRenderHint(QPainter::Antialiasing);

    QColor c = FX::blend( FCOLOR(WindowText), FCOLOR(Window), 3, 1+hover );
    painter->setPen( QPen( c, F(3) ) );
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse( rect );

//     c = hover ? CCOLOR(tab.active, Fg) : FX::blend( CCOLOR(tab.active, Bg), CCOLOR(tab.active, Fg) );
//     painter->setPen( QPen( c, F(2) ) );
//     rect.adjust(F(2),F(2),-F(2),-F(2));
//     painter->drawEllipse( rect );
    RESTORE_PAINTER
}

void
Style::drawToolboxTab(const QStyleOption *option, QPainter *painter, const QWidget * widget) const
{
    ASSURE_OPTION(tbt, ToolBox);

    // color fix...
    if (widget && widget->parentWidget())
        const_cast<QStyleOption*>(option)->palette = widget->parentWidget()->palette();

    drawToolboxTabShape(tbt, painter, widget);
    QStyleOptionToolBox copy = *tbt;
    copy.rect.adjust(F(2), F(2), -F(2), -F(2));
    drawToolboxTabLabel(&copy, painter, widget);
}

void
Style::drawToolboxTabShape(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    ASSURE_OPTION(tbt, ToolBox);
    if (option->state & State_Selected) {
        if (config.invert.headers) {
            SAVE_PAINTER(Pen|Brush|Alias);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(Qt::NoPen);
            painter->setBrush(FCOLOR(WindowText));
            const int rnd = qMin(config.frame.roundness, RECT.height()/2);
            painter->drawRoundedRect(RECT, rnd, rnd);
            RESTORE_PAINTER
        }
    } else {
        OPT_SUNKEN
        SAVE_PAINTER(Pen);
        painter->setPen(sunken ? FOCUS_FRAME_PEN : FRAME_PEN);
        QRect r = painter->fontMetrics().boundingRect(RECT, BESPIN_MNEMONIC|Qt::AlignCenter, tbt->text);
        const int y = r.y() + r.height()/2;
        const int d = (RECT.width() - r.width())/6 + F(6);
        painter->drawLine(r.x() - d, y, r.x() - (sunken ? F(12) : F(6)), y);
        painter->drawLine(r.right() + (sunken ? F(12) : F(6)), y, r.right() + d, y);
        RESTORE_PAINTER
    }
}

void
Style::drawToolboxTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    ASSURE_OPTION(tbt, ToolBox);
    OPT_ENABLED OPT_SUNKEN OPT_HOVER OPT_SELECTED
    SAVE_PAINTER(sunken||selected ? Font : 0);
    uint tf = BESPIN_MNEMONIC;
    QPalette::ColorRole role = QPalette::WindowText;
    if (selected) {
        tf |= Qt::AlignBottom;
        if (config.invert.headers) {
            role = QPalette::Window;
            tf |= Qt::AlignHCenter;
        } else {
            tf |= Qt::AlignLeft;
        }
        QFont fnt(painter->font());
        if (fnt.pointSize() > 0) {
            fnt.setPointSize(15*fnt.pointSize()/10);
            fnt.setBold(true);
        }
        painter->setFont(fnt);
    } else {
        if (hover || sunken)
            role = QPalette::Highlight;
        if (sunken) {
            QFont fnt(painter->font());
            fnt.setBold(true);
            painter->setFont(fnt);
        }
        tf |= Qt::AlignCenter;
    }
    drawItemText(painter, RECT, tf, PAL, isEnabled, tbt->text, role);
    RESTORE_PAINTER
}
