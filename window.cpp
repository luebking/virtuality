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

//    case PE_PanelTipLabel: // The panel for a tip label.

#include <QApplication>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QMainWindow>
#include <QMetaObject>
#include <QSplitter>
#include <QtDebug>
#include "draw.h"

#include "FX.h"

#ifdef BE_WS_X11
#include "xproperty.h"
#else
#define QT_NO_XRENDER #
#endif

static void shapeCorners( QPainter *p, const QRect &r, int rnd )
{
    rnd = qMin(rnd, 6);
    static QPixmap mask;
    if (mask.isNull()) {
        mask = QPixmap(2*rnd+1, 2*rnd+1);
        mask.fill(Qt::transparent);
        QPainter p(&mask);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.drawEllipse(mask.rect());
        p.end();
    }
    p->setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p->drawPixmap(r.x(), r.y(), mask, 0, 0, rnd, rnd);
    p->drawPixmap(r.right() - rnd + 1, r.y(), mask, rnd + 1, 0, rnd, rnd);
    p->drawPixmap(r.right() - rnd + 1, r.bottom() - rnd + 1, mask, rnd + 1, rnd + 1, rnd, rnd);
    p->drawPixmap(r.x(), r.bottom() - rnd + 1, mask, 0, rnd + 1, rnd, rnd);
    p->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void
Style::drawWindowFrame(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    // windows, docks etc. - just a frame
    SAVE_PAINTER(Pen|Brush|Alias);
    painter->setPen(FCOLOR(WindowText));
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawRect(RECT.adjusted(0,0,-1,-1));
    RESTORE_PAINTER
}

static QPixmap *rings = 0L;
#include <QTimer>
static QTimer ringResetTimer;
static inline void
createRingPix(int alpha, int value)
{
    QPainterPath ringPath;
//     ringPath.setFillRule(Qt::WindingFill); // Qt::OddEvenFill (default)
    ringPath.addEllipse(0,0,200,200);
    ringPath.addEllipse(30,30,140,140);

    ringPath.addEllipse(210,10,230,230);
    ringPath.addEllipse(218,18,214,214);
    ringPath.addEllipse(226,26,198,198);
    ringPath.addEllipse(234,34,182,182);
    ringPath.addEllipse(300,100,50,50);

    ringPath.addEllipse(100,96,160,160);
    ringPath.addEllipse(108,104,144,144);
    ringPath.addEllipse(116,112,128,128);
    ringPath.addEllipse(122,120,112,112);

    ringPath.addEllipse(250,160,200,200);
    ringPath.addEllipse(280,190,140,140);
    ringPath.addEllipse(310,220,80,80);

    rings = new QPixmap(450,360);
    rings->fill(Qt::transparent);
    QPainter p(rings);
    QColor color(value,value,value,(alpha+16)*112/255);
    p.setPen(color);
//     p.setPen(Qt::NoPen);
    color.setAlpha(24*(alpha+16)/255);
    p.setBrush(color);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPath(ringPath);
    p.end();
}
void
Style::resetRingPix()
{
    ringResetTimer.stop();
    delete rings; rings = 0L;
}

static inline
QWidget *widgetOfClass(const char *className, QObject *o)
{
    if (o->isWidgetType() && !qstrcmp(o->metaObject()->className(), className))
        return static_cast<QWidget*>(o);
    return NULL;
}

static inline
QWidget *dolphinViewContainer(bool search, const QWidget *from) {
    if (!search)
        return NULL;
    foreach (QObject *runner, from->children()) {
        if (qobject_cast<QSplitter*>(runner)) {
            from = static_cast<QWidget*>(runner);
            foreach (runner, from->children()) {
                if (QWidget *dvc = widgetOfClass("DolphinViewContainer", runner))
                    return dvc;
            }
        }
    }
    return NULL;
}

void
Style::drawWindowBg(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!widget)
        return;
    if (widget->parentWidget() && widget->property("Virtuality.centralWidget").toBool()) {
        const int rnd = config.frame.roundness;
        static QPixmap mask(2*rnd+1, 2*rnd+1);
        static QColor lastColor;
        if (!lastColor.isValid() || lastColor != PAL.color(QPalette::Active, QPalette::WindowText)) {
            lastColor = PAL.color(QPalette::Active, QPalette::WindowText);
            mask.fill(Qt::transparent);
            mask.fill(lastColor);
            QPainter p(&mask);
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::black);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setRenderHint(QPainter::Antialiasing);
            p.drawEllipse(mask.rect());
            p.end();
        }

        QWidget *window = widget->window();
        const bool invertTitle = window->property("Virtuality.invertTitlebar").toBool();
        const QRect parentRect(widget->parentWidget()->rect());
        const QRect windowRect(window->rect());
        const QRect geo(widget->geometry());
        QRect r(widget->rect());
        const QRect winGeo = (window == widget->parentWidget()) ? geo : r.translated(widget->mapTo(window, r.topLeft()));

        Qt::DockWidgetAreas areas;
        if (QMainWindow *mw = qobject_cast<QMainWindow*>(widget->parentWidget())) {
            foreach (QObject *o, mw->children()) {
                if (o->isWidgetType() && static_cast<QWidget*>(o)->isVisible())
                if (QDockWidget *dock = qobject_cast<QDockWidget*>(o))
                    areas |= mw->dockWidgetArea(dock);
            }
        }

        bool il(r.x() == winGeo.x()), ir(r.right() == winGeo.right()),
             it(invertTitle && r.y() == winGeo.y()), ib(r.bottom() == winGeo.bottom());

        if (!il && geo.x() > parentRect.x()) {
            il = (areas & Qt::LeftDockWidgetArea);
            if (!il) {
                QWidget *sibling = widget->parentWidget()->childAt(geo.x()-1, geo.y());
                il = sibling && sibling->property("Virtuality.inverted").toBool();
            }
        }
        if (!ir && geo.right() < parentRect.right()) {
            ir = (areas & Qt::RightDockWidgetArea);
            if (!ir) {
                QWidget *sibling = widget->parentWidget()->childAt(geo.right()+1, geo.y());
                ir = sibling && sibling->property("Virtuality.inverted").toBool();
            }
        }

        if (il || ir) {
            if (!it && geo.y() > parentRect.y()) {
                it = (areas & Qt::TopDockWidgetArea);
                if (!it) {
                    QWidget *sibling = widget->parentWidget()->childAt(geo.x(), geo.y()-1);
                    it = sibling && sibling->property("Virtuality.inverted").toBool();
                }
            }
            if (QWidget *dvc = dolphinViewContainer(appType == Dolphin, widget)) {
                ib = config.invert.toolbars || config.invert.docks;
                foreach (QObject *o, dvc->children()) {
                    if (QWidget *dsb = widgetOfClass("DolphinStatusBar", o)) {
                        r.adjust(0,0,0, -dsb->height());
                    }
                }
            } else if (!ib && geo.bottom() < parentRect.bottom()) {
                ib = (areas & Qt::BottomDockWidgetArea);
                if (!ib) {
                    QWidget *sibling = widget->parentWidget()->childAt(geo.x(), geo.bottom()+1);
                    ib = sibling && sibling->property("Virtuality.inverted").toBool();
                }
            }
            if (il) {
                if (it)
                    painter->drawPixmap(r.x(), r.y(), mask, 0, 0, rnd, rnd);
                if (ib)
                    painter->drawPixmap(r.x(), r.bottom() - rnd + 1, mask, 0, rnd + 1, rnd, rnd);
            }
            if (ir) {
                if (it)
                    painter->drawPixmap(r.right() - rnd + 1, r.y(), mask, rnd + 1, 0, rnd, rnd);
                if (ib)
                    painter->drawPixmap(r.right() - rnd + 1, r.bottom() - rnd + 1, mask, rnd + 1, rnd + 1, rnd, rnd);
            }
        }
        return;
    }
    // Invalid attempts --------------------------------------------------------
    if (!widget->isWindow())
        return; // can't do anything here
        // err... no. splashscreens want their own bg? but this applies to popups as well ???
