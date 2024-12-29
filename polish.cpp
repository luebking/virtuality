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
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QCommandLinkButton>
#include <QDockWidget>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLCDNumber>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>
#include <QTreeView>
#include <QWizard>

#include <QTextBrowser>
#include <QTextDocument>

#include <QGraphicsView>
#include <QGraphicsWidget>

#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>

#include <QtDebug>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <cmath>

#include "FX.h"
#include "shadows.h"

#ifndef QT_NO_DBUS
#include "macmenu.h"
#endif
#ifdef BE_WS_X11
#include "xproperty.h"
#include "fixx11h.h"
#endif

#include "hacks.h"
#include "virtuality.h"

#include "animator/focus.h"
#include "animator/hover.h"
#include "animator/aprogress.h"
#include "animator/tab.h"

#include "makros.h"
#undef CCOLOR
#undef FCOLOR
#define CCOLOR(_TYPE_, _FG_) PAL.color(QPalette::Active, Style::config._TYPE_##_role[_FG_])
#define FCOLOR(_TYPE_) PAL.color(QPalette::Active, QPalette::_TYPE_)
#define FILTER_EVENTS(_WIDGET_) { _WIDGET_->removeEventFilter(this); _WIDGET_->installEventFilter(this); } // skip semicolon

#define BESPIN_MOUSE_DEBUG 0

using namespace BE;

Hacks::Config Hacks::config;

static inline void
setBoldFont(QWidget *w, bool bold = true)
{
    if (w->font().pointSize() < 1)
        return;
    QFont fnt = w->font();
    fnt.setBold(bold);
    w->setFont(fnt);
}

void Style::polish(QApplication *app)
{
    QPalette pal = app->palette();
    polish(pal);
    QPalette *opal = originalPalette;
    originalPalette = 0; // so our eventfilter won't react on this... ;-P
    app->setPalette(pal);
    originalPalette = opal;
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))
#define _ALIGN_COLOR_(_FROM_, _TO_) \
pal.setColor(QPalette::Active, QPalette::_FROM_, pal.color(QPalette::Active, QPalette::_TO_));\
pal.setColor(QPalette::Inactive, QPalette::_FROM_, pal.color(QPalette::Inactive, QPalette::_TO_));\
pal.setColor(QPalette::Disabled, QPalette::_FROM_, pal.color(QPalette::Disabled, QPalette::_TO_));


#undef PAL
#define PAL pal

#define SWAP_ALL(_P_, _R1_, _R2_) FX::swap(_P_, QPalette::Active, _R1_, _R2_); FX::swap(_P_, QPalette::Inactive, _R1_, _R2_); FX::swap(_P_, QPalette::Disabled, _R1_, _R2_)


void Style::polish( QPalette &pal, bool onInit )
{
    QColor c = pal.color(QPalette::Active, QPalette::Window);
    pal.setColor( QPalette::Window, c );

    if (onInit) {
        _ALIGN_COLOR_(Button, Window);
        _ALIGN_COLOR_(ButtonText, WindowText);
        _ALIGN_COLOR_(Base, Window);
        _ALIGN_COLOR_(Text, WindowText);
        _ALIGN_COLOR_(ToolTipBase, WindowText);
        _ALIGN_COLOR_(ToolTipText, Window);

        invertedPalette = pal;
        SWAP_ALL(invertedPalette, QPalette::Window, QPalette::WindowText);
        SWAP_ALL(invertedPalette, QPalette::Base, QPalette::Text);
        SWAP_ALL(invertedPalette, QPalette::Button, QPalette::ButtonText);
        SWAP_ALL(invertedPalette, QPalette::Highlight, QPalette::HighlightedText);
        invertedPalette.setColor(QPalette::PlaceholderText, FX::blend(invertedPalette.color(QPalette::Active, QPalette::Base),
                                                                      invertedPalette.color(QPalette::Active, QPalette::Text), 70,30));
        polish(invertedPalette, false);
    }

//     Style::polish(invertedPalette, true);
//     QPixmap brush(32,32);
//     brush.fill(pal.color(QPalette::Window).lighter(101));
//     QPainter p(&brush);
//     p.setPen(pal.color(QPalette::Window).darker(102));
//     for (int i = 2; i < 33; i+=4)
//         p.drawLine(0,i,32,i);
//     p.end();
//     pal.setBrush(QPalette::Active, QPalette::Base, brush);
//     pal.setBrush(QPalette::Inactive, QPalette::Base, brush);
//     pal.setBrush(QPalette::Disabled, QPalette::Base, brush);

    // AlternateBase
    pal.setColor(QPalette::AlternateBase, FX::blend(pal.color(QPalette::Active, QPalette::Base),
                                                      pal.color(QPalette::Active, QPalette::Text), 100,3));

    // Link colors can not be set through qtconfig - and the colors suck
    QColor link = pal.color(QPalette::Active, QPalette::Highlight);
    const int vwt = FX::value(pal.color(QPalette::Active, QPalette::Window));
    const int vt = FX::value(pal.color(QPalette::Active, QPalette::Base));
    int h,s,v;
    link.getHsv(&h,&s,&v);
    if (s < 16) { // low saturated color - maybe we've more luck with the foreground
        QColor link2 = pal.color(QPalette::Active, QPalette::HighlightedText);
        int s2;
        link2.getHsv(&h,&s2,&v);
        if (s2 > s) {
            link = link2;
            s = s2;
        }
        else // re-fix attributes
            link.getHsv(&h,&s,&v);
    }
    s = sqrt(s/255.0)*255.0;

    if (vwt > 128 && vt > 128)
        v = 3*v/4;
    else if (vwt < 128 && vt < 128)
        v = qMin(255, 7*v/6);

    link.setHsv(h, s, v);
    pal.setColor(QPalette::Link, link);

    link = FX::blend(link, FX::blend(pal.color(QPalette::Active, QPalette::Text),
                                                         pal.color(QPalette::Active, QPalette::WindowText)), 4, 1);
    pal.setColor(QPalette::LinkVisited, link);

    // inactive palette
    if (config.fadeInactive)
    { // fade out inactive foreground and highlight colors...
        pal.setColor(QPalette::Inactive, QPalette::Window, pal.color(QPalette::Active, QPalette::Window));
        pal.setColor(QPalette::Inactive, QPalette::WindowText,
                     FX::blend(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::WindowText), 1,4));
        pal.setColor(QPalette::Inactive, QPalette::Button, pal.color(QPalette::Active, QPalette::Button));
        pal.setColor(QPalette::Inactive, QPalette::ButtonText,
                     FX::blend(pal.color(QPalette::Active, QPalette::Button), pal.color(QPalette::Active, QPalette::ButtonText), 1,4));
        pal.setColor(QPalette::Inactive, QPalette::Base, pal.color(QPalette::Active, QPalette::Base));
        pal.setColor(QPalette::Inactive, QPalette::Text,
                     FX::blend(pal.color(QPalette::Active, QPalette::Base), pal.color(QPalette::Active, QPalette::Text), 1,4));
    }

    // fade disabled palette
    pal.setColor(QPalette::Disabled, QPalette::WindowText,
                 FX::blend(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::WindowText),2,1));
    pal.setColor(QPalette::Disabled, QPalette::Base,
                 FX::blend(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::Base),1,2));
    pal.setColor(QPalette::Disabled, QPalette::Text,
                 FX::blend(pal.color(QPalette::Active, QPalette::Base), pal.color(QPalette::Active, QPalette::Text)));
    pal.setColor(QPalette::Disabled, QPalette::AlternateBase, pal.color(QPalette::Disabled, QPalette::Base));

    // highlight colors
    h = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
    QColor hgt(h,h,h);
    pal.setColor(QPalette::Inactive, QPalette::Highlight, hgt);
    pal.setColor(QPalette::Disabled, QPalette::Highlight, hgt);
    hgt = FX::blend(hgt, pal.color(QPalette::Active, QPalette::HighlightedText));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, hgt);
    h = qGray(pal.color(QPalette::Active, QPalette::HighlightedText).rgb());
    hgt = QColor(h,h,h);
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText, hgt);

    // more on tooltips... (we force some colors...)
    if (!onInit)
        return;

    QPalette toolPal = QApplication::palette();
    const QColor bg = pal.color(QPalette::Active, QPalette::WindowText);
    const QColor fg = pal.color(QPalette::Active, QPalette::Window);
    toolPal.setColor(QPalette::Window, bg);
    toolPal.setColor(QPalette::WindowText, fg);
    toolPal.setColor(QPalette::Base, bg);
    toolPal.setColor(QPalette::Text, fg);
    toolPal.setColor(QPalette::Button, bg);
    toolPal.setColor(QPalette::ButtonText, fg);
    toolPal.setColor(QPalette::Highlight, fg); // sic!
    toolPal.setColor(QPalette::HighlightedText, bg); // sic!
    toolPal.setColor(QPalette::ToolTipBase, bg);
    toolPal.setColor(QPalette::ToolTipText, fg);
    QToolTip::setPalette(toolPal);


