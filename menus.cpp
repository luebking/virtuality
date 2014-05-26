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

#include <QAction>
#include <QMainWindow>
#include <QMenuBar>
#include "draw.h"
#include "animator/hoverindex.h"

#include <QtDebug>

void
Style::drawMenuBarItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(mbi, MenuItem);
    // NOTICE a)bused in XBar, b)ut shouldn't happen anyway
//     if (mbi->menuItemType == QStyleOptionMenuItem::Separator && appType != Plasma)
//         return;

    OPT_SUNKEN OPT_ENABLED
    SAVE_PAINTER(Pen|Font);

    bool hover = (option->state & State_Selected);
    Animator::IndexInfo *info = 0;
    int step = 0;
    QFont pFont = painter->font();

    if (isEnabled && sunken)
        step = 6;
    else {   // check for hover animation ==========================
        if (const QMenuBar* mbar = qobject_cast<const QMenuBar*>(widget)) {
            QAction *action = mbar->actionAt(RECT.topLeft()); // is the action for this item!
            if (action && action->font().bold())
                setBold(painter, mbi->text);
            QAction *activeAction = mbar->activeAction();
            info = const_cast<Animator::IndexInfo*>(Animator::HoverIndex::info(widget, (long int)activeAction));
            if (info && (!(activeAction && activeAction->menu()) || activeAction->menu()->isHidden()))
                step = info->step((long int)action);
        } else if (appType == Plasma && widget) {
            // i abuse this property as xbar menus are no menus and the active state is s thing of it's own...
            int action = (mbi->menuItemType & 0xffff);
            int activeAction = ((mbi->menuItemType >> 16) & 0xffff);
            info = const_cast<Animator::IndexInfo*>(Animator::HoverIndex::info(widget, activeAction));
            if (info)
                step = info->step(action);
        }
        // ================================================
    }

    if (isEnabled && (step || hover)) {
        if (!step)
            step = 6;
        painter->setPen(FX::blend(FCOLOR(WindowText), FCOLOR(Highlight), 6-step, step));
    } else 
        painter->setPen(FCOLOR(WindowText));
    QPixmap pix = mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), isEnabled ? QIcon::Normal : QIcon::Disabled);
    const uint alignment = Qt::AlignCenter | BESPIN_MNEMONIC | Qt::TextDontClip | Qt::TextSingleLine;
    if (!pix.isNull())
        drawItemPixmap(painter, RECT, alignment, pix);
    else {
        drawItemText(painter, RECT, alignment, mbi->palette, isEnabled, mbi->text);
    }
    RESTORE_PAINTER
}

static const int windowsItemFrame   = 1; // menu item frame width
static const int windowsItemHMargin = 3; // menu item hor text margin
static const int windowsItemVMargin = 1; // menu item ver text margin
static const int windowsRightBorder = 12; // right border on windows

void
Style::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(menuItem, MenuItem);
    OPT_SUNKEN OPT_ENABLED
    if (appType == GTK)
        sunken = false;

    if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {   // separator ===============================
        SAVE_PAINTER(Pen|Font|Alias);
        int dx = RECT.width()/10, y = RECT.y() + (RECT.height()-F(1))/2;
        QRect txtRct = painter->boundingRect( RECT, Qt::AlignCenter, menuItem->text);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(QPen(FX::blend(FCOLOR(Window), FCOLOR(WindowText),6,1), F(1)));
        painter->drawLine(RECT.x()+dx,y,txtRct.left()-F(6),y);
        painter->drawLine(txtRct.right()+F(6),y,RECT.right()-dx,y);
        if (!menuItem->text.isEmpty()) {
            setBold(painter, menuItem->text, RECT.width());
            painter->setPen(FCOLOR(WindowText));
            drawItemText(painter, RECT, Qt::AlignCenter, PAL, isEnabled, menuItem->text);
        }
        RESTORE_PAINTER
        return;
    }

    SAVE_PAINTER(Pen|Brush);

    QRect r = RECT;
    bool selected = isEnabled && menuItem->state & State_Selected;
    if (selected && config.menu.indent) {
        SAVE_PAINTER(Alias);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(QPen(FCOLOR(Highlight), F(2)));
        int x = r.x() + F(1);
        painter->drawLine(x, r.y() + F(2), x, r.bottom() - F(2));
        r.setX(r.x() + F(2));
        RESTORE_PAINTER
    }

    const bool checkable = (menuItem->checkType != QStyleOptionMenuItem::NotCheckable);
    const bool checked = checkable && menuItem->checked;
    const bool subMenu = (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu);

    // Text and icon, ripped from windows style
    const QStyleOptionMenuItem *menuitem = menuItem;
    int iconCol = config.menu.showIcons*menuitem->maxIconWidth;

    if (isEnabled && config.menu.showIcons && !menuItem->icon.isNull()) {
        QRect vCheckRect = visualRect(option->direction, r, QRect(r.x(), r.y(), iconCol, r.height()));
        const QPixmap &pixmap = menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal, checked ? QIcon::On : QIcon::Off);

        QRect pmr(QPoint(0, 0), pixmap.size());
        pmr.moveCenter(vCheckRect.center());

        painter->drawPixmap(pmr.topLeft(), pixmap);
    }

    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    const int tab = menuitem->tabWidth;
    const int cDim = (2*(r.height()+2)/3);
    const int xm = windowsItemFrame + iconCol + windowsItemHMargin;
    int xpos = r.x() + xm;

    if (checkable) {   // Checkmark =============================
        if (isEnabled) {
            QStyleOptionMenuItem tmpOpt = *menuItem;
            tmpOpt.rect = QRect(xpos, r.y() + (r.height() - cDim)/2, cDim, cDim);
            tmpOpt.rect = visualRect(menuItem->direction, menuItem->rect, tmpOpt.rect);
            tmpOpt.state &= ~State_Selected; // cause of color, not about checkmark!
            tmpOpt.state &= ~State_MouseOver; // no extra hover, confusing and doesn't work correctly
            tmpOpt.state |= QStyle::State_Item; // tell the button painting code to not try an animation
            if (checked) {
                tmpOpt.state |= State_On;
                tmpOpt.state &= ~State_Off;
            } else {
                tmpOpt.state |= State_Off;
                tmpOpt.state &= ~State_On;
            }
            drawRadioOrCheckBox(&tmpOpt, painter, widget, menuItem->checkType & QStyleOptionMenuItem::Exclusive);
        }
        xpos += cDim + F(4);
    }