//     if ( widget->windowFlags() & (Qt::SplashScreen & ~Qt::Window) )
//         return;

//     if (widget->testAttribute(Qt::WA_NoSystemBackground))
//         return; // those shall be translucent - but should be catched by Qt

    const QPalette &pal = widget->palette();
    if (pal.brush(widget->backgroundRole()).style() > 1)
        return; // we'd cover a gradient/pixmap/whatever

    QColor c = pal.color(widget->backgroundRole());
    if (c == Qt::transparent) // plasma uses this
        return;
    if (c.alpha() == 0xff && (widget->windowFlags() & Qt::WindowType_Mask) == Qt::Popup && widget->style() == this) {
            c.setAlpha(config.bg.modal.opacity);
    }

//     if (c.alpha() < 0xff && QPainter::redirected(painter->device()) != widget) {
//         QPainter p(const_cast<QWidget*>(widget));
//         p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//         p.fillRect( widget->rect(), Qt::black );
//         p.end();
//     }
    painter->fillRect( widget->rect(), c );

    // Ensure ring texture --------------------------------------------------------
    bool drawRings = false;
    if (false && config.bg.ringOverlay) {
        const bool hasTitleBar = !(widget->windowFlags() & ((Qt::Popup | Qt::ToolTip | Qt::SplashScreen | Qt::Desktop | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint) & ~Qt::Window));
        drawRings = hasTitleBar;
        if (drawRings && !rings) {
//             int ringValue = (FX::value(pal.color(widget->backgroundRole())) + 128) / 2; //[64,191]
//             ringValue += (64 - qAbs(ringValue - 128))/2; //[64,191]
            int ringValue = FX::value(pal.color(widget->backgroundRole()));
            if (ringValue < 48)
                ringValue += qMax(48-ringValue,24);
            else if (ringValue < 160)
                ringValue -= 24;
            else if (ringValue < 236)
                ringValue = qMin(ringValue+24,255);
            else
                ringValue -= 24;
            createRingPix(255, ringValue);
            disconnect(&ringResetTimer, SIGNAL(timeout()), this, SLOT(resetRingPix()));
            connect(&ringResetTimer, SIGNAL(timeout()), this, SLOT(resetRingPix()));
        }
        ringResetTimer.start(5000);
        if (drawRings)
            painter->drawPixmap(widget->width()-450, 0, *rings);
    }
    if (widget->testAttribute(Qt::WA_TranslucentBackground)) {
        const QVariant wdv = widget->property("BespinWindowHints");
        const int windowDecoration = wdv.isValid() ? wdv.toInt() : 0;
        if ( windowDecoration & Rounded )
            shapeCorners( painter, widget->rect(), config.frame.roundness );
    }
}

