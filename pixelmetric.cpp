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

#include <QAbstractScrollArea>
#include <QFrame>
#include <QFontMetrics>
#include <QSlider>
#include <QStyleOptionTabWidgetFrame>
#include <QTabBar>
#include <QTabWidget>
#include "virtuality.h"
#include "hacks.h"
#include "makros.h"

using namespace BE;
#include <QtDebug>
int Style::pixelMetric( PixelMetric pm, const QStyleOption *option, const QWidget *widget ) const
{
    switch ( pm )
    {
    case PM_ButtonMargin: // Amount of whitespace between push button labels and the frame
        return F(2); //FRAME_STROKE_WIDTH;
    case PM_ButtonDefaultIndicator: // Width of the default-button indicator frame
        return 0;
    case PM_MenuButtonIndicator: // Width of the menu button indicator proportional to the widget height
        return F(7);
    case PM_ButtonShiftHorizontal: // Horizontal contents shift of a button when the button is down
    case PM_ButtonShiftVertical: // Vertical contents shift of a button when the button is down
        return 0;
    case PM_DefaultFrameWidth: // 2
//         if (const QFrame *frame = qobject_cast<const QFrame*>(widget))
//         if (frame->frameShape() != QFrame::StyledPanel)
//             return F(6);
//         if (qobject_cast<const QAbstractScrollArea*>(widget))
            return FRAME_STROKE_WIDTH + (config.frame.roundness+3)/4 + config.frame.roundness/20;
//         return F(2);
    case PM_SpinBoxFrameWidth: // Frame width of a spin box, defaults to PM_DefaultFrameWidth
        return F(1);
    case PM_ComboBoxFrameWidth: // Frame width of a combo box, defaults to PM_DefaultFrameWidth.
        return 1; //  cosmetic, qt hates 0 sized popupframes
    case PM_MdiSubWindowFrameWidth: // Frame width of an MDI window
        return 1;
    //    case PM_MDIMinimizedWidth: // Width of a minimized MDI window
    case PM_MaximumDragDistance: // Some feels require the scroll bar or other sliders to jump back to the original position when the mouse pointer is too far away while dragging; a value of -1 disables this behavior
        return -1;
    case PM_ScrollView_ScrollBarSpacing:
        return F(1);
    case PM_ScrollBarExtent: // Width of a vertical scroll bar and the height of a horizontal scroll bar
        return config.slider.thickness - F(2);
    case PM_ScrollBarSliderMin: // The minimum height of a vertical scroll bar's slider and the minimum width of a horizontal scroll bar's slider
        return F(32);
    case PM_SliderThickness: // Total slider thickness
    case PM_SliderControlThickness: // Thickness of the slider handle
    case PM_SliderLength: // Length of the slider
        return config.slider.thickness;
   case PM_SliderTickmarkOffset: // The offset between the tickmarks and the slider
       return 0;
    case PM_SliderSpaceAvailable:
    {   // The available space for the slider to move
        if (const QSlider *slider = qobject_cast<const QSlider*>(widget))
        {
            if (slider->orientation() == Qt::Horizontal)
                return (widget->width() - config.slider.thickness);
            else
                return (widget->height() - config.slider.thickness);
        }
        return 0;
    }
    case QStyle::PM_DockWidgetTitleBarButtonMargin:
        return 0;//F(0);
    case QStyle::PM_DockWidgetTitleMargin:
        if (widget &&widget->windowTitle().isEmpty())
            return 0;
        return F(4);
    case PM_DockWidgetSeparatorExtent: // Width of a separator in a horizontal dock window and the height of a separator in a vertical dock window
        return F(6);
    case PM_DockWidgetHandleExtent: // Width of the handle in a horizontal dock window and the height of the handle in a vertical dock window
        return F(6);
    case PM_DockWidgetFrameWidth: // Frame width of a dock window
        return 0; //F(1);
    case PM_MenuBarPanelWidth: // Frame width of a menubar, defaults to PM_DefaultFrameWidth
        return 0;
    case PM_MenuBarItemSpacing: // Spacing between menubar items
        return 0; //F(2);
    case PM_MenuBarHMargin: // Spacing between menubar items and left/right of bar
        return F(2);
    case PM_MenuBarVMargin: // Spacing between menubar items and top/bottom of bar
        return 0;
    case PM_ToolBarFrameWidth: // Width of the frame around toolbars
        return 0; //F(1);//f4;
    case PM_ToolBarHandleExtent: // Width of a toolbar handle in a horizontal toolbar and the height of the handle in a vertical toolbar
        return F(6);
    case PM_ToolBarItemMargin: // Spacing between the toolbar frame and the items
        return F(2);
    case PM_ToolBarItemSpacing: // Spacing between toolbar items
        return F(4);
    case PM_ToolBarSeparatorExtent: // Width of a toolbar separator in a horizontal toolbar and the height of a separator in a vertical toolbar
        return F(6);
    case PM_ToolBarExtensionExtent: // Width of a toolbar extension button in a horizontal toolbar and the height of the button in a vertical toolbar
        return F(13);
    case PM_TabBarTabOverlap: // Number of pixels the tabs should overlap
        return 0;
    case PM_TabBarTabHSpace: // Extra space added to the tab width
        return F(12);
    case PM_TabBarTabVSpace: // Extra space added to the tab height
        return F(8);
    case PM_TabBarBaseHeight: // Height of the area between the tab bar and the tab pages
    case PM_TabBarBaseOverlap: { // Number of pixels the tab bar overlaps the tab bar base
    // ... but yesterday it was...
#if 1
        if (!widget) // dragging the base below the tabs doesn't work w/ QML :-(
            return 0; //16*QFontMetrics(QFont()).height()/10;

        if (appType == KDevelop)
            return 0;

        const QTabBar *tabBar = qobject_cast<const QTabBar*>(widget);
        if (!tabBar) {
        if (const QTabWidget *tw = qobject_cast<const QTabWidget*>(widget)) {
            if ( tw->styleSheet().contains("pane", Qt::CaseInsensitive) &&
                 tw->styleSheet().contains("border", Qt::CaseInsensitive))
                return 0;
#if QT_VERSION < 0x050000
            foreach(QObject *obj, widget->children())
                if (qobject_cast<QTabBar*>(obj))
                    { tabBar = (QTabBar*)obj; break; }
#else
            tabBar = tw->tabBar();
#endif
        } }
        if (!tabBar || !tabBar->isVisible())
            return 0; //F(16);

        if (const QStyleOptionTabWidgetFrame *twf =
            qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            if (twf->shape == QTabBar::RoundedEast || twf->shape == QTabBar::TriangularEast ||
                twf->shape == QTabBar::RoundedWest || twf->shape == QTabBar::TriangularWest)
                return tabBar->width();
        }
        return tabBar->height();
#endif
    }
    case PM_TabBarScrollButtonWidth: //
        return F(16);
    case PM_TabBarTabShiftHorizontal: // Horizontal pixel shift when a tab is selected
        return 0;
    case PM_TabBarTabShiftVertical: // Vertical pixel shift when a tab is selected
        return 0; // problem solved outside ;-)
//    case PM_ProgressBarChunkWidth: // Width of a chunk in a progress bar indicator
    case PM_SplitterWidth: // Width of a splitter
        return F(6);
    case PM_TitleBarHeight: // Height of the title bar
        return F(20);
    case PM_IndicatorWidth: // Width of a check box indicator
    case PM_IndicatorHeight: // Height of a checkbox indicator
    case PM_ExclusiveIndicatorWidth: // Width of a radio button indicator
    case PM_ExclusiveIndicatorHeight: // Height of a radio button indicator
        return F(17);
    case PM_MenuPanelWidth: // Border width (applied on all sides) for a QMenu
        return 1; // cosmetic, qt hates 0 sized popupframes
    case PM_MenuHMargin: // Additional border (used on left and right) for a QMenu
    case PM_MenuVMargin: // Additional border (used for bottom and top) for a QMenu
        return F(1);
//    case PM_MenuScrollerHeight: // Height of the scroller area in a QMenu
//    case PM_MenuTearoffHeight: // Height of a tear off area in a QMenu
    case PM_MenuDesktopFrameWidth: //
        return 4;
//    case PM_CheckListButtonSize: // Area (width/height) of the checkbox/radio button in a Q3CheckListItem
//    case PM_CheckListControllerSize: // Area (width/height) of the controller in a Q3CheckListItem
//       if (option) return option->rect.height()-F(4);
//       return F(16);
   case PM_DialogButtonsSeparator:
       return F(4);
   case PM_DialogButtonsButtonWidth:
       return F(80);
//    case PM_DialogButtonsButtonHeight: // Minimum height of a button in a dialog buttons widget
//    case PM_HeaderMarkSize: //
//    case PM_HeaderGripMargin: //
    case PM_HeaderMargin: //
        return F(2);
    case PM_SpinBoxSliderHeight: // The height of the optional spin box slider
        return F(4);
    case PM_LayoutBottomMargin:
    case PM_LayoutHorizontalSpacing:
        return F(9);
    case PM_LayoutTopMargin:
    case PM_LayoutVerticalSpacing:
        return F(6);
    case PM_LayoutLeftMargin:
    case PM_LayoutRightMargin:
        return F(8);
    case PM_ToolBarIconSize: // Default tool bar icon size, defaults to PM_SmallIconSize
        return F(22);
    case PM_SmallIconSize: // Default small icon size
        return F(16);
    case PM_LargeIconSize: // Default large icon size
        return F(32);
//     case PM_FocusFrameHMargin: // Horizontal margin that the focus frame will outset the widget by.
//         return //F(4); // GNARF, KCategoryView or dolphin hardcodes this value to Qt def. TODO!
    case PM_FocusFrameVMargin: // Vertical margin that the focus frame will outset the widget by.
        return 0;
   case PM_IconViewIconSize: //
       return F(32);
   case PM_ListViewIconSize: //
       return F(22);
    case PM_TabBarIconSize:
    case PM_ButtonIconSize:
        return (16);
    case PM_MessageBoxIconSize:
        return F(64);
    case PM_ToolTipLabelFrameWidth: //
        return F(4); // they're so tiny ;)
    default:
        return QCommonStyle::pixelMetric( pm, option, widget );
    } // switch
}
