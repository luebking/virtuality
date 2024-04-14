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

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QTabBar>
#include <QTabWidget>
#include <QStyleOption>
#include <QStyleOptionTab>
#include <limits.h>
#include "virtuality.h"
#include "draw.h"

using namespace BE;

inline static bool
verticalTabs(QTabBar::Shape shape)
{
    return  shape == QTabBar::RoundedEast || shape == QTabBar::TriangularEast ||
            shape == QTabBar::RoundedWest || shape == QTabBar::TriangularWest;
}

QRect
Style::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                      SubControl subControl, const QWidget *widget) const
{
    QRect ret;
    switch (control) {
    case CC_SpinBox: {   // A spinbox, like QSpinBox
        ASSURE_OPTION(spinbox, SpinBox) ret;

        int h = spinbox->rect.height()/2;
        int w = 2*h;
        int x = spinbox->rect.width() - w;
        w -= FRAME_STROKE_WIDTH;

        switch (subControl) {
        case SC_SpinBoxUp:
            ret = QRect(x, spinbox->rect.y(), w, h);
            break;
        case SC_SpinBoxDown:
            ret = QRect(x, spinbox->rect.y() + h + 1, w, h);
            break;
        case SC_SpinBoxEditField:
            w = h = 0; // becomes framesizes
            if (spinbox->frame)
                w = h = FRAME_STROKE_WIDTH;
            ret = QRect(w, h, x, spinbox->rect.height() - 2*h);
            break;
        case SC_SpinBoxFrame:
            ret = spinbox->rect;
        default:
            break;
        }
        ret = visualRect(config.leftHanded, spinbox->rect, ret);
        break;
    }
    case CC_ComboBox:
    {   // A combobox, like QComboBox
        ASSURE_OPTION(cb, ComboBox) ret;
        int x,y,wi,he;
        cb->rect.getRect(&x,&y,&wi,&he);
        const int fh = (cb->fontMetrics.ascent() + FRAME_STROKE_WIDTH) | 1;

        switch (subControl)
        {
        case SC_ComboBoxFrame:
            ret.setRect(x, y, wi, he);
            break;
         case SC_ComboBoxArrow:
            x += wi; wi = fh;
            x -= wi;
            y += (he - fh + 1)/2;
            ret.setRect(x, y, wi, fh);
            break;
        case SC_ComboBoxEditField:
            wi -= fh;
            ret.setRect(x, y, wi, he);
            break;
        case SC_ComboBoxListBoxPopup:
            if (const QComboBox *box = qobject_cast<const QComboBox*>(widget))
            if (box->count() <= box->maxVisibleItems()) {
                // shorten for the arrow
                wi -= 10*fh/11;
            }
            ret.setRect(x, y, wi, he);
            break;
        default:
            break;
        }
        ret = visualRect(option->direction, cb->rect, ret);
      break;
    }
    case CC_GroupBox:
    {
        ASSURE_OPTION(groupBox, GroupBox) ret;

        switch (subControl)
        {
        case SC_GroupBoxFrame:
            ret = groupBox->rect;
            break;
        case SC_GroupBoxContents:
        {
            int top = 0;
            if (groupBox->subControls & SC_GroupBoxCheckBox)
                top = pixelMetric(PM_IndicatorHeight, groupBox, widget);
            if (!groupBox->text.isEmpty())
                top = qMax(top, groupBox->fontMetrics.height());
            top += (groupBox->features & QStyleOptionFrame::Flat) ? F(3) : F(6);
            ret = groupBox->rect.adjusted(F(3), top, -F(3), -F(5));
            break;
        }
        case SC_GroupBoxCheckBox: {
            const int indicator = pixelMetric(PM_IndicatorHeight, groupBox, widget);
            ret.setWidth(indicator);
            ret.setHeight(indicator);
            break;
        }
        case SC_GroupBoxLabel: {
            const int h = groupBox->fontMetrics.height() + F(3);
            const int tw = groupBox->fontMetrics.size(BESPIN_MNEMONIC, groupBox->text.toUpper() + QLatin1Char(' ')).width();

            int dx = F(2); // frame
            if (groupBox->subControls & SC_GroupBoxCheckBox)
                dx += pixelMetric(PM_IndicatorHeight, groupBox, widget) + F(3);
            ret = groupBox->rect;
            ret.setLeft(ret.left() + dx);
            ret.setHeight(h);

            // Adjusted rect for label + indicatorWidth + indicatorSpace
            ret = alignedRect(groupBox->direction, Qt::AlignLeft | Qt::AlignVCenter, QSize(tw, h), ret);
            break;
        }
        default:
            break;
        }
        break;
    }

    case CC_ScrollBar: // A scroll bar, like QScrollBar
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            int sbextent = pixelMetric(PM_ScrollBarExtent, scrollbar, widget);
            int buttonSpace = config.slider.buttons * sbextent * 2;
            bool needSlider = false;

            switch (subControl)
            {
            case SC_ScrollBarGroove:
            {
                ret = RECT;
                int off = 0, d = 0;
                if (scrollbar->orientation == Qt::Horizontal) {
                    ret.adjust(off, d, -(buttonSpace + off), -d);
                }
                else {
                    ret.adjust(d, off, -d, -(buttonSpace + off));
                }
                break;
            }
            // top/left button
            case SC_ScrollBarSubLine:
            // bottom/right button
            case SC_ScrollBarAddLine:
            {
                const int f = 1 + (subControl == SC_ScrollBarSubLine);
                if (!config.slider.buttons)
                    ret = QRect();
                else if (scrollbar->orientation == Qt::Horizontal)
                {
                    const int buttonWidth = qMin(RECT.width() / 2, sbextent);
                    ret.setRect(RECT.right() + 1 - f*buttonWidth, RECT.y(), buttonWidth, sbextent);
                }
                else
                {
                    const int buttonHeight = qMin(scrollbar->rect.height() / 2, sbextent);
                    ret.setRect(RECT.x(), RECT.bottom() + 1 - f*buttonHeight, sbextent, buttonHeight);
                }
                break;
            }
            default:
                needSlider = true; break;
            }
            if (needSlider)
            {
                const uint range = scrollbar->maximum - scrollbar->minimum;
                const int maxlen = ((scrollbar->orientation == Qt::Horizontal) ? RECT.width() : RECT.height()) - buttonSpace;
                int sliderlen = maxlen;
                // calculate slider length
                if (scrollbar->maximum != scrollbar->minimum)
                {
                    sliderlen = (maxlen * scrollbar->pageStep) / (range + scrollbar->pageStep);
                    if (sliderlen > maxlen)
                        sliderlen = maxlen;
                    else
                    {
                        const int slidermin = pixelMetric(PM_ScrollBarSliderMin, option, widget);
                        if (sliderlen < slidermin || range > INT_MAX / 2)
                            sliderlen = slidermin;
                    }
                }

                const int sliderstart = sliderPositionFromValue(scrollbar->minimum, scrollbar->maximum,
                                                                scrollbar->sliderPosition, maxlen - sliderlen,
                                                                scrollbar->upsideDown);
                switch (subControl)
                {
                // between top/left button and slider
                case SC_ScrollBarSubPage:
                    if (scrollbar->orientation == Qt::Horizontal)
                        ret.setRect(RECT.x() + F(2), RECT.y(), sliderstart, sbextent);
                    else
                        ret.setRect(RECT.x(), RECT.y() + F(2), sbextent, sliderstart);
                    break;
                // between bottom/right button and slider
                case SC_ScrollBarAddPage:
                    if (scrollbar->orientation == Qt::Horizontal)
                        ret.setRect(RECT.x() + sliderstart + sliderlen - F(2), RECT.y(),
                                    maxlen - (sliderstart + sliderlen), sbextent);
                    else
                        ret.setRect(RECT.x(), RECT.y() + sliderstart + sliderlen - F(3),
                                    sbextent, maxlen - (sliderstart + sliderlen));
                    break;
                case SC_ScrollBarSlider:
                    if (scrollbar->orientation == Qt::Horizontal)
                        ret.setRect(RECT.x() + sliderstart, RECT.y(), sliderlen, sbextent);
                    else
                        ret.setRect(RECT.x(), RECT.y() + sliderstart, sbextent, sliderlen + F(1) );
                    break;
                default:
                    break;
                }
            }

            ret = visualRect(scrollbar->direction, RECT, ret);
        }
        break;

    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            const bool horizontal = slider->orientation == Qt::Horizontal;
            const int thickness = pixelMetric(PM_SliderControlThickness, slider, widget);
            switch (subControl) {
            case SC_SliderHandle: {
                ret = QRect(0,0,thickness,thickness);
                ret.moveCenter(RECT.center());
                int sliderPos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                        (horizontal ? RECT.width() : RECT.height()) - thickness, slider->upsideDown);
                if (horizontal)
                    ret.moveLeft(RECT.x() + sliderPos);
                else
                    ret.moveTop(RECT.y() + sliderPos);
                break;
            }
            case SC_SliderTickmarks:
            case SC_SliderGroove: {
                if (horizontal)
                    ret.setRect(RECT.x(), RECT.y(), RECT.width(), thickness );
                else
                    ret.setRect(RECT.x(), RECT.y(), thickness, RECT.height());
                ret.moveCenter(RECT.center());
                break;
            }
            default:
                break;
            ret = visualRect(slider->direction, RECT, ret);
            }
        }
        break;

    case CC_ToolButton: // A tool button, like QToolButton
        if HAVE_OPTION(tb, ToolButton) {
            ret = tb->rect;
            if (hasMenuIndicator(tb)) {   // has an arrow
                int x = ret.right() - pixelMetric(PM_MenuButtonIndicator, tb, widget);
                if (subControl == SC_ToolButton)
                    ret.setRight(x);
                else if (subControl == SC_ToolButtonMenu) {
                    ret.setLeft(x);
                }
            }
            return visualRect(tb->direction, tb->rect, ret);
        }
        break;
   case CC_TitleBar: // A Title bar, like what is used in Q3Workspace
      if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
      {
         const int controlMargin = F(3);
         const int controlHeight = tb->rect.height() - controlMargin*2;
         const int delta = controlHeight + 2*controlMargin;
         int offset = 0;

         bool isMinimized = tb->titleBarState & Qt::WindowMinimized;
         bool isMaximized = tb->titleBarState & Qt::WindowMaximized;

         SubControl sc = subControl;
         if (sc == SC_TitleBarNormalButton) { // check what it's good for
            if (isMinimized)
               sc = SC_TitleBarMinButton; // unminimize
            else if (isMaximized)
               sc = SC_TitleBarMaxButton; // unmaximize
            else
               break;
         }

         switch (sc) {
         case SC_TitleBarLabel:
            if (tb->titleBarFlags &
                (Qt::WindowTitleHint | Qt::WindowSystemMenuHint)) {
               ret = tb->rect;
//                ret.adjust(delta, 0, 0, 0);
               if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                  ret.adjust(delta, 0, -delta, 0);
               if (tb->titleBarFlags & Qt::WindowMinimizeButtonHint)
                  ret.adjust(delta, 0, 0, 0);
               if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
                  ret.adjust(delta, 0, 0, 0);
               if (tb->titleBarFlags & Qt::WindowShadeButtonHint)
                  ret.adjust(0, 0, -delta, 0);
               if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                  ret.adjust(0, 0, -delta, 0);
            }
            break;
         case SC_TitleBarMaxButton:
            if (isMaximized && subControl == SC_TitleBarMaxButton)
               break;
            if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
               offset += delta;
            else if (sc == SC_TitleBarMaxButton) // true...
               break;
         case SC_TitleBarMinButton:
            if (isMinimized && subControl == SC_TitleBarMinButton)
               break;
            if (tb->titleBarFlags & Qt::WindowMinimizeButtonHint)
               offset += delta;
            else if (sc == SC_TitleBarMinButton)
               break;
         case SC_TitleBarCloseButton:
            if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
               offset += controlMargin;
            else if (sc == SC_TitleBarCloseButton)
               break;
            ret.setRect(tb->rect.left() + offset, tb->rect.top() + controlMargin,
                        controlHeight, controlHeight);
            break;

         case SC_TitleBarContextHelpButton:
            if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
               offset += delta;
         case SC_TitleBarShadeButton:
            if (!isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
               offset += delta;
            else if (sc == SC_TitleBarShadeButton)
               break;
         case SC_TitleBarUnshadeButton:
            if (isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
               offset += delta;
            else if (sc == SC_TitleBarUnshadeButton)
               break;
         case SC_TitleBarSysMenu:
            if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
               offset += delta + controlMargin;
            else if (sc == SC_TitleBarSysMenu)
               break;
            ret.setRect(tb->rect.right() - offset,
                        tb->rect.top() + controlMargin, controlHeight, controlHeight);
            break;
         default:
            break;
         }
         ret = visualRect(tb->direction, tb->rect, ret);
      }
      break;
   case CC_Dial: // A dial, like QDial
   default:
      ret = QCommonStyle::subControlRect ( control, option, subControl, widget);
   }
   return ret;
}

