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
#include <QDockWidget>
#include <QMainWindow>
#include <QMetaObject>
#include <QPainterPath>
#include <QSplitter>
#include <QTimer>
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
    painter->setPen(FRAME_COLOR);
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawRect(RECT.adjusted(0,0,-1,-1));
    RESTORE_PAINTER
}

static QPixmap *gs_overlay = 0L;

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

    gs_overlay = new QPixmap(450,360);
    gs_overlay->fill(Qt::transparent);
    QPainter p(gs_overlay);
    QColor color(value,value,value,(alpha+16)*112/255);
    p.setPen(color);
//     p.setPen(Qt::NoPen);
    color.setAlpha(24*(alpha+16)/255);
    p.setBrush(color);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPath(ringPath);
    p.end();
}

static inline void
createImperialPix(const QColor &c)
{
    gs_overlay = new QPixmap(240,240);
    gs_overlay->fill(Qt::transparent);
    QPainter p(gs_overlay);
    p.setBrush(Qt::NoBrush);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(c);
    pen.setWidth(10);
    p.setPen(pen);
    p.drawEllipse(5,5,230,230);
    pen.setCapStyle(Qt::FlatCap); // w/o flat cap, we won't get segments for the thicker pens
    pen.setDashPattern(QVector<qreal>() << 1 << 10.1);
    pen.setDashOffset(6);
    p.setPen(pen);
    p.drawEllipse(14,14,212,212);
    pen.setWidth(28);
    pen.setDashPattern(QVector<qreal>() << 1.5 << 1.8);
    pen.setDashOffset(-0.9);
    p.setPen(pen);
    p.drawEllipse(32,32,176,176);
    pen.setWidth(60);
    pen.setDashPattern(QVector<qreal>() << 0.8 << 0.3);
    pen.setDashOffset(-0.125);
    p.setPen(pen);
    // the blocks of the innermost ring cover the 0° (3 o'clock) position and the flat cap causes a
    // gap (flat cap is required to have segments at all at this size ;-)
    // so we paint an arc and start at the uncovered 90° position
    p.drawArc(QRect(57,57,126,126), 90*16, 360*16);
    p.end();
}

static inline void
createTronPix(const QColor &c)
{
    gs_overlay = new QPixmap(240,240);
    gs_overlay->fill(Qt::transparent);
    QPainter p(gs_overlay);
    p.setBrush(Qt::NoBrush);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(c);
    pen.setWidth(16);
    p.setPen(pen);
    p.drawEllipse(8,8,224,224);
    pen.setWidth(8);
    p.setPen(pen);
    p.drawEllipse(56,56,128,128);
    p.end();
}

static inline void
createPlasmaPix(const QColor &c)
{
    gs_overlay = new QPixmap(210,210);
    gs_overlay->fill(Qt::transparent);
    QPainter p(gs_overlay);
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawEllipse(40,0,24,24);
    p.drawEllipse(0,80,36,36);
    p.drawEllipse(52,158,52,52);
    p.setPen(QPen(c, 36, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    QPainterPath path;
    path.moveTo(130,16);
    path.lineTo(180,64);
    path.lineTo(130,120);
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    p.end();
}

static inline void
createArtDecoPix(const QColor &c, Qt::Orientation o)
{
    if (o == Qt::Horizontal)
        gs_overlay = new QPixmap(320,160);
    else
        gs_overlay = new QPixmap(160,320);
    gs_overlay->fill(Qt::transparent);
    QPainter p(gs_overlay);
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    for (int y = 0; y < 160-24; y += 24) {
        const int x = 32 + (y%48 ? -1 : 1)*16 + (y%72 ? -1 : 1)*8 + (y%96 ? -1 : 1)*8;
        if (o == Qt::Horizontal)
            p.drawRect(x, y, 320-x, 16);
        else
            p.drawRect(y, 0, 16, 320-x);
    }
    p.end();
}

void
Style::resetRingPix()
{
    delete gs_overlay; gs_overlay = 0L;
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
                if (config.invert.toolbars) {
                    foreach (QObject *o, dvc->children()) {
                        if (QWidget *dsb = widgetOfClass("DolphinStatusBar", o)) {
                            r.adjust(0,0,0, -dsb->height());
                        }
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
    if (widget->windowType() == Qt::Dialog && config.bg.ringOverlay) {
        static QRgb lastRgb = 0;
        static QTimer *ringResetTimer = NULL;

        const QColor bgColor = pal.color(widget->backgroundRole());
        if (!gs_overlay || bgColor.rgb() != lastRgb) {
            lastRgb = bgColor.rgb();
//             int ringValue = (FX::value(pal.color(widget->backgroundRole())) + 128) / 2; //[64,191]
//             ringValue += (64 - qAbs(ringValue - 128))/2; //[64,191]
            int ringValue = FX::value(bgColor);
            if (ringValue < 48)
                ringValue += qMax(48-ringValue,24);
            else if (ringValue < 160)
                ringValue -= 24;
            else if (ringValue < 236)
                ringValue = qMin(ringValue+24,255);
            else
                ringValue -= 24;
            const QColor grey(ringValue, ringValue, ringValue);
            switch (config.bg.ringOverlay) {
                default:
                case 1: createRingPix(255, ringValue); break;
                case 2: createImperialPix(FX::blend(bgColor, grey, 10, 1)); break;
                case 3: createTronPix(FX::blend(bgColor, grey, 10, 1)); break;
                case 4: createPlasmaPix(FX::blend(bgColor, grey, 5, 1)); break;
                case 5: createArtDecoPix(FX::blend(bgColor, grey, 20, 1), Qt::Horizontal); break;
                case 6: createArtDecoPix(FX::blend(bgColor, grey, 20, 1), Qt::Vertical); break;
            }
            if (!ringResetTimer) {
                ringResetTimer = new QTimer(const_cast<BE::Style*>(this));
                ringResetTimer->setSingleShot(true);
                connect(ringResetTimer, SIGNAL(timeout()), SLOT(resetRingPix()));
                connect(ringResetTimer, &QObject::destroyed, [=]() {ringResetTimer = nullptr;});
                connect(ringResetTimer, SIGNAL(destroyed()), SLOT(resetRingPix()));
            }
        }
        int x(widget->width()-gs_overlay->width()), y(0);
        switch (config.bg.ringOverlay) {
            case 6: x -= 32; // fall through
            case 1: break;
            default: x -= 32; // fall through
            case 5: y = widget->height() - (gs_overlay->height() + 48); break;
        }
        painter->drawPixmap(x, y, *gs_overlay);
        if (ringResetTimer)
            ringResetTimer->start(5000);
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
    const int v = FX::value(c);
    if (v < 16)
        c = c.lighter(120);
    else if (v > 240 || v < 128)
        c = c.darker(120);
    else // 128 - 240
        c = c.lighter(120);
//     painter->setPen(FCOLOR(ToolTipText));
    painter->setPen(c);
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
    // windows, docks etc. - just a frame
    SAVE_PAINTER(Pen|Brush|Alias);
    painter->setPen(FRAME_COLOR);
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint(QPainter::Antialiasing, false);
    QPainterPath path;
    path.moveTo(RECT.bottomRight());
    path.lineTo(RECT.topRight());
    path.lineTo(RECT.topLeft());
    path.lineTo(RECT.bottomLeft());
    painter->drawPath(path);
    RESTORE_PAINTER

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


