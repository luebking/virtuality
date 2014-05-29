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

#include <QAbstractButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionMenuItem>
#include <QTabBar>
#include <QToolBar>
#include "virtuality.h"
#include "makros.h"

#include <QtDebug>

using namespace BE;

static const int windowsArrowHMargin = 6; // arrow horizontal margin

inline static bool
verticalTabs(QTabBar::Shape shape)
{
    return  shape == QTabBar::RoundedEast ||
            shape == QTabBar::TriangularEast ||
            shape == QTabBar::RoundedWest ||
            shape == QTabBar::TriangularWest;
}

QSize
Style::sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    switch ( ct )
    {
    case CT_CheckBox: // A check box, like QCheckBox
    case CT_RadioButton: // A radio button, like QRadioButton
        if HAVE_OPTION(btn, Button)
        {
            const int indicator = pixelMetric(PM_IndicatorHeight, btn, widget);
            int w, h;
            w = h = qMax( indicator, indicator );
            h = qMax(h, contentsSize.height() + 4);

            int margin = 0;
            // we add 4 pixels for label margins
            if (btn->icon.isNull() || !btn->text.isEmpty())
                margin = F(7);
            return QSize(contentsSize.width() + w + margin, h);
        }
    case CT_ComboBox: // A combo box, like QComboBox
        if HAVE_OPTION(cb, ComboBox)
        {
            if (cb->editable)
                return contentsSize + QSize(F(12) + (cb->fontMetrics.ascent() + F(2))*1.1, qMax(F(4) - config.fontExtent,0));

            int hgt = contentsSize.height();
            int d = F(8);
            if (cb->frame) {
                hgt = qMax(config.btn.minHeight, hgt + F(4) - config.fontExtent);
                d += F(16);
            } else
                hgt = qMax(config.btn.minHeight, hgt);
//             if ( !cb->currentIcon.isNull()) // leads to inequal heights + pot. height changes on item change
//                 hgt += F(2);
            return QSize(contentsSize.width() + d + hgt, hgt);
        }
//    case CT_DialogButtons: //
//       return QSize((contentsSize.width()+16 < 80) ? 80 : contentsSize.width()+16, contentsSize.height()+10);
//    case CT_Q3DockWindow: //
    case CT_HeaderSection: // A header section, like QHeader
        if HAVE_OPTION(hdr, Header)
        {
            QSize sz;
            int margin = F(2);
            int iconSize = hdr->icon.isNull() ? 0 : pixelMetric(PM_SmallIconSize, hdr, widget);
            QSize txt = hdr->fontMetrics.size(0, hdr->text);
            sz.setHeight(qMax(iconSize, txt.height()) + F(5) - config.fontExtent);
            sz.setWidth((iconSize?margin+iconSize:0) + (hdr->text.isNull()?0:margin+txt.width()) + margin);
            return sz;
        }
    case CT_LineEdit: // A line edit, like QLineEdit
        return contentsSize + QSize(F(4), qMax(F(4) - config.fontExtent,0));
    case CT_MenuBarItem:
    {   // A menu bar item, like the buttons in a QMenuBar
        const int h = contentsSize.height() + F(8) - config.fontExtent;
        return QSize(qMax(contentsSize.width()+F(12), h*8/5), h);
    }
   case CT_MenuItem: // A menu item, like QMenuItem
        if HAVE_OPTION(menuItem, MenuItem)
        {
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator && menuItem->text.isEmpty())
                return QSize(10, F(6));

            bool checkable = menuItem->menuHasCheckableItems;
            int maxpmw = config.menu.showIcons*menuItem->maxIconWidth;
            int w = contentsSize.width();
            int h = qMax(contentsSize.height(), menuItem->fontMetrics.lineSpacing()) + F(4) - config.fontExtent;

            if (config.menu.showIcons && !menuItem->icon.isNull())
                h = qMax(h, menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height() + F(4) - config.fontExtent);
            if (menuItem->text.contains('\t'))
                w += F(12);
            if (maxpmw > 0)
                w += maxpmw + F(6);
            if (checkable)
                w += 2*(h - F(4))/3 + F(7);
            w += (checkable + (maxpmw > 0))*F(2);
            w += config.menu.indent ? F(20) : F(12);
            if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 2 * windowsArrowHMargin;
            if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem)
            {   // adjust the font and add the difference in size.
                // it would be better if the font could be adjusted in the
                // getStyleOptions qmenu func!!
                QFontMetrics fm(menuItem->font);
                QFont fontBold = menuItem->font;
                fontBold.setBold(true);
                QFontMetrics fmBold(fontBold);
                w += fmBold.width(menuItem->text) - fm.width(menuItem->text);
            }
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator)
                w += 32; // add some space for the separator lines
            return QSize(w, h);
        }
        break;
    case CT_PushButton: // A push button, like QPushButton
        if HAVE_OPTION(btn, Button)
        {
            int w = contentsSize.width();
            if (btn->features & QStyleOptionButton::HasMenu)
                w += contentsSize.height()+F(16);
            else
                if (widget)
                if (const QAbstractButton* abn = qobject_cast<const QAbstractButton*>(widget))
                if (abn->isCheckable())
                    w += contentsSize.height() + F(4);

            int h = contentsSize.height() - config.fontExtent;
            h += F(4);

            if (btn->text.isEmpty()) {
                w += 8;
            } else {
                w += F(20);
                if (!btn->icon.isNull())
                    w += F(4) + btn->iconSize.width(); // we want symmetry and need 2px padding (+the blind icon and it's blind padding)
                if (w < F(80))
                    w = F(80);
            }

            return QSize(w, qMax(config.btn.minHeight, h));
        }