#ifdef BE_WS_X11
    if (appType == GTK)
        setupDecoFor(NULL, pal);
#endif
}


#if 0
static QMenuBar *
bar4popup(QMenu *menu)
{
    if (!menu->menuAction())
        return 0;
    if (menu->menuAction()->associatedWidgets().isEmpty())
        return 0;
    foreach (QWidget *w, menu->menuAction()->associatedWidgets())
        if (qobject_cast<QMenuBar*>(w))
            return static_cast<QMenuBar *>(w);
    return 0;
}
#endif


inline static void
polishGTK(QWidget * widget)
{
    enum MyRole{Bg = Style::Bg, Fg = Style::Fg};
    QColor c1, c2, c3, c4;

    if (widget->objectName() == "QTabWidget" ||
        widget->objectName() == "QTabBar")
    {
        QPalette pal = widget->palette();
        pal.setColor(QPalette::Disabled, QPalette::WindowText, FX::blend(FCOLOR(WindowText), FCOLOR(Window), 3, 1));
        pal.setColor(QPalette::Inactive, QPalette::Window, FCOLOR(WindowText));
        pal.setColor(QPalette::Active, QPalette::Window, FCOLOR(WindowText));
        pal.setColor(QPalette::Inactive, QPalette::WindowText, FCOLOR(Window));
        pal.setColor(QPalette::Active, QPalette::WindowText, FCOLOR(Window));
        widget->setPalette(pal);
    }

    if (widget->objectName() == "QMenu" )
    {
        QPalette pal = widget->palette();
        pal.setColor(QPalette::Inactive, QPalette::Window, FCOLOR(WindowText));
        pal.setColor(QPalette::Active, QPalette::Window, FCOLOR(WindowText));
        pal.setColor(QPalette::Inactive, QPalette::WindowText, FCOLOR(Window));
        pal.setColor(QPalette::Active, QPalette::WindowText, FCOLOR(Window));
        widget->setPalette(pal);
    }
}

static QAction *dockLocker = 0;

static void invertContainer(QWidget *widget, const QPalette &invertedPalette)
{
    widget->setPalette(invertedPalette);
    QList<QWidget*> kids = widget->findChildren<QWidget*>();
    for (int i = kids.count()-1; i > -1; --i) {
        QWidget *kid = kids.at(i);
        if (kid->testAttribute(Qt::WA_SetPalette) || kid->testAttribute(Qt::WA_StyleSheet)) {
            kid->setPalette(invertedPalette); // shitted widgets inherit the app wide palette ...
        }
        if (kid->testAttribute(Qt::WA_StyleSheet) && !kid->styleSheet().isEmpty()) {
            QString shit(kid->styleSheet());
            // the shit uses the app wide FG color, so we need to force the correct one
            // NOTICE: the leading ';' is for semi-broken styleshits that omit the trailing ';'
            int idx = shit.lastIndexOf('}');
            if (idx > -1)
                shit = shit.left(idx) + ";color:" + invertedPalette.color(QPalette::Active, QPalette::WindowText).name() + ";}";
            else
                shit.append(";color:" + invertedPalette.color(QPalette::Active, QPalette::WindowText).name() + ";");
            kid->setStyleSheet(shit);
        }
    }
}

