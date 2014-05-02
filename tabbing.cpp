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
//     QRect r(RECT);
//     switch (tbb->shape) {
//     case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
//         r.adjust(F(4), 0, -F(4), -F(4));
//         break;
//     case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
//         r.adjust(F(4), F(4), -F(4), 0);
//         break;
//     case QTabBar::RoundedEast: case QTabBar::TriangularEast:
//         r.adjust(F(4), F(4), 0, -F(4));
//         break;
//     case QTabBar::RoundedWest: case QTabBar::TriangularWest:
//         r.adjust(0, F(4), -F(4), -F(4));
//         break;
//     }
//     
//     

    if (RECT.x() == winRect.x() || RECT.y() == winRect.y() || RECT.right() == winRect.right() || RECT.bottom() == winRect.bottom()) {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(RECT);
    } else {
        painter->setRenderHint(QPainter::Antialiasing, true);
        const int rnd = qMin(config.frame.roundness, verticalTabs(tbb->shape) ? RECT.width()/2 : RECT.height()/2);
        painter->drawRoundedRect(RECT, rnd, rnd);
    }
//     const QRect r(RECT.adjusted(0,0,1,1));
//     bool documentMode = false;
//     if HAVE_OPTION(tbbV2, TabBarBaseV2) {
//         documentMode = tbbV2->documentMode;
//     }
//     QPainterPath path;
//     if (documentMode) {
//         path.moveTo(r.topLeft());
//         path.lineTo(r.topRight());
//         path.lineTo(r.right(), r.bottom() - F(4));
//         path.arcTo(r.right() - F(8), r.bottom() - F(8), F(8), F(8), 0, -90);
//         path.lineTo(r.x() + F(4), r.bottom());
//         path.arcTo(r.x(), r.bottom() - F(8), F(8), F(8), -90, -90);
//         path.closeSubpath();
//     } else {
//         path.moveTo(r.bottomRight());
//         path.lineTo(r.bottomLeft());
//         path.lineTo(r.x(), r.y() + F(4));
//         path.arcTo(r.x(), r.y(), F(8), F(8), 180, -90);
//         path.lineTo(r.right() - F(4), r.y());
//         path.arcTo(r.right() - F(8), r.y(), F(8), F(8), 90, -90);
//         path.closeSubpath();
//     }
//     painter->setRenderHint(QPainter::Antialiasing, true);
//     painter->drawPath(path);
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

    if (appType == GTK) {
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
Style::drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    SAVE_PAINTER(Pen|Brush);
    painter->setPen(Qt::NoPen);
    painter->setBrush(FCOLOR(WindowText));
    painter->drawRect(RECT);
    RESTORE_PAINTER
}

void
Style::drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(tab, Tab);
    OPT_SUNKEN OPT_ENABLED

    calcAnimStep( option, painter, widget );

    if (tab->position == QStyleOptionTab::OnlyOneTab)
        { sunken = false; /*hover = false;*/ }
    else
        sunken = sunken || (option->state & State_Selected);
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

        if (animStep > 2 || sunken)
            painter->setClipRect(tr.adjusted(F(4), F(4), -F(5), -F(5)));
        painter->drawPixmap(tr.left() + F(9), tr.center().y() - tabIcon.height() / 2, tabIcon);
        tr.setLeft(tr.left() + iconSize.width() + F(12));
        alignment = Qt::AlignLeft | Qt::AlignVCenter | BESPIN_MNEMONIC;
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
    QColor cF = FCOLOR(Window), cB = FCOLOR(WindowText);
    if (option->state & State_Selected) {
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
Style::drawToolboxTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (option->state & State_Selected)
        return; // plain selected items
    OPT_HOVER OPT_SUNKEN
    SAVE_PAINTER(Pen|Brush);
    painter->setPen(Qt::NoPen);
    if (hover || sunken)
        painter->setBrush(FX::blend(FCOLOR(WindowText), FCOLOR(Highlight), 2, hover + 4*sunken));
    else
        painter->setBrush(FCOLOR(WindowText));
    painter->drawRect(RECT);
    RESTORE_PAINTER
}

void
Style::drawToolboxTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    ASSURE_OPTION(tbt, ToolBox);
    OPT_ENABLED OPT_SUNKEN
    uint tf = BESPIN_MNEMONIC;
    QPalette::ColorRole role = QPalette::WindowText;
    if (option->state & State_Selected) {
        tf |= Qt::AlignBottom | Qt::AlignLeft;
        QFont fnt(painter->font());
        QFont oFnt(fnt);
        if (fnt.pointSize() > 0) {
            fnt.setPointSize(16*fnt.pointSize()/10);
            fnt.setBold(true);
        }
        painter->setFont(fnt);
        drawItemText(painter, RECT, tf, PAL, isEnabled, tbt->text, role);
        painter->setFont(oFnt);
    } else {
        tf |= Qt::AlignHCenter | Qt::AlignRight;
        if (sunken)
            role = QPalette::HighlightedText;
        else
            role = QPalette::Window;
        drawItemText(painter, RECT, tf, PAL, isEnabled, tbt->text, role);
    }
}
