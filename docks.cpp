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
#include <QDockWidget>
#include <QPainterPath>
#include <QStyleOptionDockWidget>
#include "draw.h"
#include "hacks.h"

#include <QtDebug>

static QDockWidget *carriedDock = 0;

void
Style::dockLocationChanged( Qt::DockWidgetArea /*area*/ )
{
    QDockWidget *dock = carriedDock ? carriedDock : qobject_cast<QDockWidget*>( sender() );
    if ( !dock )
        return;
    if ( dock->isFloating() || !Hacks::config.lockDocks )
    {
        if ( QWidget *title = dock->titleBarWidget() )
        {
            if ( title->objectName() ==  "bespin_docktitle_dummy" )
            {
                dock->setTitleBarWidget(0);
                title->deleteLater();
            }
            else
                title->show();
        }
    }
    else
    {
        QWidget *title = dock->titleBarWidget();
        if ( !title )
        {
            title = new QWidget;
            title->setObjectName( "bespin_docktitle_dummy" );
            dock->setTitleBarWidget( title );
        }
        if ( title->objectName() ==  "bespin_docktitle_dummy" )
            dock->titleBarWidget()->hide();
    }
}

static struct {
    QPainterPath path;
    QSize size;
    bool ltr;
} glas = {QPainterPath(), QSize(), true};

void
Style::drawDockBg(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->isWindow())
        drawWindowFrame(option, painter, widget);
}

void
Style::drawDockTitle(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(dock, DockWidget);
    SAVE_PAINTER(Pen|Brush|Alias|Font);

    const bool floating = widget && widget->isWindow();
    QRect rect = RECT;

    if (!dock->title.isEmpty()) {
        OPT_ENABLED
        const int bo = 16 + F(6);
        /**** @todo - verticaltitlebar got canned, using the state NOT correct ***/
        if (true /*dock->state & QStyle::State_Horizontal*/) {
            rect.adjust(dock->closable ? bo : F(4), 0, dock->closable ? -bo : -F(4), 0);
        } else {
            int y = rect.y() + rect.height();
            rect.setRect(0, 0, rect.height(), rect.width());
            QTransform m; m.translate(0, y);
            m.rotate(-90);
            painter->setTransform(m, true);
            rect.adjust(0, dock->closable ? bo : F(4), 0, dock->closable ? -bo : -F(4));
        }

        // text
        const int itemtextopts = Qt::AlignCenter | Qt::TextSingleLine | Qt::TextHideMnemonic;
        QPalette::ColorRole bg = widget ? widget->backgroundRole() : QPalette::Window;
        QPalette::ColorRole fg = widget ? widget->foregroundRole() : QPalette::WindowText;
        setBold(painter, dock->title, rect.width());
        if (floating && widget->isActiveWindow())
            painter->setPen(COLOR(fg));
        else
            painter->setPen(FX::blend(COLOR(bg), COLOR(fg), 2, 1+isEnabled));
        drawItemText(painter, rect, itemtextopts, PAL, isEnabled, dock->title, QPalette::NoRole, &rect);
        if (option->direction == Qt::LeftToRight)
            rect.setRect(rect.x() - F(8), rect.y(), F(8), rect.height());
        else
            rect.setRect(rect.right(), rect.y(), F(8), rect.height());
    }
    if ((dock->floatable || dock->movable) && !floating) {
        QRect r(RECT);
        const_cast<QStyleOption*>(option)->rect = rect;
        drawDockHandle(option, painter, widget);
        const_cast<QStyleOption*>(option)->rect = r;
    }
    RESTORE_PAINTER
}

void
Style::drawHandle(const QStyleOption *option, QPainter *painter, const QWidget *w, bool invert) const
{
    SAVE_PAINTER(Pen|Brush|Alias);

    painter->setPen(Qt::NoPen);
    QPalette::ColorRole bg(QPalette::Window), fg(QPalette::WindowText);
    if (invert && !qobject_cast<const QDockWidget*>(w)) {
        bg = QPalette::WindowText; fg = QPalette::Window;
        painter->setBrush(PAL.color(QPalette::Active, QPalette::WindowText));
        painter->drawRect(RECT);
    }

    OPT_HOVER

    QRect r(0,0,F(4),F(4));
    r.moveCenter(RECT.center());
    painter->setBrush(FX::blend(COLOR(bg), COLOR(fg),8,1+2*hover));
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawEllipse(r);
    RESTORE_PAINTER
}

void
Style::drawMDIControls(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    QStyleOptionButton btnOpt;
    btnOpt.QStyleOption::operator=(*option);
    OPT_SUNKEN

#define PAINT_MDI_BUTTON(_btn_)\
if (option->subControls & SC_Mdi##_btn_##Button)\
{\
    if (sunken && option->activeSubControls & SC_Mdi##_btn_##Button)\
    {\
        btnOpt.state |= State_Sunken;\
        btnOpt.state &= ~State_Raised;\
    }\
    else\
    {\
        btnOpt.state |= State_Raised;\
        btnOpt.state &= ~State_Sunken;\
    }\
    btnOpt.rect = subControlRect(CC_MdiControls, option, SC_Mdi##_btn_##Button, widget);\
    painter->drawPixmap(btnOpt.rect.topLeft(), standardPixmap(SP_TitleBar##_btn_##Button, &btnOpt, widget));\
}//

    PAINT_MDI_BUTTON(Close);
    PAINT_MDI_BUTTON(Normal);
    PAINT_MDI_BUTTON(Min);

#undef PAINT_MDI_BUTTON
}

void
Style::unlockDocks(bool b)
{
    const bool lock = Hacks::config.lockDocks;
    Hacks::config.lockDocks = b;
    foreach ( QWidget *w, qApp->allWidgets() )
    {
        if ( (carriedDock = qobject_cast<QDockWidget*>(w)) )
        if ( !carriedDock->isFloating() )
            dockLocationChanged( Qt::AllDockWidgetAreas );
    }
    carriedDock = 0;
    Hacks::config.lockDocks = lock;
}