void
Style::polish( QWidget * widget )
{
    // GTK-Qt gets a special handling - see above
    if (appType == GTK)
    {
        polishGTK(widget);
        return;
    }

    // !!! protect against polishing /QObject/ attempts! (this REALLY happens from time to time...)
    if (!widget)
        return;

//     if (widget->inherits("QGraphicsView"))
//         widget->setPalette(invertedPalette);
//         qDebug() << "BESPIN" << widget;
#if BESPIN_MOUSE_DEBUG
    FILTER_EVENTS(widget);
#endif

    // apply any user selected hacks
    Hacks::add(widget);

    KStyleFeatureRequest kStyleFeatureRequest = (KStyleFeatureRequest)widget->property("KStyleFeatureRequest").toUInt();

    //BEGIN Window handling                                                                        -
    if ( widget->isWindow() &&
//          widget->testAttribute(Qt::WA_WState_Created) &&
//          widget->internalWinId() &&
            !(widget->inherits("QSplashScreen") || widget->inherits("KScreenSaver")
            || widget->objectName() == "decoration widget" /*|| widget->inherits("QGLWidget")*/ ) )
    {

        /// this is dangerous! e.g. applying to QDesktopWidget leads to infinite recursion...
        /// also doesn't work bgs get transparent and applying this to everything causes funny sideeffects...

//         qDebug() << widget << widget->windowType();
        if ( widget->windowType() == Qt::ToolTip)
        {
            if (config.bg.modal.opacity < 0xff)
                widget->setWindowOpacity(config.bg.modal.opacity/255.0);
            if (widget->inherits("BrightnessOSDWidget")) {
                // ok. some moron thinks it's required to tell me that the screen brightness just changed. not like the entire screen wouldn't tell me
                // then some other moron comes around and makes this stupid thing grab input focus (bypassing the wm)
                // this does nicely breag drag and drop and interfers anytime you do actually something to *tell* you what you just saw. ***grrrr***
                FILTER_EVENTS(widget); // so we suppress this bloody shit.
            }
            if (widget->inherits("QTipLabel") || widget->inherits("KToolTipWindow") || widget->inherits("FileMetaDataToolTip"))
            {
                if ( !(widget->testAttribute(Qt::WA_TranslucentBackground) || kStyleFeatureRequest & NoARGB) && FX::compositingActive() )
                    widget->setAttribute(Qt::WA_TranslucentBackground);
                widget->setProperty("BespinWindowHints", config.frame.roundness?Rounded:0);
                if (!(kStyleFeatureRequest & NoShadow))
                    Shadows::manage(widget);
            }
        }
        else if (widget->windowType() == Qt::Popup)
        {
            if (config.bg.modal.opacity < 0xff)
                widget->setWindowOpacity(config.bg.modal.opacity/255.0);
            if (!widget->testAttribute(Qt::WA_TranslucentBackground) && widget->mask().isEmpty()) {
                if (config.invert.menus && widget->inherits("QComboBoxPrivateContainer")) {
                    widget->setPalette(invertedPalette);
                }
                if (!(kStyleFeatureRequest & NoShadow))
                    Shadows::manage(widget);
            }
        } else if (widget->windowType() == Qt::Dialog) {
            if (config.bg.ringOverlay && !widget->testAttribute(Qt::WA_TranslucentBackground)) {
                widget->setAttribute(Qt::WA_StyledBackground);
            }
        } else if ( widget->testAttribute(Qt::WA_X11NetWmWindowTypeDND) && FX::compositingActive() )
        {
            if (!(kStyleFeatureRequest & NoARGB))
                widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->clearMask();
        }

        if ( QMainWindow *mw = qobject_cast<QMainWindow*>(widget) ) {
            if (appType == Dolphin)
                mw->setDockOptions(mw->dockOptions()|QMainWindow::VerticalTabs);
//             mw->setTabPosition ( Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea, QTabWidget::North );
            if (config.invert.docks && mw->centralWidget()) {
                mw->centralWidget()->setAttribute(Qt::WA_StyledBackground);
                mw->centralWidget()->setProperty("Virtuality.centralWidget", true);
            }
            if (Hacks::config.lockDocks) {
                if (!dockLocker) {
                    dockLocker = new QAction( "Locked Dock Positions", qApp );
                    dockLocker->setShortcutContext( Qt::ApplicationShortcut );
                    dockLocker->setShortcuts( QList<QKeySequence>() << QKeySequence("Ctrl+Alt+D") );
                    dockLocker->setEnabled( true );
                    dockLocker->setCheckable( true );
                    dockLocker->setChecked( true );
                    connect ( dockLocker, SIGNAL(toggled(bool)), SLOT(unlockDocks(bool)) );
                }
                widget->addAction( dockLocker );
            }
        } else if ( QWizard *wiz = qobject_cast<QWizard*>(widget) ) {
            if (config.macStyle && wiz->pixmap(QWizard::BackgroundPixmap).isNull())
            {
                if (!wiz->pixmap(QWizard::WatermarkPixmap).isNull())
                    wiz->setPixmap( QWizard::BackgroundPixmap, wiz->pixmap(QWizard::WatermarkPixmap) );
                else
                {
                    QPixmap pix(468,128);
                    pix.fill(Qt::transparent);
                    QRect r(0,0,128,128);
                    QPainter p(&pix);
                    QColor c = wiz->palette().color(wiz->foregroundRole());
                    c.setAlpha(24);
                    p.setBrush(c);
                    p.setPen(Qt::NoPen);
                    Navi::Direction dir = wiz->layoutDirection() == Qt::LeftToRight ? Navi::E : Navi::W;
                    Style::drawArrow(dir, r, &p);
                    r.translate(170,0); Style::drawArrow(dir, r, &p);
                    r.translate(170,0); Style::drawArrow(dir, r, &p);
                    p.end();
                    wiz->setPixmap( QWizard::BackgroundPixmap, pix );
                }
            }
        }
        //BEGIN Popup menu handling                                                                -
        if (QMenu *menu = qobject_cast<QMenu *>(widget)) {
            // opacity
            if ((config.bg.modal.opacity != 0xff || config.frame.roundness) && !(kStyleFeatureRequest & NoARGB)) {
                if (appType == Plasma || !(menu->testAttribute(Qt::WA_TranslucentBackground))) {
                    menu->setAttribute(Qt::WA_TranslucentBackground);
                    menu->setAttribute(Qt::WA_StyledBackground);
                    menu->setAutoFillBackground(false);
                }
            } else {
                menu->setAutoFillBackground(true);
            }

            // color swapping
            if (config.invert.menus)
                menu->setPalette(invertedPalette);

            if (appType == Plasma) // GNARF!
                menu->setWindowFlags( menu->windowFlags()|Qt::Popup);

            menu->setProperty("BespinWindowHints", config.frame.roundness ? Rounded : 0);
            if (!(kStyleFeatureRequest & NoShadow))
                Shadows::manage(menu);

            // eventfiltering to reposition MDI windows, shaping, shadows, paint ARGB bg and correct distance to menubars
            FILTER_EVENTS(menu);
        }
        //END Popup menu handling                                                                  -
        /// WORKAROUND Qt color bug, uses daddies palette and FGrole, but TooltipBase as background
        else if (widget->inherits("QWhatsThat"))
        {
//             FILTER_EVENTS(widget); // IT - LOOKS - SHIT - !
            widget->setPalette(QToolTip::palette()); // so this is Qt bug WORKAROUND
//             widget->setProperty("BespinWindowHints", Shadowed);
//             Shadows::set(widget->winId(), Shadows::Small);
        }
        else
        {
            // talk to kwin about colors, gradients, etc.
            Qt::WindowFlags ignore =    Qt::Popup | Qt::ToolTip |
                                        Qt::SplashScreen | Qt::Desktop |
                                        Qt::X11BypassWindowManagerHint;// | Qt::FramelessWindowHint; <- could easily change mind...?!
            ignore &= ~(Qt::Dialog|Qt::Sheet); // erase dialog, it's in drawer et al. but takes away window as well
            // this can be expensive, so avoid for popups, combodrops etc.
            if (!(widget->windowFlags() & ignore)) {
                if (widget->isVisible())
                    setupDecoFor(widget, widget->palette());
                FILTER_EVENTS(widget); // catch show event and palette changes for deco
            }
        }
        if (kStyleFeatureRequest & Shadow)
            Shadows::manage(widget);
    }
    //END Window handling                                                                          -

    //BEGIN Frames                                                                                 -
    if (QFrame *frame = qobject_cast<QFrame*>(widget)) {
        if (frame->isWindow()) {
            frame->setFrameShape(QFrame::NoFrame); // no. ugly & pointless.
        } else {
            if (QLabel *label = qobject_cast<QLabel*>(frame)) {
                if (label->parentWidget() && label->parentWidget()->inherits("KFontRequester"))
                    label->setAlignment(Qt::AlignCenter); // fix alignment
            } else if (frame->parentWidget() && frame->parentWidget()->inherits("KTitleWidget")) {
                if (config.invert.headers) {
                    frame->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
                    frame->setAutoFillBackground(true);
                    frame->setBackgroundRole(QPalette::WindowText);
                    frame->setForegroundRole(QPalette::Window);
                    if (frame->layout())
                        frame->layout()->setContentsMargins(F(6),0,F(6),0);
                } else {
                    frame->setFrameShape(QFrame::NoFrame);
                    frame->setAutoFillBackground(false);
                    if (frame->layout())
                        frame->layout()->setContentsMargins(0,0,0,0);
                }
                QList<QLabel*> labels = frame->findChildren<QLabel*>();
                foreach (QLabel *label, labels) {
                    label->setAutoFillBackground(false);
                    label->setBackgroundRole(frame->backgroundRole());
                    label->setForegroundRole(frame->foregroundRole());
                }
            } else if (frame->parentWidget() && frame->parentWidget()->inherits("KateView")) { // nonono..
                frame->setFrameShape(QFrame::NoFrame);
            }
        }
        // just saw they're niftier in skulpture -> had to do sth. ;-P
        if ( QLCDNumber *lcd = qobject_cast<QLCDNumber*>(frame) )
        {
            if (lcd->frameShape() != QFrame::NoFrame)
                lcd->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
            lcd->setSegmentStyle(QLCDNumber::Flat);
            lcd->setAutoFillBackground(true);
        }

        // scrollarea hovering
        if ( QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea*>(frame) ) {
            if (appType == Dolphin && (config.invert.docks || config.invert.toolbars) &&
                area->parentWidget() && area->parentWidget()->inherits("DolphinView"))
                area->parentWidget()->setContentsMargins(F(4),F(4),F(4),F(4));
            Animator::Hover::manage(frame);
            if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(frame) ) {
                if (widget->inherits("KCompletionBox") && !(kStyleFeatureRequest & NoShadow))
                    Shadows::manage(widget);
                else if (widget->inherits("QTableView")) {
                    frame->setFrameShape(QFrame::NoFrame); // ugly and superfluous - has grid and/or headers
#if QT_VERSION >= 0x060000
                    if (widget->inherits("QtPrivate::QCalendarView")) {
#else
                    if (widget->inherits("QCalendarView")) {
#endif
                        // MEGAUGLY HACK
                        // QCalendarView looks shit. I want. *want* the selected date round.
                        // unfortunately the view uses a private delegate on top of QItemDelegate
                        // which just paints rects.
                        // Tried tricking by erasing with the focus frame, but that only works while
                        // the widget has the foucs.
                        // So instead we set the Highlight role to 0 alpha -> the stupid delegate will
                        // paint the big invisibility, what means the PE item from QTableView shines
                        // through... which happens to be painted by us >-)
                        QPalette pal = widget->palette();
                        QColor c = pal.color(QPalette::Active, QPalette::Highlight); c.setAlpha(0);
                        pal.setColor(QPalette::Active, QPalette::Highlight, c);
                        c = pal.color(QPalette::Inactive, QPalette::Highlight); c.setAlpha(0);
                        pal.setColor(QPalette::Inactive, QPalette::Highlight, c);
                        c = pal.color(QPalette::Disabled, QPalette::Highlight); c.setAlpha(0);
                        pal.setColor(QPalette::Disabled, QPalette::Highlight, c);
                        widget->setPalette(pal);
                    }
                } else if (itemView->inherits("KCategorizedView")) { // fix scrolldistance...
                    FILTER_EVENTS(itemView);
                } else if ( QTreeView* tv = qobject_cast<QTreeView*>(itemView) ) {
                    tv->setAnimated(true); // allow all treeviews to be animated!
                }

                if (qobject_cast<QHeaderView*>(itemView)) {
                    if (config.invert.headers) {
                        itemView->setBackgroundRole(QPalette::Text);
                        itemView->setForegroundRole(QPalette::Base);
                    } else {
                        itemView->setBackgroundRole(QPalette::Base);
                        itemView->setForegroundRole(QPalette::Text);
                    }
                    widget->setAttribute(Qt::WA_Hover);
                } else if (QWidget *vp = itemView->viewport()) {
                    vp->setAttribute(Qt::WA_Hover);
                }

                if (appType == Amarok) // fix the palette anyway. amarok tries to reset it's slooww transparent one... gnagnagna
                    FILTER_EVENTS(itemView);
            }
            // just use <strike>broadsword</strike> <strike>gladius</strike> foil here
            // the stupid viewport should use the mouse...
            else  if (area->viewport() && !qobject_cast<QAbstractScrollArea*>(area->parent()) &&
                                            !qobject_cast<QGraphicsView*>(area->viewport())) {
                area->viewport()->setAttribute(Qt::WA_NoMousePropagation);
            }
            // Dolphin Information panel still (again?) does this
            // *sigh* - this cannot be true. this CANNOT be true. this CAN NOT BE TRUE!
            if (area->viewport() && area->viewport()->autoFillBackground() && !area->viewport()->palette().color(area->viewport()->backgroundRole()).alpha() )
                area->viewport()->setAutoFillBackground(false);
//             if (QGraphicsView *qgc = qobject_cast<QGraphicsView*>(area)) {
//                 QWidget *runner = area;
//                 while (runner = runner->parentWidget()) {
//                     if (runner->palette() == invertedPalette) {
//                         if (qgc->scene()) {
//                             QPalette pal = area->palette();
//                             pal.setColor(QPalette::Text, Qt::red);
//                             pal.setColor(QPalette::WindowText, Qt::green);
//                             qDebug() << "gotcha!";
//                             qgc->scene()->setPalette(pal);
//                             QList<QGraphicsItem*> fuckers = qgc->scene()->items();
//                             foreach (QGraphicsItem *fucker, fuckers) {
//                                 if (QGraphicsWidget *sucker = dynamic_cast<QGraphicsWidget*>(fucker)) {
//                                     qDebug() << "SUCKER!" << sucker;
//                                     sucker->setPalette(pal);
//                                 }
//                             }
//                         }
//                         break;
//                     }
//                 }
//             }
        }
        /// Tab Transition animation,
        if (widget->inherits("QStackedWidget"))
            // NOTICE do NOT(!) apply this on tabs explicitly, as they contain a stack!
            Animator::Tab::manage(widget);
        else if (widget->inherits("KColorPatch"))
            widget->setAttribute(Qt::WA_NoMousePropagation);
        /// QToolBox handling - a shame they look that crap by default!
        else if (widget->inherits("QToolBox"))
        {   // get rid of QPalette::Button
            widget->setBackgroundRole(QPalette::Window);
            widget->setForegroundRole(QPalette::WindowText);
            if (widget->layout())
            {   // get rid of nasty indention
                widget->layout()->setContentsMargins(0,0,0,0);
                widget->layout()->setSpacing ( 0 );
            }
        }

        if (!frame->isWindow() && frame->frameShape() != QFrame::NoFrame && frame->lineWidth() < F(2))
            frame->setLineWidth(F(2));
    }
    //END FRAMES                                                                                   -

    //BEGIN PUSHBUTTONS - hovering/animation                                                       -
    else if (qobject_cast<QAbstractButton*>(widget))
    {
//         widget->setBackgroundRole(config.btn.std_role[Bg]);
//         widget->setForegroundRole(config.btn.std_role[Fg]);
        widget->setAttribute(Qt::WA_Hover, false); // KHtml
        if (widget->inherits("QToolBoxButton") || IS_HTML_WIDGET )
            widget->setAttribute(Qt::WA_Hover); // KHtml
        else
        {
            if (QPushButton *pbtn = qobject_cast<QPushButton*>(widget))
            {
                // HACK around "weird" original appearance ;-P
                // also see eventFilter
                if (pbtn->inherits("KUrlNavigatorButtonBase") || pbtn->inherits("BreadcrumbItemButton")) {
                    pbtn->setBackgroundRole(QPalette::Window);
                    pbtn->setForegroundRole(QPalette::Link);
                    QPalette pal = pbtn->palette();
                    pal.setColor(QPalette::Highlight, Qt::transparent);
                    pal.setColor(QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::Window));
                    pbtn->setPalette(pal);
                    pbtn->setCursor(Qt::PointingHandCursor);
                    FILTER_EVENTS(pbtn);
                    widget->setAttribute(Qt::WA_Hover);
                } else if (!qobject_cast<QCommandLinkButton*>(pbtn)) {
                    QFont fnt(widget->font());
                    fnt.setBold(true);
                    widget->setFont(fnt);
                }
            }
            else if (widget->inherits("QToolButton") &&
                // of course plasma needs - again - a WORKAROUND, we seem to be unable to use bg/fg-role, are we?
                !(appType == Plasma && widget->inherits("ToolButton")))
            {
                QPalette::ColorRole bg = QPalette::Window, fg = QPalette::WindowText;
                if (QWidget *dad = widget->parentWidget())
                {
                    bg = dad->backgroundRole();
                    fg = dad->foregroundRole();
                }
                widget->setBackgroundRole(bg);
                widget->setForegroundRole(fg);
                if (widget->inherits("BE::Button")) // I hover myself - QStyleSheetStyle breaks all animations :-(
                    widget->setAttribute(Qt::WA_Hover);
            }
            if (!widget->testAttribute(Qt::WA_Hover))
                Animator::Hover::manage(widget);
        }

        // NOTICE WORKAROUND - this widget uses the style to paint the bg, but hardcodes the fg...
        // TODO: inform Joseph Wenninger <jowenn@kde.org> and really fix this
        // (fails all styles w/ Windowcolored ToolBtn and QPalette::ButtonText != QPalette::WindowText settings)
        if (widget->inherits("KMultiTabBarTab"))
        {
            QPalette pal = widget->palette();
            pal.setColor(QPalette::Active, QPalette::Button, pal.color(QPalette::Active, QPalette::Window));
            pal.setColor(QPalette::Inactive, QPalette::Button, pal.color(QPalette::Inactive, QPalette::Window));
            pal.setColor(QPalette::Disabled, QPalette::Button, pal.color(QPalette::Disabled, QPalette::Window));

            pal.setColor(QPalette::Active, QPalette::ButtonText, pal.color(QPalette::Active, QPalette::WindowText));
            pal.setColor(QPalette::Inactive, QPalette::ButtonText, pal.color(QPalette::Inactive, QPalette::WindowText));
            pal.setColor(QPalette::Disabled, QPalette::ButtonText, pal.color(QPalette::Disabled, QPalette::WindowText));
            widget->setPalette(pal);
        }
    }

    //BEGIN COMBOBOXES - hovering/animation                                                        -
    else if (QComboBox *cb = qobject_cast<QComboBox*>(widget))
    {
        if (cb->view()) {
            cb->view()->setTextElideMode( Qt::ElideMiddle);
        }

        if (cb->parentWidget() && cb->parentWidget()->inherits("KUrlNavigator"))
            cb->setIconSize(QSize(0,0));

        if (IS_HTML_WIDGET)
            widget->setAttribute(Qt::WA_Hover);
        else {
            Animator::Hover::manage(widget);
            if (cb->isEditable())
                Animator::Focus::manage(widget);
        }
    }
    //BEGIN SLIDERS / SCROLLBARS / SCROLLAREAS - hovering/animation                                -
    else if (qobject_cast<QAbstractSlider*>(widget))
    {
        FILTER_EVENTS(widget); // finish animation

        widget->setAttribute(Qt::WA_Hover);
        // NOTICE
        // QAbstractSlider::setAttribute(Qt::WA_OpaquePaintEvent) saves surprisinlgy little CPU
        // so that'd just gonna add more complexity for literally nothing...
        // ...as the slider is usually not bound to e.g. a "scrollarea"
//         if ( appType == Amarok && widget->inherits("VolumeDial") )
//         {   // OMG - i'm hacking myself =D
//             QPalette pal = widget->palette();
//             pal.setColor( QPalette::Highlight, pal.color( QPalette::Active, QPalette::WindowText ) );
//             widget->setPalette( pal );
//         }
        if (widget->inherits("QScrollBar"))
        {
            // TODO: find a general catch for the plasma problem
            if (appType == Plasma) // yes - i currently don't know how to detect those things otherwise
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
            else
            {
                QWidget *dad = widget;
                while ((dad = dad->parentWidget()))
                {   // digg for a potential KHTMLView ancestor, making this a html input scroller
                    if (dad->inherits("KHTMLView"))
                    {   // NOTICE this slows down things as it triggers a repaint of the frame
                        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                        // ...but this would re-enbale speed - just: how to get the proper palette
                        // what if there's a bg image?
                        // TODO how's css/khtml policy on applying colors?
    //                     widget->setAutoFillBackground ( true );
    //                     widget->setBackgroundRole ( QPalette::Base ); // QPalette::Window looks wrong
    //                     widget->setForegroundRole ( QPalette::Text );
                        break;
                    }
                }
            }

            /// Scrollarea hovering - yes, this is /NOT/ redundant to the one above!
            if (QWidget *area = widget->parentWidget())
            {
                if ((area = area->parentWidget())) // sic!
                {
                    if (qobject_cast<QAbstractScrollArea*>(area))
                        area = 0; // this is handled for QAbstractScrollArea, but...
                    else // Konsole, Kate, etc. need a special handling!
                        area = widget->parentWidget();
                }
                if (area)
                    Animator::Hover::manage(area, true);
            }
        }
    }

    else if (qobject_cast<QLineEdit*>(widget))
        Animator::Focus::manage(widget);

    //BEGIN PROGRESSBARS - hover/animation and bold font                                           -
    else if (widget->inherits("QProgressBar"))
    {
        widget->setAttribute(Qt::WA_Hover);
        setBoldFont(widget);
        Animator::Progress::manage(widget);
    } else if ( widget->inherits( "QTabWidget" ) )
        FILTER_EVENTS(widget)

    //BEGIN Tab animation, painting override                                                       -
    else if (QTabBar *bar = qobject_cast<QTabBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
        if (bar->drawBase() && config.invert.headers) {
            widget->setBackgroundRole(QPalette::WindowText);
            widget->setForegroundRole(QPalette::Window);
        }
        QWidget *win = bar->window();
        if (win && win->inherits("DolphinMainWindow"))
            bar->setDocumentMode(true);
        bar->setExpanding(false);
        // the eventfilter overtakes the widget painting to allow tabs ABOVE the tabbar
        FILTER_EVENTS(widget);
    }
    else if ( QDockWidget *dock = qobject_cast<QDockWidget*>(widget) ) {
        if (config.invert.docks && dock->style() == this && (dock->window()->windowFlags() & Qt::Dialog) != Qt::Dialog) {
            if (QWidget *window = widget->window())
                window->setProperty("Virtuality.invertTitlebar", true);
            dock->setProperty("Virtuality.inverted", true);
            invertContainer(dock, invertedPalette);
            dock->setAutoFillBackground(true);
        }
        if ( Hacks::config.lockDocks ) {
            disconnect( dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)) );
            connect( dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)) );
        }
        dock->setContentsMargins(F(4),F(4),F(4),F(4));
        widget->setAttribute(Qt::WA_Hover);
        if (!(kStyleFeatureRequest & NoShadow))
            Shadows::manage(dock);
    }
    /// Menubars and toolbar default to QPalette::Button - looks crap and leads to flicker...?!
    else if (QMenuBar *mbar = qobject_cast<QMenuBar *>(widget)) {
        if (config.invert.menubars && qobject_cast<QMainWindow*>(mbar->parentWidget())) {
            if (QWidget *window = widget->window())
                window->setProperty("Virtuality.invertTitlebar", true);
            mbar->setProperty("Virtuality.inverted", true);
            invertContainer(mbar, invertedPalette);
            mbar->setAutoFillBackground(true);
        }
#ifndef QT_NO_DBUS
        if ( appType != KDevelop ) //&& !(appType == QtDesigner && mbar->inherits("QDesignerMenuBar")) )
            MacMenu::manage(mbar);
#endif
    }
    else if (widget->inherits("KFadeWidgetEffect"))
    {   // interfers with our animation, is slower and cannot handle non plain backgrounds
        // (unfortunately i cannot avoid the widget grabbing)
        // maybe ask ereslibre to query a stylehint for this?
        widget->hide();
        widget->installEventFilter(&eventKiller);
    }
    /// hover some leftover widgets
    else if (widget->inherits("QAbstractSpinBox") || widget->inherits("QSplitterHandle") ||
        widget->inherits("QWebView") || // to update the scrollbars
        widget->inherits("QWorkspaceTitleBar") ||
        widget->inherits("Q3DockWindowResizeHandle"))
    {
        widget->setAttribute(Qt::WA_Hover);
        if (widget->inherits("QWebView"))
            FILTER_EVENTS(widget)
        else if (widget->inherits("QAbstractSpinBox"))
            Animator::Focus::manage(widget);
    }
    // this is a WORKAROUND for amarok filebrowser, see above on itemviews...
    else if (widget->inherits("KDirOperator"))
    {
        if (widget->parentWidget() && widget->parentWidget()->inherits("FileBrowser"))
        {
            QPalette pal = widget->palette();
            pal.setColor(QPalette::Active, QPalette::Text, pal.color(QPalette::Active, QPalette::WindowText));
            pal.setColor(QPalette::Inactive, QPalette::Text, pal.color(QPalette::Inactive, QPalette::WindowText));
            pal.setColor(QPalette::Disabled, QPalette::Text, pal.color(QPalette::Disabled, QPalette::WindowText));
            widget->setPalette(pal);
        }
    } else if (widget->inherits("Marble::MarbleWidget")) {
        widget->setAttribute(Qt::WA_NoMousePropagation, true);
    }