QRect
Style::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element)
    {
    case SE_PushButtonContents: // Area containing the label (icon with text or pixmap)
        return visualRect(option->direction, RECT, RECT);
    case SE_PushButtonFocusRect: // Area for the focus rect (usually larger than the contents rect)
    case SE_LineEditContents:
        return RECT.adjusted(F(2),0,-F(2),-F(1));
    case SE_CheckBoxContents: // Area for the state label
    case SE_ItemViewItemCheckIndicator: // Area for a view item's check mark
    case SE_CheckBoxIndicator: // Area for the state indicator (e.g., check mark)
    case SE_RadioButtonIndicator: // Area for the state indicator
    case SE_RadioButtonContents: // Area for the label
    {
        const int s = pixelMetric(PM_IndicatorHeight, option, widget);
        const int spacing = F(5);
        QRect r;
        if ( element == SE_CheckBoxContents ) {
            r.setRect(s + spacing, RECT.y(), RECT.width() - s - spacing, RECT.height());
        }
        else if ( element == SE_RadioButtonContents)
            r.setRect(s + spacing, RECT.y(), RECT.width() - s - spacing, RECT.height()-F(2));
        else
            r.setRect(RECT.x(), RECT.y() + (RECT.height()-s)/2, s, s);
        return visualRect(option->direction, RECT, r);
    }
    case SE_CheckBoxFocusRect: // Area for the focus indicator
    case SE_CheckBoxClickRect: // Clickable area, defaults to SE_CheckBoxFocusRect
    case SE_RadioButtonFocusRect: // Area for the focus indicator
    case SE_RadioButtonClickRect: // Clickable area, defaults to SE_RadioButtonFocusRect
        return RECT;

//    case SE_ComboBoxFocusRect: // Area for the focus indicator
//    case SE_SliderFocusRect: // Area for the focus indicator
//    case SE_Q3DockWindowHandleRect: // Area for the tear-off handle
    case SE_DockWidgetFloatButton:
    {
        QRect r = RECT;
        bool verticalTitleBar = false;
        if HAVE_OPTION(dock, DockWidget)
            verticalTitleBar = dock->verticalTitleBar;
        if (verticalTitleBar) {
            r.setHeight(16); r.moveTop(RECT.top()+F(4));
        } else {
            r.setWidth(16); r.moveRight(RECT.right()-F(4));
        }
        return r;
    }
    case SE_DockWidgetTitleBarText:
        return RECT;
    case SE_DockWidgetCloseButton:
    {
        QRect r = RECT;
        bool verticalTitleBar = false;
        if HAVE_OPTION(dock, DockWidget)
            verticalTitleBar = dock->verticalTitleBar;
        if (verticalTitleBar) {
            r.setHeight(16); r.moveBottom(RECT.bottom()-F(4));
        } else {
            r.setWidth(16); r.moveLeft(RECT.left()+F(4));
        }
        return r;
    }
    case SE_ProgressBarGroove: // Area for the groove
    case SE_ProgressBarContents: // Area for the progress indicator
    case SE_ProgressBarLabel: // Area for the text label
        return RECT;
//    case SE_DialogButtonAccept: // Area for a dialog's accept button
//    case SE_DialogButtonReject: // Area for a dialog's reject button
//    case SE_DialogButtonApply: // Area for a dialog's apply button
//    case SE_DialogButtonHelp: // Area for a dialog's help button
//    case SE_DialogButtonAll: // Area for a dialog's all button
//    case SE_DialogButtonRetry: // Area for a dialog's retry button
//    case SE_DialogButtonAbort: // Area for a dialog's abort button
//    case SE_DialogButtonIgnore: // Area for a dialog's ignore button
//    case SE_DialogButtonCustom: // Area for a dialog's custom widget area (in the button row)
    case SE_HeaderArrow:
    {
        int x,y,w,h;
        if (widget)
            option->rect.intersected(widget->rect()).getRect(&x,&y,&w,&h);
        else
            option->rect.getRect(&x,&y,&w,&h);
        int margin = F(2);// ;) pixelMetric(QStyle::PM_HeaderMargin, opt, widget);
        QRect r;
        if (option->state & State_Horizontal)
        {
            bool up = false;
            if HAVE_OPTION(hdr, Header)
                up = hdr->sortIndicator == QStyleOptionHeader::SortUp;
            const int h_3 = h / 3;
            r.setRect(x + (w - h_3)/2, up ? y+h-h_3 : y, h_3, h_3);
//          r.setRect(x + w - 2*margin - (h / 2), y + h/4 + margin, h / 2, h/2);
        }
        else
        {
            r.setRect(x + F(5), y, h / 2, h / 2 - margin * 2);
            r = visualRect(option->direction, option->rect, r);
        }
        return r;
    }
    case SE_HeaderLabel: //
        return RECT;
    case SE_TabWidgetLeftCorner:
    case SE_TabWidgetRightCorner:
        if HAVE_OPTION(twf, TabWidgetFrame) {
            const QSize &bar = twf->tabBarSize;
            const QSize &corner = (element == SE_TabWidgetLeftCorner) ? twf->leftCornerWidgetSize : twf->rightCornerWidgetSize;
            bool vertical = false;
            QRect ret;
            switch (twf->shape)
            {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                ret.setRect(RECT.x()+F(2), RECT.y(), corner.width(), bar.height()); break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                ret.setRect(RECT.x()+F(2), RECT.bottom()-bar.height(), corner.width(), bar.height()); break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                vertical = true;
                ret.setRect(RECT.x(), RECT.y()+F(2), bar.width(), corner.height()); break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                vertical = true;
                ret.setRect(RECT.right()-bar.width(), RECT.y()+F(2), bar.width(), corner.height()); break;
            }
            if (element == SE_TabWidgetRightCorner)
            {
                if (vertical)
                    ret.moveBottom(RECT.bottom()-F(2));
                else
                    ret.moveRight(RECT.right()-F(2));
            }
            return ret;
        }
    case SE_TabWidgetTabBar: {
        if HAVE_OPTION(twf, TabWidgetFrame) {
            QRect r = QCommonStyle::subElementRect(SE_TabWidgetTabBar, option, widget);
            if (verticalTabs(twf->shape)) {
                r.translate(0, F(4));
                if (r.bottom() > option->rect.bottom())
                    r.setBottom(option->rect.bottom());
            } else if (option->direction == Qt::LeftToRight) {
                r.translate(F(4), 0);
                if (r.right() > option->rect.right())
                    r.setRight(option->rect.right());
            } else { // rtl support
                r.translate(-F(4), 0);
                if (r.left() > option->rect.left())
                    r.setLeft(option->rect.left());
            }
            return r;
        }
    }
    case SE_TabWidgetTabContents:
        if HAVE_OPTION(twf, TabWidgetFrame)
        {
            QRect r = RECT;
            const QTabWidget *tw = qobject_cast<const QTabWidget*>(widget);
            const int margin =  tw && tw->documentMode() ? 0 : F(4);
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                r.adjust(margin, margin+twf->tabBarSize.height(), -margin, -margin);
            break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                r.adjust(margin, margin, -margin, -margin-twf->tabBarSize.height());
            break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                r.adjust(margin, margin, -margin-twf->tabBarSize.width(), -margin);
            break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                r.adjust(margin+twf->tabBarSize.width(), margin, -margin, -margin);
            }
            return r;
        }
    case QStyle::SE_TabBarTabLeftButton:
    case QStyle::SE_TabBarTabRightButton:
        if HAVE_OPTION(tab, Tab) {
            QSize sz = (element == SE_TabBarTabLeftButton) ? tab->leftButtonSize : tab->rightButtonSize;
            QRect r;
            if (verticalTabs(tab->shape)) {
                if (sz.width() > RECT.width())
                    sz.setWidth(RECT.width());
                r.setRect(RECT.x() + (RECT.width()-sz.width())/2, RECT.y() + F(2), sz.width(), sz.height());
                if (element == SE_TabBarTabRightButton)
                    r.moveBottom(RECT.bottom() - F(2));
                return r;
            } else {
                if (sz.height() > RECT.height())
                    sz.setHeight(RECT.height());
                r.setRect(RECT.x() + F(2), RECT.y() + (RECT.height()-sz.height())/2, sz.width(), sz.height());
                if (element == SE_TabBarTabRightButton)
                    r.moveRight(RECT.right()-F(2));
                return visualRect(tab->direction, RECT, r);
            }
        }
    case QStyle::SE_TabBarTabText:
        if HAVE_OPTION(tab, Tab) {
            const int dl = tab->leftButtonSize.isValid() ? tab->leftButtonSize.height() + F(4) : F(1);
            const int dr = tab->rightButtonSize.isValid() ? tab->rightButtonSize.height() + F(4) : F(1);
            if (verticalTabs(tab->shape)) {
                return QRect(RECT.y(), RECT.x() + dl, RECT.height(), RECT.width() - (dl+dr));
            } else {
                return visualRect(tab->direction, RECT, RECT.adjusted(dl, 0, -dr, 0));
            }
        }
    case SE_TabWidgetTabPane: //
        return QCommonStyle::subElementRect(SE_TabWidgetTabPane, option, widget);
        // return RECT;
//     case SE_ItemViewItemFocusRect:
//     case SE_ItemViewItemText:
//     case SE_TreeViewDisclosureItem: //Area for the actual disclosure item in a tree branch.
    case SE_ToolBoxTabContents: // Area for a toolbox tab's icon and label
        return RECT.adjusted( F(3), F(3), -F(3), -F(3) );
    case SE_TabBarTearIndicator: { // Area for the tear indicator on a tab bar with scroll arrows.
        return QCommonStyle::subElementRect(SE_TabBarTearIndicator, option, widget);
    }
    default:
        return QCommonStyle::subElementRect ( element, option, widget);
   }
}