//     painter->setPen(sunken ? FCOLOR(Highlight) : (selected ? FCOLOR(WindowText) : FX::blend(FCOLOR(Window), FCOLOR(WindowText))));
    QColor c;
    if (sunken)
        c = FCOLOR(Highlight);
    else if (selected)
        c = FCOLOR(WindowText);
    else if (config.menu.indent)
        c = FX::blend(FCOLOR(Window), FCOLOR(WindowText), 1, 3);
    else
        c = FX::blend(FCOLOR(Window), FCOLOR(WindowText));

    painter->setPen(c);

    QRect textRect(xpos, y + windowsItemVMargin,
                   w - (xm + checkable*(cDim+F(4)) + subMenu*windowsRightBorder + tab + windowsItemFrame + windowsItemHMargin),
                   h - 2 * windowsItemVMargin);
    if (subMenu) { // draw sub menu arrow ================================
        int dim = (5*r.height()/12) | 1;
        xpos = r.right() - F(4) - dim;
        textRect.setRight(xpos/* - F(4)*/);
        /*if (isEnabled)*/ {
            Navi::Direction dir = (option->direction == Qt::RightToLeft) ? Navi::W : Navi::E;
            QStyleOptionMenuItem tmpOpt = *menuItem;
            tmpOpt.rect = visualRect(option->direction, r, QRect(xpos, r.y() + (r.height() - dim)/2, dim, dim));
            const QColor c = painter->pen().color();
            painter->setBrush(FX::blend(FCOLOR(Window), c, 4, 2+3*isEnabled));
            painter->setPen(Qt::NoPen);
            drawArrow(dir, tmpOpt.rect, painter);
            painter->setPen(c);
        }
    }

    painter->setBrush(Qt::NoBrush);
    QRect vTextRect = visualRect(option->direction, r, textRect);

    QString s = menuitem->text;
    if (!s.isEmpty()) {   // draw text
        int t = s.indexOf('\t');
        const int text_flags = Qt::AlignVCenter | BESPIN_MNEMONIC | Qt::TextDontClip | Qt::TextSingleLine;
        if (t >= 0) {
            QRect vShortcutRect = visualRect(option->direction, r, QRect(textRect.topRight(),
                                             QPoint(textRect.right()+tab, textRect.bottom())));
            const QColor fg = painter->pen().color();
            painter->setPen(FX::blend(FCOLOR(Window), fg));
            drawItemText(painter, vShortcutRect, text_flags | Qt::AlignRight, PAL, isEnabled, s.mid(t + 1));
            painter->setPen(fg);
            s = s.left(t);
        }
        const int align = /*subMenu ? Qt::AlignHCenter : */Qt::AlignLeft;
        if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem) {
            QFont fnt = painter->font();
            setBold(painter, s, vTextRect.width());
            drawItemText(painter, vTextRect, text_flags | align, PAL, isEnabled, s);
            painter->setFont(fnt);
        }
        else
            drawItemText(painter, vTextRect, text_flags | align, PAL, isEnabled, s);
    }

    RESTORE_PAINTER;
}

void
Style::drawMenuScroller(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    OPT_SUNKEN OPT_HOVER
    SAVE_PAINTER(Brush);
    Navi::Direction dir = (option->state & State_DownArrow) ? Navi::S : Navi::N;
    painter->setBrush((hover & !sunken) ? FCOLOR(Highlight) : FCOLOR(WindowText));
    drawArrow(dir, RECT, painter);
    RESTORE_PAINTER
}

//    case CE_MenuTearoff: // A menu item representing the tear off section of a QMenu