#if 0
// #ifdef BE_WS_X11
    if ( config.bg.opacity != 0xff && /*widget->window() &&*/
        (widget->inherits("MplayerWindow") ||
         widget->inherits("KSWidget") ||
         widget->inherits("QX11EmbedContainer") ||
         widget->inherits("QX11EmbedWidget") ||
         widget->inherits("Phonon::VideoWidget")) )
    {
        bool vis = widget->isVisible();
        widget->setWindowFlags(Qt::Window);
        widget->show();
        printf("%s %s %d\n", widget->className(), widget->parentWidget()->className(), widget->winId());
        widget->setAttribute(Qt::WA_DontCreateNativeAncestors, widget->testAttribute(Qt::WA_DontCreateNativeAncestors));
        widget->setAttribute(Qt::WA_NativeWindow);
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        widget->setAttribute(Qt::WA_PaintOnScreen, true);
        widget->setAttribute(Qt::WA_NoSystemBackground, false);
        if (QWidget *window = widget->window())
        {
            qDebug() << "BESPIN, reverting" << widget << window;
            window->setAttribute(Qt::WA_TranslucentBackground, false);
            window->setAttribute(Qt::WA_NoSystemBackground, false);
        }
        QApplication::setColorSpec(QApplication::NormalColor);
    }
