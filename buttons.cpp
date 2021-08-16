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
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QLinearGradient>
#include <QStyleOptionButton>
#include <QStyleOptionMenuItem>

#include "draw.h"
#include "animator/hover.h"

#include <QtDebug>
#define MAX_STEPS Animator::Hover::maxSteps()
#define HOVER_STEP sunken ? MAX_STEPS : ((appType == GTK || !widget) ? MAX_STEPS*hover : Animator::Hover::step(widget))

static struct AnimPair { const QWidget *widget; int step; } anim = {0,0};

static bool isCheckableButton(const QWidget *w, const QStyleOption *option)
{
    const QAbstractButton *b = qobject_cast<const QAbstractButton*>(w);
    if (b && b->isCheckable()) {
        if (b->maximumWidth() < QWIDGETSIZE_MAX || RECT.width() < RECT.height()/3 + F(32)) {
            // fixed size buttons as in kcron won't - there's no space for the indicator
            if (option->state & QStyle::State_On) // we cheat and make the button focus'd if on
                const_cast<QStyleOption*>(option)->state |= QStyle::State_HasFocus;
            return false; // not a regular checkable button
        }
        return true;
    }
    return false;
}

void
Style::drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(btn, Button);
    OPT_SUNKEN OPT_HOVER OPT_FOCUS;

    if ( widget ) {
        if ( qobject_cast<const QAbstractItemView*>(widget) ) {
            painter->fillRect(RECT, FX::blend(FCOLOR(Base), FCOLOR(Text), 3,1));
            return;
        } else if ( widget->inherits("QWebView") ) {
            // paints hardcoded black text bypassing the style?! grrr...
            widget = 0; // leads to false UnderMouse assumptions...
        }
    }
    anim.widget = widget;
    anim.step = HOVER_STEP;

    if (btn->features & QStyleOptionButton::CommandLinkButton) {
        OPT_FOCUS
        if (hasFocus)
            drawPushButtonBevel(btn, painter, widget);
        const QColor c = FX::blend(FCOLOR(ButtonText), FCOLOR(Highlight), MAX_STEPS - anim.step, anim.step);
        const_cast<QStyleOptionButton*>(btn)->palette.setColor(QPalette::ButtonText, c);
    } else if (isCheckableButton(widget, option)) {
        if (!(option->state & State_On))
            const_cast<QStyleOption*>(option)->state |= State_Off; // fix state - there's no tristate pushbutton
        drawCheckBoxItem(option, painter, widget);
    } else {
        drawPushButtonBevel(btn, painter, widget);
        if (appType == GTK)
            return; // GTK paints the label itself
//         tmpBtn.rect = subElementRect(SE_PushButtonContents, btn, widget);
        drawPushButtonLabel(btn, painter, widget);
    }
    anim.widget = 0; anim.step = 0;
}

void
Style::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{

//     if (widget && widget->parentWidget() && widget->parentWidget()->inherits("KPIM::StatusbarProgressWidget"))
//         return;

    ASSURE_OPTION(btn, Button);

    OPT_SUNKEN OPT_HOVER

    bool resetAnim = false;
    if ( !widget || widget != anim.widget )
        { resetAnim = true; anim.widget = widget; anim.step = HOVER_STEP; }

    drawButtonFrame(option, painter, widget);

    if (btn->features & QStyleOptionButton::HasMenu) {
        int sz = (RECT.height()-F(6))/2;
        QRect rect = RECT;
        rect.setLeft(RECT.right() - (F(6)+sz));
        rect.setWidth(sz);
        SAVE_PAINTER(Pen|Brush);
        painter->setPen(Qt::NoPen);
        painter->setBrush(FX::blend(FCOLOR(Window), FCOLOR(WindowText), 12-anim.step, anim.step));
        drawArrow(Navi::S, rect, painter);
        RESTORE_PAINTER
    }
    if ( resetAnim )
        { anim.widget = 0; anim.step = 0; }
}

