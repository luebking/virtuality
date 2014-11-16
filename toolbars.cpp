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

#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>
#include <QVarLengthArray>
#include "draw.h"
#include "animator/hover.h"
#include "FX.h"

#include <QtDebug>

#define MAX_STEPS Animator::Hover::maxSteps()
static int step = 0;

bool
Style::hasMenuIndicator(const QStyleOptionToolButton *tb)
{
    // subcontrol requested?
    bool ret = (tb->subControls & SC_ToolButtonMenu) || (tb->features & QStyleOptionToolButton::Menu);

    // delayed menu?
    if (!ret)
        ret = (tb->features & (QStyleOptionToolButton::HasMenu | QStyleOptionToolButton::PopupDelay))
                           == (QStyleOptionToolButton::HasMenu | QStyleOptionToolButton::PopupDelay);
    return ret;
}

void
Style::drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(toolbutton, ToolButton);
    OPT_ENABLED OPT_HOVER
    SAVE_PAINTER(Pen|Brush);
    QStyleOption tool(*toolbutton);
    QWidget *daddy = widget ? widget->parentWidget() : 0L;
    const QPalette &pal = daddy ? daddy->palette() : PAL;

    // special handling for the tabbar scrollers ------------------------------
    if (toolbutton->features & QStyleOptionToolButton::Arrow && qobject_cast<QTabBar*>(daddy)) {
        QColor bg = FCOLOR(WindowText), fg = FCOLOR(Window);
        painter->setBrush(!isEnabled ? FX::blend(bg, fg, 8,1) : (hover ? FCOLOR(Highlight) : fg));
        painter->setPen(Qt::NoPen);
        drawArrow(Navi::Direction(toolbutton->arrowType), RECT.adjusted(F(4), F(4), -F(4), -F(4)), painter);
        RESTORE_PAINTER
        return;
    } // --------------------------------------------------------------------

    QRect button = subControlRect(CC_ToolButton, toolbutton, SC_ToolButton, widget);
    State bflags = toolbutton->state;

    if ((bflags & State_AutoRaise) && !hover)
        bflags &= ~State_Raised;

    if (toolbutton->activeSubControls & SC_ToolButton)
        bflags |= State_Sunken;

    hover = isEnabled && (bflags & (State_Sunken | State_On | State_Raised | State_HasFocus));

    step = Animator::Hover::step(widget);

    if (hasMenuIndicator(toolbutton)) {
        QRect menuarea = subControlRect(CC_ToolButton, toolbutton, SC_ToolButtonMenu, widget);
        painter->setPen(FX::blend(pal.color(QPalette::Window), pal.color(QPalette::WindowText), 12-step, step));
        drawSolidArrow(Navi::S, menuarea, painter);
    }

   // label in the toolbutton area
   QStyleOptionToolButton label = *toolbutton;
   label.rect = button;
   label.palette = pal;
   drawToolButtonLabel(&label, painter, widget);
   step = 0;
   RESTORE_PAINTER
}

static QPixmap &
icon(QPixmap &pix, int step)
{
#if 1
    static QVarLengthArray<float> table;
    const int n = MAX_STEPS + 1;
    if (table.size() != n) {
        table.resize(n);
        const float accel = 0.5f;
        const float factor = pow(0.5f, accel);
        for (int i = (n+1)/2; i < n; ++i) {
            float ratio = i/float(MAX_STEPS);
            if (ratio == 0.5f)
                table[i] = 0.f;
            else
                table[i] = (pow(ratio - 0.5f, accel) + factor) / (2.0f*factor);
        }
        for (int i = 0; i <= n/2; ++i) {
            table[i] = 1.0f - table[MAX_STEPS-i];
        }
    }

    const float ratio = table.at(step);
#else
    const float ratio = step/float(MAX_STEPS);
#endif
    static QPixmap scaledIcon[2], emptyIcon;
    static qint64 lastIconPix[2] = {0, 0};
    int idx = pix.cacheKey() == lastIconPix[0] ? 0 : (pix.cacheKey() == lastIconPix[1] ? 1 : -1);
    if (idx < 0) {
        idx = 1;
        if (lastIconPix[1]) {
            scaledIcon[0] = scaledIcon[1];
            lastIconPix[0] = lastIconPix[1];
        }
        scaledIcon[1] = pix.scaledToHeight(pix.height() + F(4), Qt::SmoothTransformation);
        if (emptyIcon.size() != scaledIcon[1].size()) {
            emptyIcon = QPixmap(scaledIcon[1].size());
        }
        lastIconPix[1] = pix.cacheKey();
    }

    if (step == MAX_STEPS)
        return scaledIcon[idx];

    emptyIcon.fill(Qt::transparent);

    FX::blend(pix, emptyIcon, 1.0, F(2), F(2));
    FX::blend(scaledIcon[idx], emptyIcon, ratio);
    return emptyIcon;
}