#endif

//     const bool isNavigationBar = widget->inherits("NavigationBar");
    const bool isTopContainer = qobject_cast<QToolBar *>(widget)/* || isNavigationBar*/;
    if (isTopContainer && config.invert.toolbars)
    if (QWidget *window = widget->window())
    if (qobject_cast<QMainWindow*>(window)) {
        widget->setProperty("Virtuality.inverted", true);
        window->setProperty("Virtuality.invertTitlebar", true);
        widget->setAutoFillBackground(true);
        invertContainer(widget, invertedPalette);
    }

    // Arora needs a separator between the buttons and the lineedit - looks megadull w/ shaped buttons otherwise :-(
    if ( appType == Arora && isTopContainer && widget->objectName() == "NavigationToolBar")
    {
        QAction *before = 0;
        QToolBar *bar = static_cast<QToolBar *>(widget);
        foreach ( QObject *o, bar->children() )
        {
            before = 0;
            if ( o->inherits("QWidgetAction") && bar->widgetForAction( (before = static_cast<QAction*>(o)) )->inherits("QSplitter") )
                break;
        }
        if ( before )
            bar->insertSeparator( before );
    }

    if (isTopContainer || qobject_cast<QToolBar*>(widget->parent()))
    {
        widget->setBackgroundRole(QPalette::Window);
        widget->setForegroundRole(QPalette::WindowText);
        if (!isTopContainer && widget->inherits("QToolBarHandle"))
            widget->setAttribute(Qt::WA_Hover);
    }

    const bool isDolphinStatusBar = widget->inherits("DolphinStatusBar");
    if (isDolphinStatusBar) {
        QMargins marg = widget->contentsMargins();
        int m[4] = {marg.left(), marg.top(), marg.right(), marg.bottom()};
        for (int i = 0; i < 4; ++i)
            m[i] = qMax(m[i], F(2));
        widget->setContentsMargins(m[0], m[1], m[2], m[3]);
    }
    if ((config.invert.toolbars||config.invert.titlebars) && (!isDolphinStatusBar || config.invert.docks) &&
        (isDolphinStatusBar || widget->inherits("QStatusBar") || widget->inherits("KStatusBar") ||
         widget->inherits("KonqFrameStatusBar") || widget->inherits("KonqStatusBarMessageLabel"))) {
        widget->setAutoFillBackground(true);
        widget->setProperty("Virtuality.inverted", true);
        invertContainer(widget, invertedPalette);
    }

    /// this is for QToolBox kids - they're autofilled by default - what looks crap
    if (widget->autoFillBackground() && widget->parentWidget() &&
        ( widget->parentWidget()->objectName() == "qt_scrollarea_viewport" ) &&
        widget->parentWidget()->parentWidget() && //grampa
        qobject_cast<QAbstractScrollArea*>(widget->parentWidget()->parentWidget()) &&
        widget->parentWidget()->parentWidget()->parentWidget() && // grangrampa
        widget->parentWidget()->parentWidget()->parentWidget()->inherits("QToolBox") )
    {
        widget->parentWidget()->setAutoFillBackground(false);
        widget->setAutoFillBackground(false);
    }

    /// KHtml css colors can easily get messed up, either because i'm unsure about what colors
    /// are set or KHtml does wrong OR (mainly) by html "designers"
    if (IS_HTML_WIDGET)
    {   // the eventfilter watches palette changes and ensures contrasted foregrounds...
        FILTER_EVENTS(widget);
        QEvent ev(QEvent::PaletteChange);
        eventFilter(widget, &ev);
    }

    /// QTextDocument will *often* come with some dark colored through the CSS and no particular background
    /// so if we've a dark Text role, we're printing black on black
    /// should not happen and is part of WORKAROUNDs for "lousy dark theme support everywhere"
    if (FX::value(widget->palette().color(QPalette::Active, QPalette::Base)) < 128) {
        if (QTextEdit *edit = qobject_cast<QTextEdit*>(widget)) {
            if (edit->document()) {
                if (edit->autoFillBackground())
                    edit->document()->setDefaultStyleSheet( QString("*{color:%1;background-color:%2;}a{color:%3;}").arg(edit->palette().color(QPalette::Active, QPalette::Text).name()).arg(edit->palette().color(QPalette::Active, QPalette::Base).name()).arg(edit->palette().color(QPalette::Active, QPalette::Link).name()) );
                else
                    edit->document()->setDefaultStyleSheet( QString("*{color:%1;}a{color:%2;}").arg(edit->palette().color(QPalette::Active, QPalette::WindowText).name()).arg(edit->palette().color(QPalette::Active, QPalette::Link).name()) );

                if (!edit->document()->isEmpty()) {
                    if (QTextBrowser *browser = qobject_cast<QTextBrowser*>(edit)) {
                        if (browser->source().isValid())
                            browser->reload();
//                     } else {
//                         const QString html(edit->document()->toHtml());
//                         edit->document()->setHtml(QString());
//                         edit->document()->setHtml(html);
                    }
                }
            }
        }
    }

    if ( appType == KMail )
    {   // This no only has not been fixed in 4 years now, but there's even a new widget with this flag set: head -> desk!
        if (widget->inherits("KMail::MessageListView::Widget"))
            widget->setAutoFillBackground(false);
        else if ( widget->parentWidget() && widget->inherits("RecipientLine") )
            widget->parentWidget()->setAutoFillBackground(false);
    }

    if ( appType == KDM )
    {
        if (QLineEdit *le = qobject_cast<QLineEdit*>(widget))
            le->setAlignment(Qt::AlignCenter);
    }

}
#undef PAL

