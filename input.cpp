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
#include <QComboBox>
#include <QPainterPath>
#include <QToolBar>
#include "draw.h"
#include "hacks.h"
#include "animator/focus.h"
#include "animator/hover.h"

#include <QtDebug>
#define MAX_STEPS Animator::Hover::maxSteps()
#define LEF_COLOR FX::blend(FCOLOR(Window), FCOLOR(WindowText), 8, isEnabled+1)

void
Style::drawLineEditFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (RECT.height() > qMax(32, 2*(FRAME_STROKE_WIDTH + painter->fontMetrics().height()))) { // abuse for mass text, eg. by webkit
        drawFrame(option, painter, widget);
        return;
    }
    OPT_ENABLED OPT_FOCUS
    SAVE_PAINTER(Pen|Alias);
    STROKED_RECT(r, RECT);
    QRectF r2(r);
    r2.setWidth(r2.height());
    r2.moveRight(r.right());
    QPainterPath path;
    path.moveTo(r.x() + config.frame.roundness, r.bottom()); // +config.frame.roundness to match aligned fames
    path.lineTo(r2.bottomLeft());
    path.arcTo(r2, -90, 180);
    const int max = Animator::Focus::maxSteps();
    int step = widget ? Animator::Focus::step(widget) : max;
    if (!step && hasFocus)
        step = max;
    path.lineTo(r.x() + ((2*max - step)*r.width())/(3*max), r.y());
    QColor c;
    bool goodBad = false;
    if (widget && widget->testAttribute(Qt::WA_SetPalette)) {
        int r,g,b;
        FCOLOR(Base).getRgb(&r,&g,&b);
        if ((goodBad = (r > g+b && qAbs(g-b) < qMax(qMin(g,b)/10,1)))) // that's red
            c = FX::blend(Qt::red, LEF_COLOR, 1, 16-15*hasFocus);
        else if ((goodBad = (g > r+b && qAbs(r-b) < qMax(qMin(r,b)/10,1)))) // that's green
            c = FX::blend(Qt::green, LEF_COLOR, 1, 16-15*hasFocus);
        if (goodBad)
            painter->setPen(QPen(c, FRAME_STROKE));
    }
    if (!goodBad) {
        painter->setPen(QPen(FX::blend(FX::blend(FCOLOR(Window), FCOLOR(WindowText), 8, 1),
                             FX::blend(FCOLOR(Window), FCOLOR(Highlight), 1, 2), max - step, step), FRAME_STROKE));
    }
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPath(path);
    RESTORE_PAINTER
}

void
Style::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget, bool /*round*/) const
{
    // spinboxes and combos allready have a lineedit as global frame
    QWidget *daddy = widget ? widget->parentWidget() : 0L;
    if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame*>(option))
    if (frame->lineWidth < 1) {
        if (appType == KDM || (daddy && ( qobject_cast<QComboBox*>(daddy) || daddy->inherits("QAbstractSpinBox"))))
            return;
        painter->fillRect(RECT, FCOLOR(Base));
        return;
    }

    bool needFill = false;
    if (widget && widget->testAttribute(Qt::WA_SetPalette)) {
        needFill = appType == Plasma; // plasma doesn't expose the theme colors anywhere - we MUST add an opaque background :-(
        if (!needFill) {
            QPalette ppal = widget->parentWidget() ? widget->parentWidget()->palette() : qApp->palette();
            needFill = !FX::haveContrast(FCOLOR(Text), ppal.color(QPalette::Window));
        }
    }
    const QRect oRect = RECT;
    if (needFill) {
        OPT_FOCUS
        SAVE_PAINTER(Pen|Alias|Brush);
        painter->setPen(Qt::NoPen);
        painter->setBrush(hasFocus ? FX::blend(FCOLOR(Base), FCOLOR(Highlight), 6, 1) : FCOLOR(Base));
        painter->setRenderHint(QPainter::Antialiasing, true);
        const int rnd = (RECT.height()-1)/2;
        const int d = FRAME_STROKE_WIDTH;
        painter->drawRoundedRect(RECT.adjusted(d,d,-d,-d), rnd, rnd);
        RESTORE_PAINTER
        const_cast<QStyleOption*>(option)->rect.setX(RECT.x() + qMax(0, rnd - config.frame.roundness));
    } else {
        drawLineEditFrame(option, painter, widget);
    }
    const_cast<QStyleOption*>(option)->rect = oRect;
}