void
Style::drawButtonFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget, int _animStep) const
{
    B_STATES

    const QAbstractButton *btn = qobject_cast<const QAbstractButton*>(widget);
    bool resetAnim = false;
    if (widget && !btn && widget->inherits("QAbstractItemView"))
        { hover = false; anim.step = 0; }
    else if (_animStep > -1)
        { resetAnim = true; anim.widget = widget; anim.step = _animStep; }
    else if ( !widget || widget != anim.widget )
        { resetAnim = true; anim.widget = widget; anim.step = HOVER_STEP; }

    if (isCheckableButton(widget, option)) {
        QStyleOption opt = *option;
        if (!(opt.state & State_On))
            opt.state |= State_Off;
        opt.rect = subElementRect(SE_CheckBoxIndicator, &opt, widget);
        drawRadioOrCheckBox(&opt, painter, widget, false);
    } else {
        hasFocus = option->state & QStyle::State_HasFocus; // might have been changed by isCheckableButton check
        QColor oc(FCOLOR(WindowText));
        if (widget && widget->testAttribute(Qt::WA_SetPalette)) { // button has custom palette - see which color differs
            const QColor globalBg = QApplication::palette().color(PAL.currentColorGroup(), QPalette::Button),
                         globalFg = QApplication::palette().color(PAL.currentColorGroup(), QPalette::ButtonText);
            if (globalBg != FCOLOR(Button) && FX::haveContrast(FCOLOR(Button), FCOLOR(Window)))
                oc = FCOLOR(Button);
            else if (globalFg != FCOLOR(ButtonText) && FX::haveContrast(FCOLOR(ButtonText), FCOLOR(Window)))
                oc = FCOLOR(ButtonText);
        }
        if (!isEnabled) {
            sunken = hover = false;
            anim.step = 0;
//         } else if (!sunken) {
//             if (hasFocus)
//                 anim.step = MAX_STEPS;
//             else if HAVE_OPTION(btn, Button)
//             if (btn->features & QStyleOptionButton::DefaultButton) {
//                 anim.step = 1;
//             }
        }

        STROKED_RECT(r, RECT);
        bool squareButton = false;
        if (r.width() > r.height() + F(8)) {
            if (!hasFocus) {
//                 r.setHeight(r.height() - F(1)*(MAX_STEPS-anim.step)/2);
                r.setWidth(r.height() + anim.step*(r.width()-r.height())/MAX_STEPS);
            }
            r.adjust(F(4)*sunken, 0, -F(4)*sunken, 0);
        } else {
            squareButton = true;
            r.setHeight(qMin(r.height(), r.width()));
        }

        SAVE_PAINTER(Pen|Brush|Alias);

        QColor c(oc);
        if (sunken) {
            c = FCOLOR(Highlight);
        } else {
            c = FX::blend(FCOLOR(Window), c, MAX_STEPS, 1 + (squareButton+1)*anim.step);
            if (hasFocus)
                c = FX::blend(FCOLOR(Highlight), c, MAX_STEPS-anim.step, anim.step);
        }

        r.moveCenter(FLOAT_CENTER(RECT));
        if (sunken) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(c);
            if (int(Style::halfStroke) != Style::halfStroke)
                r.adjust(0.5f, 0.5f, -0.5f, -0.5f);
        } else {
            painter->setPen(QPen(c, FRAME_STROKE));
            painter->setBrush(Qt::NoBrush);
        }
        painter->setRenderHint(QPainter::Antialiasing);
        const int radius = r.height()/2;

        if (!(squareButton || sunken || hasFocus || anim.step)) {
            QLinearGradient lg(r.x(), r.y(), r.x(), r.bottom());
            const QColor c1(FX::blend(FCOLOR(Window), oc, MAX_STEPS, 1 + MAX_STEPS/2));
            lg.setColorAt(0.0, c1);
            lg.setColorAt(0.25, c);
            lg.setColorAt(0.75, c);
            lg.setColorAt(1.0, c1);
            painter->setPen(QPen(lg, FRAME_STROKE));
        }

        painter->drawRoundedRect(r, radius, radius);
#if false // rails
        if (!(sunken || hasFocus) && anim.step < MAX_STEPS) {
            const int x = RECT.x() + RECT.width() / 2;
            const int w = RECT.width()/6 + anim.step*RECT.width()/(3*MAX_STEPS);
            painter->drawLine(x, r.y(), RECT.right() - w, r.y());
            painter->drawLine(RECT.x() + w, r.bottom() + F(1), x, r.bottom() + F(1));
        }
#endif
#if false // swapping outer bound
        if (!(hasFocus || sunken) && RECT.width() > RECT.height() + F(8)) {
            QRect r(RECT);
            r.setWidth(r.height() + (MAX_STEPS-anim.step)*(r.width()-r.height())/MAX_STEPS);
            r.adjust(padding + F(4)*sunken, padding, -(padding + F(4)*sunken), -padding);
            r.moveCenter(RECT.center());
            QColor c(oc);
            c.setAlpha(c.alpha()*(MAX_STEPS - anim.step)/(8*MAX_STEPS));
            painter->setPen(QPen(c, F(2)));
            painter->drawRoundedRect(r, radius, radius);
        }
#endif
        RESTORE_PAINTER
    }

    if ( resetAnim )
        { anim.widget = 0; anim.step = 0; }
}