void
Style::unpolish( QApplication *app )
{
    app->removeEventFilter(this);
    app->setPalette(QPalette());
    Hacks::releaseApp();
}

void
Style::unpolish( QWidget *widget )
{
    if (!widget)
        return;

    if (widget->isWindow() && widget->testAttribute(Qt::WA_WState_Created) && widget->internalWinId())
    {
//         if (config.bg.opacity != 0xff)
//         {
//             window->setAttribute(Qt::WA_TranslucentBackground, false);
//             window->setAttribute(Qt::WA_NoSystemBackground, false);
//         }
#ifdef BE_WS_X11
        if (BE::isPlatformX11()) { // TODO: port XProperty for wayland
            BE::XProperty::remove(widget->winId(), BE::XProperty::winData);
            BE::XProperty::remove(widget->winId(), BE::XProperty::bgPics);
        }
#endif
        if (qobject_cast<QMenu *>(widget))
            widget->clearMask();
    }

    if (qobject_cast<QAbstractButton*>(widget) || qobject_cast<QToolBar*>(widget) ||
        qobject_cast<QMenuBar*>(widget) || qobject_cast<QMenu*>(widget) ||
        widget->inherits("QToolBox"))
    {
        widget->setBackgroundRole(QPalette::Button);
        widget->setForegroundRole(QPalette::ButtonText);
    }
#ifndef QT_NO_DBUS
    if (QMenuBar *mbar = qobject_cast<QMenuBar *>(widget))
        MacMenu::release(mbar);
#endif

    Animator::Hover::release(widget);
    Animator::Progress::release(widget);
    Animator::Tab::release(widget);
    Hacks::remove(widget);

    widget->removeEventFilter(this);
}