static void
drawSBArrow(QStyle::SubControl sc, QPainter *painter, QStyleOptionSpinBox *option, const QWidget *widget, const QStyle *style)
{
    if (!(option->subControls & sc))
        return;

    option->subControls = sc;
    RECT = style->subControlRect(QStyle::CC_SpinBox, option, sc, widget);
    Navi::Direction dir = Navi::N;
    QAbstractSpinBox::StepEnabledFlag sef = QAbstractSpinBox::StepUpEnabled;
    const int h = qRound(6*(RECT.height() - FRAME_STROKE_WIDTH)/16.0);
    if (sc == QStyle::SC_SpinBoxUp) {
        RECT.adjust(0, RECT.height() - h, 0, h - F(1));
    }
    else {
        dir = Navi::S; sef = QAbstractSpinBox::StepDownEnabled;
        RECT.adjust(0, F(1) - h, 0, -(RECT.height() - h));
    }

    bool isEnabled = option->stepEnabled & sef;
    bool hover = isEnabled && (option->activeSubControls == (int)sc);
//     bool sunken = hover && (option->state & QStyle::State_Sunken);

    QColor c;
    if (hover)
        c = FCOLOR(Highlight);
    else if (isEnabled)
        c = FX::blend(FCOLOR(Base), FCOLOR(Text));
    else
        c = FX::blend(FCOLOR(Base), FCOLOR(QPalette::Text),5,1);

    painter->setBrush(c);
    Style::drawArrow(dir, RECT, painter);

}

void
Style::drawSpinBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(sb, SpinBox);
    OPT_ENABLED
    SAVE_PAINTER(Pen|Brush|Alias);

    QStyleOptionSpinBox copy = *sb;

   // this doesn't work (for the moment, i assume...)
    //    isEnabled = isEnabled && !(option->state & State_ReadOnly);
    if (isEnabled)
    if (const QAbstractSpinBox *box = qobject_cast<const QAbstractSpinBox*>(widget)) {
        isEnabled = isEnabled && !box->isReadOnly();
        if (!isEnabled)
            copy.state &= ~State_Enabled;
    }

    if (sb->frame && (sb->subControls & SC_SpinBoxFrame))
        drawLineEditFrame(&copy, painter, widget);

    if (!isEnabled) {
        RESTORE_PAINTER
        return; // why bother the user with elements he can't use... ;)
    }

    painter->setPen(Qt::NoPen);
    drawSBArrow(SC_SpinBoxUp, painter, &copy, widget, this);
    copy.rect = RECT;
    copy.subControls = sb->subControls;
    drawSBArrow(SC_SpinBoxDown, painter, &copy, widget, this);
    RESTORE_PAINTER
}

static int animStep = -1;