void
Style::drawToolTip(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    SAVE_PAINTER(Pen|Brush|Alias);
    QColor c(FCOLOR(ToolTipBase));
    if (widget && widget->testAttribute(Qt::WA_TranslucentBackground) && FX::compositingActive())
        c.setAlpha(config.bg.modal.opacity);
    painter->setBrush(c);
    painter->setPen(FCOLOR(ToolTipText));
    if (config.frame.roundness && widget && widget->testAttribute(Qt::WA_TranslucentBackground) && FX::compositingActive()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        const int rnd = qMin(config.frame.roundness, 6);
        painter->drawRoundedRect(QRectF(RECT).adjusted(0.5,0.5,-0.5,-0.5),rnd,rnd);
    } else {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(RECT.adjusted(0,0,-1,-1));
    }
    RESTORE_PAINTER
}

#define PAINT_WINDOW_BUTTON(_btn_) {\
    tmpOpt.rect = subControlRect(CC_TitleBar, tb, SC_TitleBar##_btn_##Button, widget);\
    if (!tmpOpt.rect.isNull())\
    { \
        if (tb->activeSubControls & SC_TitleBar##_btn_##Button)\
            tmpOpt.state = tb->state;\
        else\
            tmpOpt.state &= ~(State_Sunken | State_MouseOver);\
        if (!(tmpOpt.state & State_MouseOver))\
            tmpOpt.rect.adjust(F(2), F(2), -F(2), -F(2));\
        painter->drawPixmap(tmpOpt.rect.topLeft(), standardPixmap(SP_TitleBar##_btn_##Button, &tmpOpt, widget));\
   }\
}

void
Style::drawTitleBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
    if (!tb) return;

    QRect ir;

    // the label
    if (option->subControls & SC_TitleBarLabel) {
        SAVE_PAINTER(Pen);
        ir = subControlRect(CC_TitleBar, tb, SC_TitleBarLabel, widget);
        painter->setPen(PAL.color(QPalette::WindowText));
        ir.adjust(F(2), 0, -F(2), 0);
        painter->drawText(ir, Qt::AlignCenter | Qt::TextSingleLine, tb->text);
        RESTORE_PAINTER
    }

    QStyleOptionTitleBar tmpOpt = *tb;
    if (tb->subControls & SC_TitleBarCloseButton)
        PAINT_WINDOW_BUTTON(Close)

    if (tb->subControls & SC_TitleBarMaxButton && tb->titleBarFlags & Qt::WindowMaximizeButtonHint) {
        if (tb->titleBarState & Qt::WindowMaximized)
            PAINT_WINDOW_BUTTON(Normal)
        else
            PAINT_WINDOW_BUTTON(Max)
    }

    if (tb->subControls & SC_TitleBarMinButton && tb->titleBarFlags & Qt::WindowMinimizeButtonHint) {
        if (tb->titleBarState & Qt::WindowMinimized)
            PAINT_WINDOW_BUTTON(Normal)
        else
            PAINT_WINDOW_BUTTON(Min)
    }

    if (tb->subControls & SC_TitleBarNormalButton && tb->titleBarFlags & Qt::WindowMinMaxButtonsHint)
        PAINT_WINDOW_BUTTON(Normal)

    if (tb->subControls & SC_TitleBarShadeButton)
        PAINT_WINDOW_BUTTON(Shade)

    if (tb->subControls & SC_TitleBarUnshadeButton)
        PAINT_WINDOW_BUTTON(Unshade)

    if (tb->subControls & SC_TitleBarContextHelpButton && tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
        PAINT_WINDOW_BUTTON(ContextHelp)

    if (tb->subControls & SC_TitleBarSysMenu && tb->titleBarFlags & Qt::WindowSystemMenuHint) {
        if (!tb->icon.isNull()) {
            ir = subControlRect(CC_TitleBar, tb, SC_TitleBarSysMenu, widget);
            tb->icon.paint(painter, ir);
        }
    //    else
    //       PAINT_WINDOW_BUTTON(SC_TitleBarSysMenu, SP_TitleBarMenuButton)
    }
#undef PAINT_WINDOW_BUTTON
}

void
Style::drawSizeGrip(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
    SAVE_PAINTER(Pen|Brush|Alias);
    QRect r(0,0,F(8),F(8));
    if (option->direction == Qt::RightToLeft)
        r.moveBottomRight(RECT.bottomRight());
    else
        r.moveBottomLeft(RECT.bottomLeft());
    if (const QStyleOptionSizeGrip *sgOpt = qstyleoption_cast<const QStyleOptionSizeGrip *>(option)) {
        switch (sgOpt->corner) {
        case Qt::BottomLeftCorner: r.moveBottomLeft(RECT.bottomLeft()); break;
        default: case Qt::BottomRightCorner: r.moveBottomRight(RECT.bottomRight()); break;
        case Qt::TopLeftCorner: r.moveTopLeft(RECT.topLeft()); break;
        case Qt::TopRightCorner: r.moveTopRight(RECT.topRight()); break;
        }
    }
    r.adjust(F(2),F(2),-F(2),-F(2));

    painter->setPen(Qt::NoPen);
    painter->setBrush(FX::blend(FCOLOR(Window),FCOLOR(WindowText)));
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawEllipse(r);
    RESTORE_PAINTER
}