void
Style::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(btn, Button);
    OPT_ENABLED OPT_FOCUS OPT_HOVER OPT_SUNKEN;
    bool resetAnim = false;
    if ( !widget || widget != anim.widget )
        { resetAnim = true; anim.widget = widget; anim.step = HOVER_STEP; }

    QRect ir = RECT;
    if (!btn->text.isEmpty()) {
        SAVE_PAINTER(Pen);
        if (sunken)
            painter->setPen(FCOLOR(HighlightedText));
        else if (hasFocus)
            painter->setPen(FX::blend(FCOLOR(WindowText), FCOLOR(Highlight), MAX_STEPS-anim.step, anim.step));
        else if (btn->features & QStyleOptionButton::DefaultButton) {
            painter->setPen(FX::blend(FCOLOR(WindowText), FCOLOR(Highlight),1,2));
        } else {
//             const int step = MAX_STEPS - qAbs(2 * (anim.step - MAX_STEPS/2));
            painter->setPen(FX::blend(FCOLOR(WindowText), FCOLOR(Highlight), MAX_STEPS-anim.step, anim.step));
//             painter->setPen(FX::blend(FCOLOR(WindowText), FCOLOR(Window), MAX_STEPS-step, step));
//             painter->setPen(FCOLOR(WindowText));
        }
        drawItemText(painter, ir, Qt::AlignCenter | BESPIN_MNEMONIC, PAL, isEnabled, btn->text, QPalette::NoRole, &ir);
        RESTORE_PAINTER
    } else if (!btn->icon.isNull()) {   // The ICON ================================================
        QIcon::Mode mode = isEnabled ? QIcon::Normal : QIcon::Disabled;
        if (mode == QIcon::Normal && hasFocus)
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (btn->state & State_On)
            state = QIcon::On;
        QPixmap pixmap = btn->icon.pixmap(btn->iconSize, mode, state);
        int pixw = pixmap.width();
        int pixh = pixmap.height();

        //Center the icon if there is no text (and it's no checkbutton)
        QPoint point;
        if (btn->text.isEmpty())
            point = QPoint(ir.x() + (ir.width() - pixw) / 2, ir.y() + (ir.height() - pixh) / 2);
        else
            point = QPoint(ir.x() - (F(2) + pixw), ir.y() + (ir.height() - pixh) / 2);

        if (btn->direction == Qt::RightToLeft)
            point.rx() += pixw/2;

        painter->drawPixmap(visualPos(btn->direction, btn->rect, point), pixmap);
    }

    if ( resetAnim )
        { anim.widget = 0; anim.step = 0; }
}

#define DRAW_SEGMENT(_R_, _START_, _SPAN_) (hasFocus || qAbs(_SPAN_) > 5759) ? painter->drawEllipse(_R_) : painter->drawArc(_R_, _START_, _SPAN_)

