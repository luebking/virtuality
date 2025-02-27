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

#include <QCache>
#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>
#include "draw.h"
#include "animator/hover.h"
#include "FX.h"

#include <QtDebug>

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
    bool isTooBar = true;
    if (widget) {
        const QWidget *dad = widget->parentWidget();
        if (dad && !qobject_cast<const QToolBar*>(dad))
            isTooBar = false;
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
        if (widget && widget->inherits("Fm::PathButton")) {
            QColor c = drawIndicator > 1 ? FCOLOR(Highlight) : FX::blend(COLOR(bgRole), COLOR(role), 2, 1);
            painter->setPen(QPen(c, 0.5));
//            painter->setPen(Qt::NoPen);
            painter->setBrush(c);
            QRect r = RECT;
            Navi::Direction dir = Navi::E;
            if (option->direction == Qt::RightToLeft) {
                r.setRight(r.left() + F(8));
                dir = Navi::W;
            } else {
                r.setLeft(r.right() - F(8));
            }
            drawSolidArrow(dir, r, painter);
        } else {
            if (drawIndicator > 1) {
                painter->setPen(QPen(COLOR(role), 0.5));
                painter->setBrush(FCOLOR(Highlight));
            } else {
                painter->setPen(QPen(FX::blend(COLOR(bgRole), COLOR(role), 2, 1), F(1)));
                painter->setBrush(Qt::NoBrush);
            }
            QRect r(RECT.right()-F(6),RECT.y()+F(1),F(5),F(5));
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawEllipse(r);
        }
    }

    if (justText) {   // the most simple way
        text = FX::blend(text, FCOLOR(Link), Animator::Hover::maxSteps()-step, step);
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
        if (isTooBar && true) {
            static QCache<qint64, QPixmap> iconCache(64);
            QPixmap *pix = iconCache[pm.cacheKey()];
            if (!pix) {
                pix = new QPixmap(FX::tintedIcon(pm, 1, 1, text));
                iconCache.insert(pm.cacheKey(), pix);
            }
            pm = *pix;
        }
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
            pm = FX::tintedIcon(pm, step, Animator::Hover::maxSteps(), FCOLOR(Highlight));
//             pm = FX::scaledIcon(pm, step, Animator::Hover::maxSteps(), F(2));
        pmSize = pm.size();
    }

    if (!(toolbutton->text.isEmpty() || toolbutton->toolButtonStyle == Qt::ToolButtonIconOnly)) {
        if (pm.isNull())
            text = FX::blend(text, FCOLOR(Link), Animator::Hover::maxSteps()-step, step);
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