#define DRAW_SEGMENT(_R_, _START_, _SPAN_) (hasFocus || qAbs(_SPAN_) > 5759) ? painter->drawEllipse(_R_) : painter->drawArc(_R_, _START_, _SPAN_)
void
Style::drawComboBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(cmb, ComboBox);
    OPT_ENABLED OPT_HOVER OPT_FOCUS
    SAVE_PAINTER(Pen|Brush|Alias);
    if ( widget && widget->inherits("WebView") ) {
        // paints hardcoded black text bypassing the style?! grrr...
        const_cast<QStyleOptionComboBox*>(cmb)->palette.setColor(QPalette::Window, QColor(230,230,230,255));
        widget = 0;
    }

    animStep = !widget ? MAX_STEPS*hover : Animator::Hover::step(widget);

    QRect ar;
    const QComboBox *combo = widget ? qobject_cast<const QComboBox*>(widget) : NULL;
    if ((cmb->subControls & SC_ComboBoxArrow) && (!combo || combo->count() > 0)) {   // do we have an arrow?
        ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
        const int dx = (FRAME_STROKE_WIDTH + 1) / 2;
        ar.translate(cmb->direction == Qt::LeftToRight ? -dx : dx, 0);
    }

    const bool listShown = combo && combo->view() && ((QWidget*)(combo->view()))->isVisible();
    if (listShown) { // this messes up hover
        hover = hover || QRect(widget->mapToGlobal(RECT.topLeft()), RECT.size()).contains(QCursor::pos()); // TODO Qt5, avoid asking for the cursor pos
        animStep = MAX_STEPS;
    }

    QColor c = hasFocus ? FCOLOR(Highlight) : (cmb->editable ? FCOLOR(Text) : FCOLOR(WindowText));
    if (cmb->editable) {
        if (appType == Plasma && widget && widget->testAttribute(Qt::WA_SetPalette))
            drawLineEdit(option, painter, widget);
        else
            drawLineEditFrame(option, painter, widget);
        c = FX::blend(FCOLOR(Base), c, MAX_STEPS, 1 + 2*animStep);
    } else {
        const int icon = cmb->currentIcon.isNull() ? 0 : cmb->iconSize.width() + F(4);
        QFont fnt(painter->font());
        fnt.setBold(true);
        int text = QFontMetrics(fnt).horizontalAdvance(cmb->currentText);
        if (text)
            text += F(4);

        if (!(text+icon)) // webkit etc.
            text = ar.left() - (F(16) + RECT.x());

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(hasFocus ? FCOLOR(Highlight) : FX::blend(FCOLOR(Window), c, 4, 1), FRAME_STROKE));
        painter->setBrush(Qt::NoBrush);

        const int y = ar.y() + ar.height()/2;
        const int da = animStep * 168 / MAX_STEPS;
        if (option->direction == Qt::LeftToRight) {
            DRAW_SEGMENT(ar, 16*(120 - da/2), 16*(192+da));
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->drawLine(RECT.x() + icon + text + F(6), y, ar.left()-F(2), y);
        } else {
            DRAW_SEGMENT(ar, 16*(30 + da/2), -16*(192+da));
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->drawLine(RECT.right() - (icon + text + F(6)), y, ar.right()+F(2), y);
        }
        c = FX::blend(FCOLOR(Window), FCOLOR(WindowText), MAX_STEPS - animStep, 1 + animStep);
    }

    if (!isEnabled) {
        RESTORE_PAINTER
        return;
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);

    // the arrow -------------------------------------------------------------------
    Navi::Direction dir = Navi::S;
    bool upDown = false;
    if (listShown)
        dir = (option->direction == Qt::LeftToRight) ? Navi::W : Navi::E;
    else if (combo) {
        if (combo->currentIndex() == 0)
            dir = Navi::S;
        else if (combo->currentIndex() == combo->count()-1)
            dir = Navi::N;
        else
            upDown = true;
    }

    int dx = 0;
    if (cmb->editable) {
        if (upDown || dir == Navi::N) {
            dir = Navi::S;
        }
        dx = -ar.width()/4;
        upDown = false; // shall never look like spinbox!
        hover = hover && (cmb->activeSubControls == SC_ComboBoxArrow);
    }
    const int dy = qMax(int(ceil(halfStroke)), upDown ? ar.height()/3 : 3*ar.height()/11);
    ar.adjust(0, dy, 0, -dy);
    if (upDown) {
        ar.translate(0, -F(1));
        drawArrow(Navi::N, ar, painter);
        ar.translate(0, F(2));
        drawArrow(Navi::S, ar, painter);
    } else {
        if (dir == Navi::N)
            ar.translate(dx, F(1));
        else if (dir == Navi::S)
            ar.translate(dx, cmb->editable ? -F(2) : -F(1));
        drawArrow(dir, ar, painter);
    }
    // --------------------------------------------------------------------------------
    RESTORE_PAINTER
}


void
Style::drawComboBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(cb, ComboBox);
    OPT_ENABLED

    QRect editRect;
    if (cb->editable) {
        editRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
        painter->save();
        painter->setClipRect(editRect);
    } else {
        editRect = RECT;
    }

    if (!(cb->currentIcon.isNull() || cb->iconSize.isNull()))
    {   // icon ===============================================
        QIcon::Mode mode = isEnabled ? QIcon::Normal : QIcon::Disabled;
        QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
        QRect iconRect(editRect);
        iconRect.setWidth(cb->iconSize.width() + 4);
        iconRect = alignedRect( cb->direction, Qt::AlignLeft | Qt::AlignVCenter, iconRect.size(), editRect);
        drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

        if (cb->direction == Qt::LeftToRight)
            editRect.setLeft(editRect.left() + cb->iconSize.width() + 4);
        else
            editRect.setRight(editRect.right() - (cb->iconSize.width() + 4));
    }

    if (!cb->currentText.isEmpty() && !cb->editable) {  // text =================================
        SAVE_PAINTER(Font);
        QFont fnt(painter->font());
        fnt.setBold(true);
        painter->setFont(fnt);
        if (animStep < 0) {
            OPT_HOVER
            animStep = hover ? MAX_STEPS : 0;
        } else {
            if (const QComboBox* combo = qobject_cast<const QComboBox*>(widget))
            if (combo->view() && ((QWidget*)(combo->view()))->isVisible())
                animStep = MAX_STEPS;
        }
        editRect.adjust(F(3),0, -F(3), 0);
        const int align = Qt::AlignVCenter | (cb->direction == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
        drawItemText(painter, editRect, align, PAL, isEnabled, cb->currentText, QPalette::WindowText);
        RESTORE_PAINTER
    }
    if (cb->editable)
        painter->restore();
    animStep = -1;
}