void
Style::drawRadioOrCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget, bool isRadio) const
{
    OPT_ENABLED OPT_HOVER OPT_SUNKEN OPT_FOCUS

    if ( widget && widget->inherits("QWebView") )
        widget = 0;

    char state = (option->state & State_On) ? 2 : 0;
    if (!isRadio && !(option->state & (State_On|State_Off))) // tristate
        state = 1;
    if (sunken)
        state = state ? (state + 1) % 3 : 2;

    const int s = (qMin(RECT.width(), RECT.height()) & ~1);
    STROKED_RECT(r, QRect(RECT.topLeft(), QSize(s-FRAME_STROKE_WIDTH, s-FRAME_STROKE_WIDTH)));
    if (option->direction == Qt::RightToLeft)
        r.moveRight(RECT.right() + halfStroke);

    SAVE_PAINTER(Pen|Brush|Alias);

    painter->setRenderHint(QPainter::Antialiasing, true);

    int animStep = (isRadio && state) || option->state & QStyle::State_Item ? 0 : HOVER_STEP;
    const int a = animStep*168/(2*MAX_STEPS);
    if (isEnabled && (option->state & State_Selected)) { // popup menu
        painter->setPen(QPen(FCOLOR(Highlight), FRAME_STROKE));
        painter->setBrush(Qt::NoBrush);
    } else if (sunken) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(FCOLOR(Highlight));
    } else {
        const QColor c(FX::blend(FCOLOR(Window), FCOLOR(WindowText)));
        painter->setPen(QPen(hasFocus ? FX::blend(FCOLOR(Highlight), c, MAX_STEPS-animStep, animStep) : c, FRAME_STROKE));
        painter->setBrush(Qt::NoBrush);
    }
    if (isRadio) {
        if (option->direction == Qt::LeftToRight)
            DRAW_SEGMENT(r, -16*(96+a), 16*(192+2*a));
        else
            DRAW_SEGMENT(r, 16*(84-a), 16*(192+2*a));
    } else
        DRAW_SEGMENT(r, 16*(6+a), -16*(192+2*a));

    if (state) { // the drop
        painter->setPen(Qt::NoPen);
        const int d = intMin(F(4), r.height()/3);
        r.adjust(d, d, -d, -d);
        if (sunken) {
            painter->setBrush(FCOLOR(HighlightedText));
        } else if (hasFocus) {
            painter->setBrush(FX::blend(FCOLOR(WindowText), FCOLOR(Highlight), MAX_STEPS - animStep, animStep));
        } else {
            painter->setBrush(FCOLOR(WindowText));
        }
        if (state == 1) // tri-state
            painter->drawChord(r, -180*16, 180*16);
        else
            painter->drawEllipse(r);
    }
    RESTORE_PAINTER
}

void
Style::drawRadioItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(btn, Button);
    QStyleOptionButton subopt = *btn;
    subopt.rect = subElementRect(SE_RadioButtonIndicator, btn, widget);
    drawRadio(&subopt, painter, widget);
    subopt.rect = subElementRect(SE_RadioButtonContents, btn, widget);
    drawCheckLabel(&subopt, painter, widget);
}

void
Style::drawCheckBoxItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(btn, Button);
    QStyleOptionButton subopt = *btn;
    subopt.rect = subElementRect(SE_CheckBoxIndicator, btn, widget);
    drawCheckBox(&subopt, painter, widget);
    subopt.rect = subElementRect(SE_CheckBoxContents, btn, widget);
    drawCheckLabel(&subopt, painter, widget);
}

void
Style::drawCheckLabel(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    ASSURE_OPTION(btn, Button);
    OPT_ENABLED

    uint alignment = visualAlignment(btn->direction, Qt::AlignLeft)  | Qt::AlignVCenter;
    QRect textRect = RECT;

    if (!btn->icon.isNull()) {
        const QPixmap pix = btn->icon.pixmap(btn->iconSize, isEnabled ? QIcon::Normal : QIcon::Disabled);
        drawItemPixmap(painter, btn->rect, alignment, pix);
        if (btn->direction == Qt::RightToLeft)
            textRect.setRight(textRect.right() - btn->iconSize.width() - F(4));
        else
            textRect.setLeft(textRect.left() + btn->iconSize.width() + F(4));
    }
    if (!btn->text.isEmpty())
        drawItemText(painter, textRect, alignment | BESPIN_MNEMONIC, PAL, isEnabled, btn->text, QPalette::WindowText);
}