//    case CT_SizeGrip: // A size grip, like QSizeGrip

    case CT_Menu: // A menu, like QMenu
    case CT_MenuBar: // A menu bar, like QMenuBar
    case CT_ProgressBar: // A progress bar, like QProgressBar
    case CT_Slider: // A slider, like QSlider
    case CT_ScrollBar: // A scroll bar, like QScrollBar
        return contentsSize;
    case CT_SpinBox: // A spin box, like QSpinBox
        return contentsSize + QSize(contentsSize.height()/2  + F(3), 0);
//    case CT_Splitter: // A splitter, like QSplitter
    case CT_TabBarTab: { // A tab on a tab bar, like QTabBar
        if HAVE_OPTION(tab, Tab) {
            if (!verticalTabs(tab->shape)) {
                QFont fnt(widget?widget->font():QFont());
                if (fnt.pointSize() > 0) {
                    QSize osz(QFontMetrics(fnt).boundingRect(tab->text).size());
                    fnt.setPointSize(16*fnt.pointSize()/10);
//                     fnt.setBold(true);
                    QSize sz(QFontMetrics(fnt).boundingRect(tab->text).size());
                    sz.setWidth((sz.width() - osz.width() + 1) / 2 + contentsSize.width());
                    sz.setHeight(qMax(sz.height(), contentsSize.height()));
                    return sz + QSize(F(2), F(2));
                }
            }
//             if ( appType == Dolphin && widget )
//             if ( /*const QTabBar *bar =*/ qobject_cast<const QTabBar*>(widget) )
//                 other = qMax( 0, 16+F(8)-contentsSize.height() ); // compensate the close buttons
        }
        return contentsSize + QSize(F(2), F(2));
    }
    case CT_TabWidget: // A tab widget, like QTabWidget
        return contentsSize; // + QSize(F(8),F(6)); WARNING! this can causes recursive updates! (Qt 4.7 bug?)
    case CT_ToolButton:
    {   // A tool button, like QToolButton
        const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(option);
        // get ~goldem mean ratio
        int w, h;
        if (toolbutton && toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon)
            h = contentsSize.height() + F(8);
        else
            h = contentsSize.height() + F(4);

        w = qMax(contentsSize.width() + F(8), h*4/3); // 4/3 - 16/9
        if (toolbutton && hasMenuIndicator(toolbutton))
            w += pixelMetric(PM_MenuButtonIndicator, option, widget)/* + F(4)*/;
        return QSize(w, h);
    }
    default: ;
    } // switch
    return QCommonStyle::sizeFromContents( ct, option, contentsSize, widget );
}