// X11 properties for the deco ---------------
#ifndef QT_NO_DBUS
#define MSG(_FNC_) QDBusMessage::createMethodCall( "org.kde.kwin", "/BespinDeco", "org.kde.BespinDeco", _FNC_ )
#define KWIN_SEND( _MSG_ ) QDBusConnection::sessionBus().send( _MSG_ )
#else
#define MSG(_FNC_) void(0)
#define KWIN_SEND( _MSG_ ) void(0)
#endif

void
Style::setupDecoFor(QWidget *widget, const QPalette &palette)
{
#ifdef BE_WS_X11
    if (!BE::isPlatformX11())
        return; // TODO: port XProperty for wayland
    if ((appType == KWin))
        return;

    // WORKAROUND the raster graphicssystem destructor & some special virtual widget
    // as we now only set this on the show event, this should not occur anyway, but let's keep it safe
    if (widget && !(widget->testAttribute(Qt::WA_WState_Created) || widget->internalWinId())) {
        //NOTICE esp. when using the raster engine some "virtual" widgets segfault in their destructor
        // as the winId() call below will unexpectedly generate a native window
        // known offending widgets:
        // inherits("KXMessages"), inherits("KSelectionWatcher::Private") inherits("KSelectionOwner::Private")
        // unfortunately the latter two are internal (and thus don't propagate their class through moc)
        qDebug() << "BESPIN: Not exporting decoration hints for " << widget;
        return;
    }

    // this is important because KDE apps may alter the original palette any time
    bool invert = (config.invert.titlebars && widget->property("Virtuality.invertTitlebar").toBool()) ||
                   widget->property("BE.swappedPalette").toBool();
    const QPalette &pal = invert ? invertedPalette : (originalPalette ? *originalPalette : palette);

    // the title region in the center
    WindowData data;

    // MODE ======================================

    QColor bg = pal.color(QPalette::Inactive, QPalette::Window);

    // STYLE ===================================
    {
        int mMode = Plain;
        data.style = (0 << 24) | ((mMode & 0xff) << 16);
    }

    data.inactiveWindow = bg.rgba();

    // COLORS =======================
    bg = pal.color(QPalette::Active, QPalette::Window);

    data.activeWindow = bg.rgba();

    data.inactiveDeco   = pal.color(QPalette::Inactive, QPalette::Window).rgba();
    data.activeDeco     = pal.color(QPalette::Active, QPalette::Window).rgba();
    data.inactiveText   = FX::blend(pal.color(QPalette::Inactive, QPalette::Window),
                                               pal.color(QPalette::Inactive, QPalette::WindowText)).rgba();
    data.activeText     = pal.color(QPalette::Active, QPalette::Highlight).rgba();
    data.inactiveButton = FX::blend(pal.color(QPalette::Inactive, QPalette::Window),
                                              pal.color(QPalette::Inactive, QPalette::WindowText),3,2).rgba();
    data.activeButton   = pal.color(QPalette::Active, QPalette::Highlight).rgba();
    if (!FX::haveContrast(bg, data.activeText))
        data.activeButton = data.activeText = pal.color(QPalette::Active, QPalette::WindowText).rgba();

    if (widget) {
        WId id = widget->winId();
        BE::XProperty::set<uint>(id, BE::XProperty::winData, (uint*)&data, BE::XProperty::WORD, 9);
        //         XSync(QX11Info::display(), False);
        KWIN_SEND( MSG("updateDeco") << (uint)id );
    } else {   // dbus solution, currently for gtk
        QByteArray ba(36, 'a');
        uint *ints = (uint*)ba.data();
        ints[0] = data.inactiveWindow;
        ints[1] = data.activeWindow;
        ints[2] = data.inactiveDeco;
        ints[3] = data.activeDeco;
        ints[4] = data.inactiveText;
        ints[5] = data.activeText;
        ints[6] = data.inactiveButton;
        ints[7] = data.activeButton;
        ints[8] = data.style;
        
        const qint64 pid = QCoreApplication::applicationPid();
        KWIN_SEND( MSG("styleByPid") << pid << ba );
    }
#endif // X11
}

#undef MSG
#undef KWIN_SEND

#undef CCOLOR
#undef FCOLOR
#undef FILTER_EVENTS