void
Style::drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(toolbutton, ToolButton);
    OPT_ENABLED OPT_SUNKEN
    SAVE_PAINTER(Pen|Brush|Alias|Font);

    // Arrow type always overrules and is always shown
    const bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
    const bool justText = toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly ||
                          (!hasArrow && toolbutton->icon.isNull() && !toolbutton->text.isEmpty() );

    QPalette::ColorRole role = QPalette::WindowText;
    QPalette::ColorRole bgRole = QPalette::Window;
    if (widget) {
        const QWidget *dad = widget->parentWidget();

        const QWidget *w = dad ? dad : widget;
        bgRole = w->backgroundRole();
        role = w->foregroundRole();

        if (role == QPalette::ButtonText && dad && dad->inherits("QMenu")) {
            role = QPalette::WindowText; // this is a f**** KMenu Header
            step = 0;
        }
    }

    QColor text = PAL.color(role);

    if (hasArrow) {
        painter->setPen(text);
        const int f5 = F(5);
        drawSolidArrow(Navi::Direction(toolbutton->arrowType), RECT.adjusted(f5,f5,-f5,-f5), painter);
    }

    int drawIndicator = 0;
    if (option->state & State_On)
        drawIndicator = 2;
    else if (const QToolButton *btn = qobject_cast<const QToolButton*>(widget)) {
        if (btn->isCheckable())
            drawIndicator = btn->isChecked() ? 2 : 1;
    }

    if (drawIndicator) {
        QRect r(RECT.right()-F(6),RECT.y()+F(1),F(5),F(5));
        if (drawIndicator > 1) {
            painter->setPen(QPen(COLOR(role), 0.5));
            painter->setBrush(FCOLOR(Highlight));
        } else {
            painter->setPen(QPen(FX::blend(COLOR(bgRole), COLOR(role), 2, 1), F(1)));
            painter->setBrush(Qt::NoBrush);
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawEllipse(r);
    }

    if (justText) {   // the most simple way
        text = FX::blend(text, FCOLOR(Link), MAX_STEPS-step, step);
        if (sunken) {
            QFont fnt(painter->font());
            if (fnt.pointSizeF() > 0.0) {
                fnt.setPointSizeF(0.85*fnt.pointSizeF());
                painter->setFont(fnt);
            }
        }
        painter->setPen(text);
        drawItemText(painter, RECT, Qt::AlignCenter | BESPIN_MNEMONIC, PAL, isEnabled, toolbutton->text);
        RESTORE_PAINTER
        return;
    }

    QPixmap pm;
    QSize pmSize = RECT.size() - QSize(F(4), F(4));
    pmSize = pmSize.boundedTo(toolbutton->iconSize);
    pmSize.setWidth(qMin(pmSize.width(), pmSize.height()));
    pmSize.setHeight(pmSize.width());

    if (!toolbutton->icon.isNull()) {
        const int style = config.btn.tool.disabledStyle;
//         const QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
        pm = toolbutton->icon.pixmap(RECT.size().boundedTo(pmSize), isEnabled || style ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
#if 0   // this is -in a way- the way it should be done..., but KIconLoader gives a shit on this or anything else
        if (!isEnabled)
            pm = generatedIconPixmap(QIcon::Disabled, pm, toolbutton);
#else
        if (!isEnabled && style) {
            QImage img(pm.width() + F(4), pm.height() + F(4), QImage::Format_ARGB32);
            img.fill(Qt::transparent);
            QPainter p(&img);
            if (style > 1) { // blurring
                p.setOpacity(0.5);
                p.drawImage(F(3),F(3), pm.toImage().scaled(pm.size() - QSize(F(2),F(2)), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                p.end();
                FX::expblur(img, F(3));
            }
            else { // desaturation (like def. Qt but with a little transparency)
                p.setOpacity(0.7);
                p.drawImage(F(2), F(2), pm.toImage());
                p.end();
                FX::desaturate(img, COLOR(bgRole));
            }
            pm = QPixmap::fromImage(img);
        }
#endif
        else if (step && !(sunken || pm.isNull()))
            pm = icon(pm, step);
        pmSize = pm.size();
    }

    if (!(toolbutton->text.isEmpty() || toolbutton->toolButtonStyle == Qt::ToolButtonIconOnly)) {
        if (pm.isNull())
            text = FX::blend(text, FCOLOR(Link), MAX_STEPS-step, step);
        painter->setPen(text);

        painter->setFont(toolbutton->font);

        QRect pr = RECT, tr = RECT;
        int alignment = BESPIN_MNEMONIC;

        if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
            int fh = painter->fontMetrics().height();
            pr.adjust(0, 0, 0, -fh - F(2));
            tr.adjust(0, pr.bottom(), 0, -F(3));
            if (!hasArrow)
                drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
            else
                drawSolidArrow(Navi::S, pr, painter);
            alignment |= Qt::AlignCenter;
        } else {
            pr.setWidth(toolbutton->iconSize.width() + F(4));

            if (!hasArrow)
                drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
            else
                drawSolidArrow(Navi::S, pr, painter);

            tr.adjust(pr.width() + F(4), 0, 0, 0);
            alignment |= Qt::AlignLeft | Qt::AlignVCenter;
        }
        drawItemText(painter, tr, alignment, PAL, isEnabled, toolbutton->text);
        RESTORE_PAINTER
        return;
    }

    RESTORE_PAINTER
    if (!hasArrow) {
        drawItemPixmap(painter, RECT, Qt::AlignCenter, pm);
    }
}

