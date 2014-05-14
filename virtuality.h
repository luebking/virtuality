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

#ifndef BE_STYLE_H
#define BE_STYLE_H

class QAbstractItemView;
class QSettings;
class QStyleOptionToolButton;
class QToolBar;

#include <QCommonStyle>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QStyleOptionMenuItem>
#include <QStylePlugin>

#include "dpi.h"
#include "types.h"
#include "config.h"
#include "debug.h"

#ifndef Q_WS_X11
#define QT_NO_XRENDER 1
#endif

namespace BE
{

#ifdef Q_WS_X11
typedef struct _WindowData WindowData;
#endif

class EventKiller : public QObject { public: bool eventFilter( QObject *, QEvent *) { return true; } };

enum KStyleFeatureRequest { Shadow = 1<<0, NoShadow = 1<<1, Blur = 1<<2, NoBlur = 1<<3, NoARGB = 1<<4, DragWidget = 1<<5, NoDragWidget = 1<<6 };

class
#ifndef Q_WS_WIN
Q_GUI_EXPORT
#endif
Style : public QCommonStyle
{
    Q_OBJECT
    Q_CLASSINFO ("X-KDE-CustomElements", "true")

public:
    enum ColorRole { Bg = 0, Fg = 1 };
    enum WindowDecoration { Shadowed = 1, Rounded = 2 };

    Style(const QString &name);
    ~Style();

    //inheritance from QStyle
    void drawComplexControl ( ComplexControl control, const QStyleOptionComplex * option,
                                QPainter * painter, const QWidget * widget = 0 ) const;

    void drawControl ( ControlElement element, const QStyleOption * option, QPainter * painter,
                        const QWidget * widget = 0 ) const;

//    virtual void drawItemPixmap ( QPainter * painter, const QRect & rect, int alignment, const QPixmap & pixmap ) const;

    void drawItemText(QPainter*, const QRect&, int alignment, const QPalette&, bool enabled,
                      const QString &text, QPalette::ColorRole textRole, QRect *boundingRect) const;
    inline void drawItemText(QPainter *p, const QRect &r, int alignment, const QPalette &pal,
                             bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const
    { drawItemText(p, r, alignment, pal, enabled, text, textRole, NULL); }

    void drawPrimitive (PrimitiveElement elem, const QStyleOption *opt, QPainter *p, const QWidget *w = 0 ) const;

    QPixmap standardPixmap(StandardPixmap stdPix, const QStyleOption *opt = 0, const QWidget *w = 0 ) const;
//     QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;

//    what do they do? ========================================

//    SubControl hitTestComplexControl ( ComplexControl control,
//                                       const QStyleOptionComplex * option,
//                                       const QPoint & pos,
//                                       const QWidget * widget = 0 ) const;
//    QRect itemPixmapRect ( const QRect & rect,
//                           int alignment,
//                           const QPixmap & pixmap ) const;
//    QRect itemTextRect ( const QFontMetrics & metrics,
//                         const QRect & rect,
//                         int alignment,
//                         bool enabled,
//                         const QString & text ) const;
//=============================================================

    int pixelMetric ( PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const;

    void polish(QWidget *w);
    void polish(QApplication *a);
    void polish(QPalette &pal, bool onInit);
    inline void polish(QPalette &pal) { polish(pal, true); }

    QSize sizeFromContents ( ContentsType type, const QStyleOption * option,
                                const QSize & contentsSize, const QWidget * widget = 0 ) const;

    int styleHint ( StyleHint hint, const QStyleOption * option = 0, const QWidget * widget = 0,
                    QStyleHintReturn * returnData = 0 ) const;

    QRect subControlRect ( ComplexControl control, const QStyleOptionComplex * option,
                            SubControl subControl, const QWidget * widget = 0 ) const;

    QRect subElementRect ( SubElement element, const QStyleOption * option,
                            const QWidget * widget = 0 ) const;

    QPalette standardPalette () const;

    void unpolish( QWidget *w );
    void unpolish( QApplication *a );

    // from QObject
    bool eventFilter( QObject *object, QEvent *event );

    // STATICS
    static void drawArrow(Navi::Direction, const QRect&, QPainter*, const QWidget *w = 0);
    static void drawSolidArrow(Navi::Direction, const QRect&, QPainter*, const QWidget *w = 0);

    static bool serverSupportsShadows();

protected:
    virtual void init(const QSettings *settings = 0L);

    // element painting routines ===============
    void skip(const QStyleOption*, QPainter*, const QWidget*) const {}
    // buttons.cpp
    void drawButtonFrame(const QStyleOption *o, QPainter *p, const QWidget *w, int animStep) const;
    inline void drawButtonFrame(const QStyleOption *o, QPainter *p, const QWidget *w) const
    { drawButtonFrame(o, p, w, -1); }
    void drawPushButton(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawPushButtonBevel(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawPushButtonLabel(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawRadioOrCheckBox(const QStyleOption*, QPainter*, const QWidget*, bool) const;
    inline void drawCheckBox(const QStyleOption *o, QPainter *p, const QWidget *w) const {
        drawRadioOrCheckBox(o, p, w, false);
    }
    inline void drawRadio(const QStyleOption *o, QPainter *p, const QWidget *w) const {
        drawRadioOrCheckBox(o, p, w, true);
    }
    void drawCheckBoxItem(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawRadioItem(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawCheckLabel(const QStyleOption*, QPainter*, const QWidget*) const;
    // docks.cpp
    void drawDockBg(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawDockTitle(const QStyleOption*, QPainter*, const QWidget*) const;
    inline void drawDockHandle(const QStyleOption *o, QPainter *p, const QWidget *w) const
    { drawHandle(o, p, w, config.invert.docks); }
    inline void drawToolbarHandle(const QStyleOption *o, QPainter *p, const QWidget *w) const
    { drawHandle(o, p, w, config.invert.toolbars); }
    inline void drawSplitterHandle(const QStyleOption *o, QPainter *p, const QWidget *w) const
    { drawHandle(o, p, w, false); }
    void drawHandle(const QStyleOption*, QPainter*, const QWidget*, bool) const;
    void drawMDIControls(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    // frames.cpp
    void drawFocusFrame(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawFrame(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawGroupBox(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    void drawGroupBoxFrame(const QStyleOption*, QPainter*, const QWidget*) const;
    // input.cpp
    void drawLineEditFrame(const QStyleOption*, QPainter*, const QWidget*) const;
    inline void drawLineEdit(const QStyleOption *opt, QPainter *p, const QWidget *w) const
    { drawLineEdit(opt, p, w, false); }
    void drawLineEdit(const QStyleOption*, QPainter*, const QWidget*, bool round) const;
    void drawSpinBox(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    void drawComboBox(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    void drawComboBoxLabel(const QStyleOption*, QPainter*, const QWidget*) const;
    // menus.cpp
    void drawMenuBarItem(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawMenuItem(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawMenuScroller(const QStyleOption*, QPainter*, const QWidget*) const;
    // progress.cpp
    void drawCapacityBar(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawSimpleProgress(const QStyleOptionProgressBar*, QPainter*, const QWidget*, bool isListView) const;
    void drawProgressBar(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawProgressBarGC(const QStyleOption*, QPainter*, const QWidget*, bool) const;
    inline void
    drawProgressBarGroove(const QStyleOption * option, QPainter * painter, const QWidget * widget) const
    { drawProgressBarGC(option, painter, widget, false); }
    inline void
    drawProgressBarContents(const QStyleOption * option, QPainter * painter, const QWidget * widget) const
    { drawProgressBarGC(option, painter, widget, true); }

    void drawProgressBarLabel(const QStyleOption*, QPainter*, const QWidget*) const;
    // scrollareas.cpp
    void drawScrollAreaCorner(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawScrollBar(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    void drawScrollBarButton(const QStyleOption*, QPainter*, const QWidget* , bool) const;
    inline void
    drawScrollBarAddLine(const QStyleOption * option, QPainter * painter, const QWidget * widget) const
    { drawScrollBarButton(option, painter, widget, false); }

    inline void
    drawScrollBarSubLine(const QStyleOption * option, QPainter * painter, const QWidget * widget) const
    { drawScrollBarButton(option, painter, widget, true); }
    void drawScrollBarGroove(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawScrollBarSlider(const QStyleOption*, QPainter*, const QWidget*) const;

    // shapes.cpp
    void drawCheck(const QStyleOption*, QPainter*, const QWidget*, bool exclusive, bool itemview) const;

#define INDI_ARROW(_D_)\
    inline void\
    drawSolidArrow##_D_(const QStyleOption * option, QPainter * painter, const QWidget *w) const\
    {\
        const int dx = option->rect.width()/8, dy = option->rect.height()/8;\
        drawSolidArrow(Navi::_D_, option->rect.adjusted(dx,dy,-dx,-dy), painter, w);\
    }
    INDI_ARROW(N) INDI_ARROW(S) INDI_ARROW(E) INDI_ARROW(W)
#undef INDI_ARROW

    inline void drawExclusiveCheck(const QStyleOption *o, QPainter *p, const QWidget *w) const
        { drawCheck(o, p, w, true, false); }

    inline void drawItemCheck(const QStyleOption *o, QPainter *p, const QWidget *w) const
        { drawCheck(o, p, w, false, true); }

    inline void drawMenuCheck(const QStyleOption *o, QPainter *p, const QWidget *w) const
        { drawCheck(o, p, w, false, false); }

    // slider.cpp
    void drawSlider(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    void drawDial(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    // tabbing.cpp
    void calcAnimStep(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawTabWidget(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawTabBar(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawTab(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawTabShape(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawTabLabel(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawToolboxTab(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawToolboxTabShape(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawToolboxTabLabel(const QStyleOption*, QPainter*, const QWidget*) const;
#if QT_VERSION >= 0x040500
    void drawTabCloser(const QStyleOption*, QPainter*, const QWidget*) const;
#endif
    // toolbars.cpp
    void drawToolButton(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    void drawToolButtonLabel(const QStyleOption*, QPainter*, const QWidget*) const;
    // views.cpp
    void drawHeader(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawHeaderSection(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawHeaderLabel(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawBranch(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawRubberBand(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawHeaderArrow(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawItem(const QStyleOption*, QPainter*, const QWidget*, bool isItem) const;
    inline void
    drawItemRow(const QStyleOption *o, QPainter *p, const QWidget *w) const { drawItem(o, p, w, false); }
    inline void
    drawItemItem(const QStyleOption *o, QPainter *p, const QWidget *w) const { drawItem(o, p, w, true); }
    // window.cpp
    void drawWindowFrame(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawWindowBg(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawToolTip(const QStyleOption*, QPainter*, const QWidget*) const;
    void drawTitleBar(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
    void drawSizeGrip(const QStyleOption*, QPainter*, const QWidget*) const;
    // ==========================================
    void fillWithMask(QPainter *painter, const QPoint &xy, const QBrush &brush, const QPixmap &mask, QPoint offset = QPoint()) const;
//private slots:
//   void fakeMouse();

private:
    Q_DISABLE_COPY(Style)
    void drawSliderHandle(const QRect &, const QStyleOption *, QPainter *, int step) const;
    int elementId(const QString &string) const;
    void erase(const QStyleOption*, QPainter*, const QWidget*, const QPoint *off = 0) const;
    static void fixViewPalette(QAbstractItemView *itemView, int style, bool alternate, bool silent = false);
    static bool hasMenuIndicator(const QStyleOptionToolButton *tb);
    void initMetrics();
    QColor mapFadeColor(const QColor &color, int index) const;
    void readSettings(const QSettings *settings = 0L, QString appName = QString());
    void registerRoutines();
    void setupDecoFor(QWidget *w, const QPalette &pal);
    enum PainterStorage { Pen = 1<<0, Brush = 1<<1, Alias = 1<<2, Font = 1<<3 };
    static int savePainter(QPainter*, int);
    static void restorePainter(QPainter*, int);
    #define SAVE_PAINTER(_F_) int storedPainterFlags = savePainter(painter, _F_)
    #define RESTORE_PAINTER restorePainter(painter, storedPainterFlags);
private:
    // gtk-qt and other workarounds
    static AppType appType;
    // KDE palette fix..
    static QPalette *originalPalette;
    static QPalette invertedPalette;
    static EventKiller eventKiller;
public:
    static Config config;
private slots:
    void clearScrollbarCache();
    void dockLocationChanged( Qt::DockWidgetArea );
    void removeAppEventFilter();
    void resetRingPix();
    void unlockDocks(bool);
    void updateBlurRegions() const;
};

} // namespace BE

class VirtualityStylePlugin : public QStylePlugin
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "virtuality.json")
#endif
public:
    QStringList keys() const;
    QStyle *create(const QString &key);
};

#endif //BE_STYLE_H
