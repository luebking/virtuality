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

/**================== Qt4 includes ======================*/

#include <QAbstractScrollArea>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QListView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QStyleOptionTabWidgetFrame>
#include <QStylePlugin>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QWeakPointer>

/**============= Bespin includes ==========================*/

// #include "debug.h"

#ifdef BE_WS_X11
#include "xproperty.h"
#include "fixx11h.h"
#endif
#include "FX.h"
#include "animator/hover.h"
#include "shadows.h"
#include "hacks.h"
#include "virtuality.h"

#include <QtDebug>

/**============ Internal Definitions ========================*/

#define BESPIN_MOUSE_DEBUG 0

/**=========================================================*/

/**============= extern C stuff ==========================*/

QStringList VirtualityStylePlugin::keys() const {
    return QStringList() << "Virtuality" << "Sienar" << "Flynn" << "VirtualBreeze" << "VirtualArch";
}

QStyle *VirtualityStylePlugin::create(const QString &key) {
    QString ikey = key.toLower();
    if (ikey == "virtuality" || ikey == "sienar" || ikey == "flynn" || ikey == "virtualbreeze" || 
        ikey == "virtualarch")
        return new BE::Style(ikey);
    return 0;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Virtuality, VirtualityStylePlugin)
#endif
/**=========================================================*/


using namespace BE;

AppType Style::appType;
Config Style::config;
float Style::halfStroke = 1;
QPalette *Style::originalPalette = 0;
QPalette Style::invertedPalette;
EventKiller Style::eventKiller;

#define N_PE 54
#define N_CE 50
#define N_CC 12
static void
(Style::*primitiveRoutine[N_PE])(const QStyleOption*, QPainter*, const QWidget*) const;
static void
(Style::*controlRoutine[N_CE])(const QStyleOption*, QPainter*, const QWidget*) const;
static void
(Style::*complexRoutine[N_CC])(const QStyleOptionComplex*, QPainter*, const QWidget*) const;

#define registerPE(_FUNC_, _ELEM_) primitiveRoutine[QStyle::_ELEM_] = &Style::_FUNC_
#define registerCE(_FUNC_, _ELEM_) controlRoutine[QStyle::_ELEM_] = &Style::_FUNC_
#define registerCC(_FUNC_, _ELEM_) complexRoutine[QStyle::_ELEM_] = &Style::_FUNC_

// static void registerPE(char *S0, ...)
// {
//    register char *s;
//    if (s=S0, s!=NULL)  { register char *sa;
//       va_list a;
//       for (va_start(a,S0);  (sa=va_arg(a,char*), sa!=NULL);  )
//          while (*s=*sa, *sa)  ++s,++sa;
//       va_end(a);
//    }
//    return ((int)(s-S0));
// }

void
Style::registerRoutines()
{
    for (int i = 0; i < N_PE; ++i)
        primitiveRoutine[i] = 0;
    for (int i = 0; i < N_CE; ++i)
        controlRoutine[i] = 0;
    for (int i = 0; i < N_CC; ++i)
        complexRoutine[i] = 0;

    // buttons.cpp
    registerPE(drawButtonFrame, PE_PanelButtonCommand);
    registerPE(drawButtonFrame, PE_PanelButtonBevel);
    registerPE(skip, PE_FrameDefaultButton);
    registerCE(drawPushButton, CE_PushButton);
    registerCE(drawPushButtonBevel, CE_PushButtonBevel);
    registerCE(drawPushButtonLabel, CE_PushButtonLabel);
    registerPE(drawCheckBox, PE_IndicatorCheckBox);
    registerPE(drawRadio, PE_IndicatorRadioButton);
    registerCE(drawCheckBoxItem, CE_CheckBox);
    registerCE(drawRadioItem, CE_RadioButton);
    registerCE(drawCheckLabel, CE_CheckBoxLabel);
    registerCE(drawCheckLabel, CE_RadioButtonLabel);
    // docks.cpp
    registerPE(drawDockBg, PE_FrameDockWidget);
    registerCE(drawDockTitle, CE_DockWidgetTitle);
    registerCC(drawMDIControls, CC_MdiControls);
    registerPE(drawDockHandle, PE_IndicatorDockWidgetResizeHandle);
    registerPE(drawToolbarHandle, PE_IndicatorToolBarHandle);
    // frames.cpp
    registerCE(skip, CE_FocusFrame);
    registerPE(skip, PE_PanelStatusBar);
    registerPE(skip, PE_FrameStatusBarItem);
    registerPE(drawFocusFrame, PE_FrameFocusRect);
    registerPE(drawFrame, PE_Frame);
    registerCE(drawFrame, CE_ShapedFrame);
    registerCC(drawGroupBox, CC_GroupBox);
    registerPE(drawGroupBoxFrame, PE_FrameGroupBox);
    // input.cpp
    registerPE(drawLineEditFrame, PE_FrameLineEdit);
    registerPE(drawLineEdit, PE_PanelLineEdit);
    registerCC(drawSpinBox, CC_SpinBox);
    registerCC(drawComboBox, CC_ComboBox);
    registerCE(drawComboBoxLabel, CE_ComboBoxLabel);
    // menus.cpp
    registerPE(skip, PE_PanelMenuBar);
    registerCE(skip, CE_MenuBarEmptyArea);
    registerCE(drawMenuBarItem, CE_MenuBarItem);
    registerCE(drawMenuItem, CE_MenuItem);
    registerCE(drawMenuScroller, CE_MenuScroller);
    registerCE(drawMenuTearOff, CE_MenuTearoff);
    registerCE(skip, CE_MenuEmptyArea);
    registerCE(skip, CE_MenuHMargin);
    registerCE(skip, CE_MenuVMargin);
    // progress.cpp
    registerCE(drawProgressBar, CE_ProgressBar);
    registerCE(drawProgressBarGroove, CE_ProgressBarGroove);
    registerCE(drawProgressBarContents, CE_ProgressBarContents);
    registerCE(drawProgressBarLabel, CE_ProgressBarLabel);
    // scrollareas.cpp
    registerPE(drawScrollAreaCorner, PE_PanelScrollAreaCorner);
    registerCC(drawScrollBar, CC_ScrollBar);
    registerCE(drawScrollBarAddLine, CE_ScrollBarAddLine);
    registerCE(drawScrollBarSubLine, CE_ScrollBarSubLine);
    registerCE(skip, CE_ScrollBarSubPage);
    registerCE(skip, CE_ScrollBarAddPage);
    registerCE(drawScrollBarSlider, CE_ScrollBarSlider);
    // indicators.cpp
    registerPE(drawItemCheck, PE_IndicatorItemViewItemCheck);
    registerPE(drawMenuCheck, PE_IndicatorMenuCheckMark);
    registerPE(drawSolidArrowN, PE_IndicatorArrowUp);
    registerPE(drawSolidArrowN, PE_IndicatorSpinUp);
    registerPE(drawSolidArrowN, PE_IndicatorSpinPlus);
    registerPE(drawSolidArrowS, PE_IndicatorArrowDown);
    registerPE(drawSolidArrowS, PE_IndicatorSpinDown);
    registerPE(drawSolidArrowS, PE_IndicatorSpinMinus);
    registerPE(drawSolidArrowS, PE_IndicatorButtonDropDown);
    registerPE(drawSolidArrowE, PE_IndicatorArrowRight);
    registerPE(drawSolidArrowW, PE_IndicatorArrowLeft);
    // slider.cpp
    registerCC(drawSlider, CC_Slider);
    registerCC(drawDial, CC_Dial);
    // tabbing.cpp
    registerPE(drawTabWidget, PE_FrameTabWidget);
    registerPE(drawTabBar, PE_FrameTabBarBase);
    registerCE(drawTab, CE_TabBarTab);
    registerCE(drawTabShape, CE_TabBarTabShape);
    registerCE(drawTabLabel, CE_TabBarTabLabel);
    registerPE(skip, PE_IndicatorTabTear);
    registerCE(drawToolboxTab, CE_ToolBoxTab);
    registerCE(drawToolboxTabShape, CE_ToolBoxTabShape);
    registerCE(drawToolboxTabLabel, CE_ToolBoxTabLabel);
    registerPE(drawTabCloser, PE_IndicatorTabClose);

    // toolbars.cpp
    registerCC(drawToolButton, CC_ToolButton);
    registerPE(skip, PE_PanelButtonTool);
    registerPE(skip, PE_IndicatorToolBarSeparator);
    registerPE(skip, PE_PanelToolBar);
    registerCE(skip, CE_ToolBar);
    registerCE(drawToolButtonLabel, CE_ToolButtonLabel);
    registerPE(skip, PE_FrameButtonTool);
    // views.cpp
    registerCE(drawHeader, CE_Header);
    registerCE(drawHeaderSection, CE_HeaderSection);
    registerCE(drawHeaderLabel, CE_HeaderLabel);
    registerPE(drawBranch, PE_IndicatorBranch);
    registerCE(drawRubberBand, CE_RubberBand);
    registerPE(drawHeaderArrow, PE_IndicatorHeaderArrow);
    registerPE(drawItemRow, PE_PanelItemViewRow);
    registerPE(drawItemItem, PE_PanelItemViewItem);
    //NOTICE this is a hell lot of code in QCommonStyle, so for the moment, it's not customized
//     registerCE(drawViewItem, CE_ItemViewItem);

    // window.cpp
    registerPE(drawWindowFrame, PE_FrameWindow);
    registerPE(skip, PE_FrameMenu);
    registerPE(drawWindowBg, PE_Widget);
    registerPE(drawToolTip, PE_PanelTipLabel);
    registerCC(drawTitleBar, CC_TitleBar);
    registerCE(drawSplitterHandle, CE_Splitter);
    registerCE(drawSizeGrip, CE_SizeGrip);
}

/**THE STYLE ITSELF*/

Style::Style(const QString &name) : QCommonStyle()
{
    m_usingStandardPalette = (name == "sienar" || name == "flynn" || name == "virtualbreeze");
    setObjectName(name);
#ifdef BE_WS_X11
    if (BE::isPlatformX11()) // TODO: port XProperty for wayland
        BE::XProperty::init();
#endif
    FX::init();
    if (m_usingStandardPalette) {
        originalPalette = new QPalette(standardPalette());
        polish(*originalPalette, true);
        static bool isRecursion = false;
        if (!isRecursion) {
            isRecursion = true; // some applications (TEA editor at least) are aggressive on forcing
                                // palettes. Too aggressive, by creating a new style everytiem. We
                                // therefore detect this and escape the recursion.
                                // TEA looks pretty ... interesting ... anyway ;-P
            QApplication::setPalette(*originalPalette);
            qApp->installEventFilter(this);
            QTimer::singleShot(10000, this, SLOT(removeAppEventFilter()));
            isRecursion = false;
        }
    }
    init();
    registerRoutines();
}

Style::~Style()
{
    Shadows::cleanUp();
    // reset palette
    if (!m_usingStandardPalette)
        return;
    QStyle *newStyle = QApplication::style();
    // qobject_cast because this can be invoked from a dying proxystyle
    if (!newStyle || newStyle == this || !qobject_cast<QStyle*>(newStyle))
        return;
    BE::Style *beStyle = qobject_cast<BE::Style*>(newStyle);
    if (beStyle && beStyle->m_usingStandardPalette)
        return; // can take care of itself and causes recursion

    // harden a bit
    qApp->removeEventFilter(this);
    m_usingStandardPalette = false;


#if 0 // set default palette of new style
    // setting the palette during the deconstructor is no good idea.
    // at least the style kcm will try to delete the deleted style in return
    // and deferring this to the end of the event queue will just cause a recursion
    // on the eventloop
    QPalette pal = newStyle->standardPalette();
    newStyle->polish(pal);
    QApplication::setPalette(pal);
#endif
}

#include "makros.h"
#undef PAL
#define PAL pal

void
Style::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal,
                          bool enabled, const QString& text, QPalette::ColorRole textRole, QRect *boundingRect) const
{
    if (text.isEmpty())
        return;
//     flags |= config.mnemonic;
    QPen savedPen;
    bool penDirty = false;
    if (textRole != QPalette::NoRole) {
        penDirty = true;
        savedPen = painter->pen();
        painter->setPen(QPen(pal.brush(textRole), savedPen.widthF()));
    }
    QRect r = rect.translated( 0, config.fontOffset[painter->font().bold()] );
    if (!enabled) {   // let's see if we can get some blurrage here =)
        if (!penDirty)
            { savedPen = painter->pen(); penDirty = true; }

        const bool wasBold = painter->font().bold();
        if (wasBold) {
            QFont fnt(painter->font());
            fnt.setBold(false);
            painter->setFont(fnt);
        }
        QColor c = painter->pen().color();
        c.setAlpha(c.alpha()/4 + 2);
        painter->setPen(QPen(c, savedPen.widthF()));
        QRect stroke;
        painter->drawText(r, flags, text, &stroke);
        const int y = stroke.y() + stroke.height()/2;
        painter->drawLine(stroke.x(), y, stroke.right(), y);
        if (wasBold) {
            QFont fnt(painter->font());
            fnt.setBold(true);
            painter->setFont(fnt);
        }
    }
    else
        painter->drawText(r, flags, text, boundingRect);
    if (penDirty)
        painter->setPen(savedPen);
}


static struct PainterStore {
    bool aliasing;
    QPen pen;
    QBrush brush;
    QFont font;
    bool clipping;
    QRegion clipRegion;
    int flags;
} gs_painterStore = { false, QPen(), QBrush(), QFont(), false, QRegion(), 0 };

#define X_KdeBase 0xff000000
#define SH_KCustomStyleELement 0xff000001

enum CustomElements { _CE_CapacityBar = 0 /*, ...*/, N_CustomControls };
#if 0
enum SubElements { _SE_AmarokAnalyzerSlider = 0 /*, ...*/, N_CustomSubElements };
#endif

static QStyle::ControlElement primitives[N_CustomControls];
#if 0
static QStyle::SubElement subcontrols[N_CustomSubElements];
#endif

enum ElementType { SH, CE, SE };
static QMap<QString, int> styleElements; // yes. one is enough...
// NOTICE: using QHash instead QMap is probably overhead, there won't be too many items per app
static uint counter[3] = { X_KdeBase+3 /*sic!*/, X_KdeBase, X_KdeBase };

void
Style::drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    Q_ASSERT(option);
    Q_ASSERT(painter);

//    if (pe == PE_IndicatorItemViewItemDrop)
// An indicator that is drawn to show where an item in an item view is about to
// be dropped during a drag-and-drop operation in an item view.
//       qWarning("IndicatorItemViewItemDrop, %d", pe);
    if (pe < N_PE && primitiveRoutine[pe])
        (this->*primitiveRoutine[pe])(option, painter, widget);
    else {
//         qDebug() << "BESPIN, unsupported primitive:" << pe << widget;
        QCommonStyle::drawPrimitive( pe, option, painter, widget );
    }
    if (gs_painterStore.flags) {
        qDebug() << "drawPrimitive : unbalanced painter" << pe << gs_painterStore.flags;
        gs_painterStore.flags = 0;
    }
}

void
Style::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    Q_ASSERT(option);
    Q_ASSERT(painter);
    if (element < N_CE && controlRoutine[element])
        (this->*controlRoutine[element])(option, painter, widget);
    else if (element > X_KdeBase) {
        if (element == primitives[_CE_CapacityBar])
            drawCapacityBar(option, painter, widget);
        //if (pe == primitives[_PE_WhateverElse])
        // ...
    } else {
//         qDebug() << "BESPIN, unsupported control:" << element << widget;
        QCommonStyle::drawControl( element, option, painter, widget );
    }
    if (gs_painterStore.flags) {
        qDebug() << "drawControl : unbalanced painter" << element;
        gs_painterStore.flags = 0;
    }
}

void
Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    Q_ASSERT(option);
    Q_ASSERT(painter);
    if (control < N_CC && complexRoutine[control])
        (this->*complexRoutine[control])(option, painter, widget);
    else {
//         qDebug() << "BESPIN, unsupported complex control:" << control << widget;
        QCommonStyle::drawComplexControl( control, option, painter, widget );
    }
    if (gs_painterStore.flags) {
        qDebug() << "drawComplexControl : unbalanced painter" << control;
        gs_painterStore.flags = 0;
    }
}

int
Style::elementId(const QString &string) const
{
    int id = styleElements.value(string, 0);
    if (id)
        return id;

    if (string == "CE_CapacityBar")
        primitives[_CE_CapacityBar] = (ControlElement)(id = ++counter[CE]);
#if 0
    else if (string == "amarok.CC_Analyzer")
        complexs[_CC_AmarokAnalyzer] = (ComplexControl)(id = ++counter[CC]);
    // subcontrols (SC_) work muchg different as they're 1. flags and 2. attached to a CC
    else if (string == "amarok.CC_Analyzer:SC_Slider")
    {
        subcontrols[_SC_AmarokAnalyzerSlider] = (SubControl)(id = (1 << scCounter[_CC_AmarokAnalyzer]));
        ++scCounter[_CC_AmarokAnalyzer];
    }
//     else if blablablaba...
#endif
    if (id)
        styleElements.insert(string, id);
    return id;
}

/// ----------------------------------------------------------------------

void
Style::erase(const QStyleOption *option, QPainter *painter, const QWidget *widget, const QPoint *offset) const
{
    const QWidget *grampa = widget;
    while ( !(grampa->isWindow() ||
            (grampa->autoFillBackground() && grampa->objectName() != "qt_scrollarea_viewport")))
        grampa = grampa->parentWidget();

    QPoint tl = widget->mapFrom(const_cast<QWidget*>(grampa), QPoint());
    if (offset)
        tl += *offset;
    painter->save();
    painter->setPen(Qt::NoPen);

    const QBrush &brush = grampa->palette().brush(grampa->backgroundRole());

    // we may encounter apps that have semi or *cough* fully *cough* amarok *cough*
    // transparent backgrounds instead of none... ;-)
    const bool needBase = brush.style() > Qt::DiagCrossPattern || brush.color().alpha() < 0xff;

    if (grampa->isWindow() || needBase)
    {   // means we need to paint the global bg as well
        painter->setClipRect(option->rect, Qt::IntersectClip);
        QStyleOption tmpOpt = *option;
        //         tmpOpt.rect = QRect(tl, widget->size());
        tmpOpt.palette = grampa->palette();

        if (tmpOpt.palette.brush(QPalette::Window).style() > 1)
            painter->fillRect(tmpOpt.rect, tmpOpt.palette.brush(QPalette::Window));

        painter->translate(tl);
        drawWindowBg(&tmpOpt, painter, grampa);
    }

    if (!grampa->isWindow())
    {
        painter->setBrush(grampa->palette().brush(grampa->backgroundRole()));
        painter->setBrushOrigin(tl);
        painter->drawRect(option->rect);
    }

    painter->restore();
}

static bool _serverSupportsShadows = false;
static QElapsedTimer _lastCheckTime;
bool
Style::serverSupportsShadows()
{
    if (!BE::isPlatformX11())
        return false; // TODO: port XProperty for wayland
    if (appType == KDM)
        return false;
#ifdef BE_WS_X11
    if (!_lastCheckTime.isValid() || _lastCheckTime.elapsed() > 1000*60*5)
    {
        unsigned long n = 0;
        Atom *supported = BE::XProperty::get<Atom>(DefaultRootWindow(BE_X11_DISPLAY), BE::XProperty::netSupported, BE::XProperty::ATOM, &n);
        for (uint i = 0; i < n; ++i)
            if (supported[i] == BE::XProperty::kwinShadow)
            {
                _serverSupportsShadows = true;
                break;
            }
        if (supported)
            XFree(supported);
        _lastCheckTime.start();
    }
#endif
    return _serverSupportsShadows;
}

static bool equal(const QPalette &p1, const QPalette &p2)
{
    return
    p1.color(QPalette::Active, QPalette::Window) == p2.color(QPalette::Active, QPalette::Window) &&
    p1.color(QPalette::Active, QPalette::WindowText) == p2.color(QPalette::Active, QPalette::WindowText) &&
    p1.color(QPalette::Active, QPalette::Base) == p2.color(QPalette::Active, QPalette::Base) &&
    p1.color(QPalette::Active, QPalette::Text) == p2.color(QPalette::Active, QPalette::Text) &&
    p1.color(QPalette::Active, QPalette::Button) == p2.color(QPalette::Active, QPalette::Button) &&
    p1.color(QPalette::Active, QPalette::ButtonText) == p2.color(QPalette::Active, QPalette::ButtonText) &&
    p1.color(QPalette::Active, QPalette::Highlight) == p2.color(QPalette::Active, QPalette::Highlight) &&
    p1.color(QPalette::Active, QPalette::HighlightedText) == p2.color(QPalette::Active, QPalette::HighlightedText);
//     p1.color(QPalette::Active, QPalette::ToolTipBase) == p2.color(QPalette::Active, QPalette::ToolTipBase) &&
//     p1.color(QPalette::Active, QPalette::ToolTipText) == p2.color(QPalette::Active, QPalette::ToolTipText);
}

static const
QPalette::ColorGroup groups[3] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };


void
Style::swapPalette(QWidget *widget)
{
    // looks complex? IS!
    // reason nr. 1: stylesheets. they're nasty and qt operates on the app palette here
    // reason nr. 2: some idiot must have spread the idea that pal.setColor(backgroundRole(), Qt::transparent)
    // is a great idea instead of just using setAutoFillBackground(false), preserving all colors and just not
    // using them. hey, why not call Qt to paint some nothing.... *grrrr*

    QMap<QWidget*, QString> shits;
    QList<QWidget*> kids = widget->findChildren<QWidget*>();
    kids.prepend(widget);

    QPalette pal;
    QPalette::ColorGroup group;
    bool hasShit = false;

    for (int i = kids.count()-1; i > -1; --i) {
        QWidget *kid = kids.at(i);
        if (kid->testAttribute(Qt::WA_StyleSheet)) {   // first get rid of shit
            shits.insert(kid, kid->styleSheet());
            kid->setStyleSheet(QString());
            hasShit = true;
        }

        // now swap the palette ----------------
        if (hasShit || kid->testAttribute(Qt::WA_SetPalette) || kid == widget) {
            pal = kid->palette();
            hasShit = false;
//             if (kid->inherits("KUrlNavigatorButtonBase") || kid->inherits("BreadcrumbItemButton")) {
//                 // we mess up with it during painting in Hacks
//                 pal.setColor(QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::Window));
//                 kid->setPalette(pal);
//                 continue;
//             }

            for (int i = 0; i < 3; ++i) {
                group = groups[i];
                FX::swap(pal, group, QPalette::Window, QPalette::WindowText, kid);
                FX::swap(pal, group, QPalette::Button, QPalette::ButtonText, kid);
                FX::swap(pal, group, QPalette::Highlight, QPalette::HighlightedText, kid);
                FX::swap(pal, group, QPalette::Base, QPalette::Text, kid);
            }
            polish(pal, false);
            kid->setPalette(pal);
        }
        QTextDocument *document(NULL);
        if (qobject_cast<QLabel*>(kid)) {
            document = kid->findChild<QTextDocument*>();
        }
        else if (QTextEdit *edit = qobject_cast<QTextEdit*>(kid)) {
            document = edit->document();
        }
        if (document) {
            document->setDefaultStyleSheet( QString("*{color:%1;background-color:%2;}a{color:%3;}").arg(kid->palette().color(QPalette::Active, QPalette::Text).name()).arg(kid->palette().color(QPalette::Active, QPalette::Base).name()).arg(kid->palette().color(QPalette::Active, QPalette::Link).name()) );
            if (QTextBrowser *browser = qobject_cast<QTextBrowser*>(kid))
                browser->reload();
        }
    }

    // this is funny: style shits rely on QApplication::palette() (nice trick, TrottelTech... again)
    // so to apply them with the proper color, we need to change the apps palette to the swapped one,...
    if (!shits.isEmpty()) {
        QPalette appPal = QApplication::palette();
        // protect our KDE palette fix - in case
        QPalette *savedPal = originalPalette;
        originalPalette = 0;
        // ... reapply the shits...
        QMap<QWidget*, QString>::const_iterator shit = shits.constBegin();
        qApp->installEventFilter(&eventKiller); // temp. change to trick next setStyleSheet calls
                                                // nobody needs to know and some clients - looking at
                                                // ktexteditor here - do *really* expensive stuff on app
                                                // palette changes
        while (shit != shits.constEnd()) {
            QApplication::setPalette(shit.key()->palette());
            if (shit.value().isEmpty()) // *sigh*
                shit.key()->setStyleSheet(QString(shit.key()->metaObject()->className()) + "{color:red;}");
            shit.key()->setStyleSheet(shit.value());
            ++shit;
        }
        // ... and reset the apps palette
        QApplication::setPalette(appPal);
        qApp->removeEventFilter(&eventKiller);
        originalPalette = savedPal;
    }
}

bool isUrlNaviButtonArrow = false;
static bool isLastNavigatorButton(const QWidget *w, const char *className)
{
    if (QWidget *navigator = w->parentWidget())
    {
        QList<QPushButton*> btns = navigator->findChildren<QPushButton*>();
        QList<QPushButton*>::const_iterator i = btns.constEnd();
        while (i != btns.constBegin())
        {
            --i;
            if ((*i)->inherits(className))
                return (*i == w);
        }
    }
    return false;
}

// settings the property can be expensive, so avoid for popups, combodrops etc.
// NOTICE:
// Qt::Dialog must be erased as it's in drawer et al. but takes away window as well
// Qt::FramelessWindowHint shall /not/ be ignored as the window could change it's opinion while being visible
static const
Qt::WindowFlags ignoreForDecoHints = ( Qt::Popup | Qt::SubWindow |
Qt::ToolTip | Qt::SplashScreen | Qt::Desktop | Qt::X11BypassWindowManagerHint /*| Qt::FramelessWindowHint*/ ) & (~(Qt::Dialog|Qt::Sheet));


static void detectBlurRegion(QWidget *window, const QWidget *widget, QRegion &blur)
{
    const QObjectList &kids = widget->children();
    QObjectList::const_iterator i;
    for ( i = kids.begin(); i != kids.end(); ++i )
    {
        QObject *o = (*i);
        if ( !o->isWidgetType() )
            continue;
        QWidget *w = static_cast<QWidget*>(o);
        if ( w->isVisible() &&
            ((w->autoFillBackground() && w->palette().color(w->backgroundRole()).alpha() > 160) || (w->testAttribute(Qt::WA_OpaquePaintEvent) && !(qobject_cast<const QScrollBar*>(w) || w->inherits("QProgressBar")))) )
        {
            QPoint offset = w->mapTo(window, QPoint(0,0));
            if (w->mask().isEmpty())
                blur -= w->rect().translated(offset);
            else
                blur -= w->mask().translated(offset);
            continue; // ne nood for deeper checks
        }
        else
            detectBlurRegion(window, w, blur);
    }
}

static QList<QWeakPointer<QWidget> > pendingBlurUpdates;

void
Style::updateBlurRegions() const
{
#ifdef BE_WS_X11 // hint blur region for the kwin plugin
    if (!BE::isPlatformX11())
        return; // TODO: port XProperty for wayland
    for (QList<QWeakPointer<QWidget> >::const_iterator it = pendingBlurUpdates.constBegin(),
                                                  end = pendingBlurUpdates.constEnd(); it != end; ++it)
    {
        if (!*it)
            continue;
        QSharedPointer<QWidget> widget = it->toStrongRef();
        if (widget && !(widget->testAttribute(Qt::WA_WState_Created) || widget->internalWinId()))
            continue; // protect against pseudo widgets, see setupDecoFor()

        QRegion blur = widget->mask().isEmpty() ? widget->rect() : widget->mask();
        detectBlurRegion(widget.data(), widget.data(), blur);
        if (blur.isEmpty())
            continue;

        QVector<unsigned long> data(blur.rectCount() * 4);
        for ( auto i = blur.begin(); i != blur.end(); ++i )
        {
            if (i->width() > 0 && i->height() > 0)
                data << i->x() << i->y() << i->width() << i->height();
        }
        BE::XProperty::set<unsigned long>(widget->winId(), BE::XProperty::blurRegion, (unsigned long*)data.constData(), BE::XProperty::LONG, data.size());
    }
#endif
    pendingBlurUpdates.clear();
}

static void shapeCorners( QWidget *widget, bool forceShadows )
{
#ifdef BE_WS_X11
    if (forceShadows && isPlatformX11()) // kwin/beshadowed needs a little hint to shadow this one nevertheless
        BE::XProperty::setAtom( widget->winId(), BE::XProperty::forceShadows );
#endif

    if (widget->isWindow() && FX::compositingActive() && BE::Style::serverSupportsShadows()) {
        widget->clearMask();
        return;
    }

    const int w = widget->width();
    const int h = widget->height();
    QRegion mask(4, 0, w-8, h);
    mask += QRegion(0, 4, w, h-8);
    mask += QRegion(2, 1, w-4, h-2);
    mask += QRegion(1, 2, w-2, h-4);
    // only top rounded - but looks nasty
    //          QRegion mask(0, 0, w, h-4);
    //          mask += QRect(1, h-4, w-2, 2);
    //          mask += QRect(2, h-2, w-4, 1);
    //          mask += QRect(4, h-1, w-8, 1);
    widget->setMask(mask);
}

void reBlur(QWidget *widget, bool round) {
#ifdef BE_WS_X11
    if (!(widget->windowOpacity() < 1.0 && widget->testAttribute(Qt::WA_WState_Created) &&
          widget->internalWinId() && BE::isPlatformX11())) // TODO: port XProperty for wayland
        return;

    if (!round) {
        unsigned long zero(0);
        XProperty::set<unsigned long>(widget->winId(), XProperty::blurRegion, &zero, XProperty::LONG, 1);
        return;
    }

    const int w = widget->rect().width();
    const int h = widget->rect().height();
    QVector<unsigned long> data(4 * 4);
    data << 4 << 0 << w - 8 << h
         << 0 << 4 << w << h - 8
         << 2 << 1 << w - 4 << h - 2
         << 1 << 2 << w - 2 << h - 4;

    BE::XProperty::set<unsigned long>(widget->winId(), BE::XProperty::blurRegion, (unsigned long*)data.constData(), BE::XProperty::LONG, data.size());
#endif
}

bool
Style::eventFilter( QObject *object, QEvent *ev )
{
    switch (ev->type())
    {
    case QEvent::HoverMove:
    case QEvent::MouseMove:
    case QEvent::Timer:
    case QEvent::Move:
        return false; // just for performance - they can occur really often
    case QEvent::Paint:
//         if (object->isWidgetType())
        if (QWidget *window = static_cast<QWidget*>(object))
        if (window->testAttribute(Qt::WA_TranslucentBackground))
        if (window->testAttribute(Qt::WA_StyledBackground))
        if (window->isWindow())
        {
            QPainter p(window);
            p.setClipRegion( static_cast<QPaintEvent*>(ev)->region() );
            drawWindowBg(0, &p, window);
            p.end();
            return false;
        }
        if (QTabBar *tabBar = qobject_cast<QTabBar*>(object))
        {
            if (tabBar->testAttribute(Qt::WA_NoSystemBackground))
                return false; // shall be translucent
            if (!tabBar->drawBase())
                return false;
            if (QTabWidget *tw = qobject_cast<QTabWidget*>(tabBar->parentWidget()))
            {   // no extra tabbar here please... unless the border is StyleShitted away ;)
                if (tw->styleSheet().isEmpty())
                    return false;
                if ( !(tw->styleSheet().contains("pane", Qt::CaseInsensitive) && tw->styleSheet().contains("border", Qt::CaseInsensitive)) )
                    return false;
            }
            if ( appType == KDevelop )
            {   // KDevelop does that... weird. - and of course /after/ painting the label string...
                QWidget *dad = tabBar->parentWidget();
                while ( dad )
                {
                    if (dad->inherits("QMenuBar"))
                        return false;
                    dad = dad->parentWidget();
                }
            }

            QPainter p(tabBar);
            QStyleOptionTabBarBase opt;
            opt.initFrom(tabBar);
            opt.shape = tabBar->shape();
            opt.selectedTabRect = tabBar->tabRect(tabBar->currentIndex());
            opt.tabBarRect = opt.rect;
            opt.documentMode = tabBar->documentMode();
//             if (QWidget *window = tabBar->window())
//             {
//                 opt.tabBarRect = window->rect();
//                 opt.tabBarRect.moveTopLeft(tabBar->mapFrom(window, opt.tabBarRect.topLeft()));
//             }
            drawTabBar(&opt, &p, NULL);
            p.end();
            return false;
        } else if ( QTabWidget *tw = qobject_cast<QTabWidget*>( object ) ) {
            // those don't paint frames and rely on the tabbar, which we ruled and rule out (looks weird with e.g. cornerwidgets...)
            if (tw->documentMode()) {
                QStyleOptionTabBarBase opt;
                opt.initFrom(tw);
                opt.documentMode = true;

                QTabBar *bar = NULL;
#if QT_VERSION < 0x050000
                foreach (QObject *o, tw->children()) {
                    if ((bar = qobject_cast<QTabBar*>(o)))
                        break;
                }
#else
                bar = tw->tabBar();
#endif
                int thickness = 0;
                if (bar) {
                    if (bar->isHidden())
                        return true;
                    opt.rect = bar->geometry();
                    opt.selectedTabRect = bar->tabRect(bar->currentIndex()).translated(opt.rect.topLeft());
                    thickness = 1;
                } else {
                    thickness = pixelMetric(PM_TabBarBaseHeight, &opt, tw);
                }

                if (thickness < 1) {
                    return true;
                }

                switch (tw->tabPosition()) {
                    default:
                    case QTabWidget::North:
                        if (bar) {
                            opt.rect.setLeft(tw->rect().left());
                            opt.rect.setRight(tw->rect().right());
                        } else {
                            opt.rect.setBottom(opt.rect.top() + thickness);
                        }
                        opt.shape = QTabBar::RoundedNorth; break;
                    case QTabWidget::South:
                        if (bar) {
                            opt.rect.setLeft(tw->rect().left());
                            opt.rect.setRight(tw->rect().right());
                        } else {
                            opt.rect.setTop(opt.rect.bottom() - thickness);
                        }
                        opt.shape = QTabBar::RoundedSouth; break;
                    case QTabWidget::West:
                        if (bar) {
                            opt.rect.setTop(tw->rect().top());
                            opt.rect.setBottom(tw->rect().bottom());
                        } else {
                            opt.rect.setRight(opt.rect.left() + thickness);
                        }
                        opt.shape = QTabBar::RoundedWest; break;
                    case QTabWidget::East:
                        if (bar) {
                            opt.rect.setTop(tw->rect().top());
                            opt.rect.setBottom(tw->rect().bottom());
                        } else {
                            opt.rect.setLeft(opt.rect.right() - thickness);
                        }
                        opt.shape = QTabBar::RoundedEast; break;
                }

                if (opt.rect.isEmpty())
                    return true; // pointless

                opt.tabBarRect = opt.rect;
                QPainter p(tw);
                drawTabBar(&opt, &p, NULL);
                p.end();
                return true; // don't let it paint weird stuff on the cornerwidgets etc.
            }
            return false;
        } else if (QPushButton *w = qobject_cast<QPushButton*>(object)) {
            bool b = false;
            if ((b = w->inherits("KUrlNavigatorButtonBase")) || w->inherits("BreadcrumbItemButton"))
            {
                static bool isRecursion = false;
                if (isRecursion)
                    return false;
                isRecursion = true;
                isUrlNaviButtonArrow = true;
                object->removeEventFilter(this);

                if (w->text() == "/")
                    w->setText("/.");
                QPalette::ColorRole fg = w->parentWidget() ? w->parentWidget()->foregroundRole() : QPalette::WindowText;
                if (fg != QPalette::WindowText || isLastNavigatorButton(w, b?"KUrlNavigatorButtonBase":"BreadcrumbItemButton"))
                {
                    if (w->foregroundRole() != fg)
                        w->setForegroundRole(fg);
                }
                else if (w->foregroundRole() != QPalette::Link)
                    w->setForegroundRole(QPalette::Link);

                QCoreApplication::sendEvent(object, ev);

                object->installEventFilter(this);
                isUrlNaviButtonArrow = false;
                isRecursion = false;
                return true;
            }
            return false;
        }
        return false;

    case QEvent::Enter:
    case QEvent::Leave:
    {
        if (qobject_cast<QPushButton*>(object))
        {
            bool b = false;
            if ((b = object->inherits("KUrlNavigatorButtonBase")) || object->inherits("BreadcrumbItemButton"))
            {
                QWidget *w = static_cast<QWidget*>(object);
                w->setCursor(Qt::PointingHandCursor);
                QFont fnt = w->font();
                if (isLastNavigatorButton(w, b?"KUrlNavigatorButtonBase":"BreadcrumbItemButton"))
                {
                    w->setCursor(Qt::ArrowCursor);
                    fnt.setUnderline(false);
                }
                else
                    fnt.setUnderline(ev->type() == QEvent::Enter);
                w->setFont(fnt);
                return false;
            }
            return false;
        }
        return false;
    }

    case QEvent::Wheel:
    {
        if (QAbstractSlider* slider = qobject_cast<QAbstractSlider*>(object))
        {
            QWheelEvent *we = static_cast<QWheelEvent*>(ev);
            if ((slider->value() == slider->minimum() && (we->angleDelta().y() || we->angleDelta().x()) > 0) ||
                (slider->value() == slider->maximum() && (we->angleDelta().y() || we->angleDelta().x()) < 0))
                Animator::Hover::Play(slider);
            return false;
        }

        if (QListView *list = qobject_cast<QListView*>(object))
        {
//             if (list->verticalScrollMode() == QAbstractItemView::ScrollPerPixel) // this should be, but semms to be not
            if (list->iconSize().height() > -1) // happens on e.g. config views
            if (list->inherits("KCategorizedView"))
                list->verticalScrollBar()->setSingleStep(list->iconSize().height()/3);
        }
        return false;
    }
#if BESPIN_MOUSE_DEBUG
    case QEvent::MouseButtonPress:
    {
//         QMouseEvent *mev = (QMouseEvent*)ev;
        QWidget *w = static_cast<QWidget*>(object);
//         QString debug("BESPIN: ");
//         while (w)
//         {
//             debug += w->className() + QString(" \"") + w->objectName() + "\"  (" + QString(w->autoFillBackground() ? "solid" : "transparent" ) + ") :: ";
//             w = w->parentWidget();
//         }
//         qDebug() << debug;
        qDebug() << "BESPIN:" << w << w->geometry() << w->parentWidget();
        //       DEBUG (object);
        return false;
    }
#endif
    case QEvent::Show:
    {
        QWidget * widget = qobject_cast<QWidget*>(object);
        if (!widget)
            return false;

        // talk to kwin about colors, gradients, etc.
        if (widget->isWindow()) {
            if (!(widget->windowFlags() & ignoreForDecoHints)) {
                const bool swappedPal = widget->property("BE.swappedPalette").toBool();
                // setup some special stuff for modal windows
                if (config.invert.modals && widget->style() == this && widget->isModal() != swappedPal) { // swapping QStyleSheetStyle is one epic fail ..
                    widget->setProperty("BE.swappedPalette", widget->isModal());
                    swapPalette(widget);
                }
                if (!(widget->testAttribute(Qt::WA_X11NetWmWindowTypeDesktop) ||
                             widget->testAttribute(Qt::WA_TranslucentBackground))) {
                    // adjust window opacity
                    if (widget->isModal()) {
                        if (config.bg.modal.opacity < 0xff)
                            widget->setWindowOpacity(config.bg.modal.opacity/255.0);
                    } else {
                        if (config.bg.opacity < 0xff)
                            widget->setWindowOpacity(config.bg.opacity/255.0);
                    }
                }
#ifdef BE_WS_X11
                if (!(widget->windowFlags() & ignoreForDecoHints))
                    setupDecoFor(widget, widget->palette());
#endif
            } else if (QMenu * menu = qobject_cast<QMenu*>(widget)) {
                if (config.bg.modal.opacity < 0xff)
                    widget->setWindowOpacity(config.bg.modal.opacity/255.0);
                // seems to be necessary, somehow KToolBar context menus manages to take QPalette::Window...?!
                // through title setting?!
                if (menu->parentWidget() && menu->parentWidget()->inherits("QMdiSubWindow")) {
                    QPoint pt = menu->parentWidget()->rect().topRight();
                    pt += QPoint(-menu->width(), pixelMetric(PM_TitleBarHeight,0,0));
                    pt = menu->parentWidget()->mapToGlobal(pt);
                    menu->move(pt);
                }
                /// @todo this abysmally fails at least w/ Qt6 on a multihead display
//                menu->move(menu->pos()-QPoint(0,F(2)));
                if (config.frame.roundness > 2)
                    shapeCorners( widget, false );
            }
#ifdef BE_WS_X11
            if (config.bg.blur)
                reBlur(widget, config.frame.roundness > 2);
#endif
            return false;
        }
        return false;
    }

    case QEvent::Resize:
    {
        QWidget * widget = qobject_cast<QWidget*>(object);
        if (!widget)
            return false;

        // talk to kwin about colors, gradients, etc.
        if (widget->isWindow() && widget->isVisible() && config.bg.blur) {
            reBlur(widget, config.frame.roundness > 2);
        }
        return false;
    }

#if 1
    case QEvent::PaletteChange:
    {
        #define CONTRAST(_C1_, _C2_) FX::contrastOf(pal.color(group, _C1_), pal.color(group, _C2_))
        #define LACK_CONTRAST(_C1_, _C2_) (pal.color(group, _C1_).alpha() > 64 && FX::contrastOf(pal.color(group, _C1_), pal.color(group, _C2_)) < 20)
        #define HARD_CONTRAST(_C_) pal.color(group, _C_).alpha() < 64 ? Qt::red : (FX::value(pal.color(group, _C_)) < 128 ? Qt::white : Qt::black)
        QWidget * widget = qobject_cast<QWidget*>(object);
        if (!widget)
            return false;
#ifdef BE_WS_X11
        // talk to kwin about colors, gradients, etc.
        if (widget->testAttribute(Qt::WA_WState_Created) && widget->internalWinId() &&
            widget->isWindow() && !(widget->windowFlags() & ignoreForDecoHints) &&
            BE::isPlatformX11()) // TODO: port XProperty for wayland
        {
            setupDecoFor(widget, widget->palette());
            BE::XProperty::remove(widget->winId(), BE::XProperty::bgPics);
            return false;
        }
#endif

        // i think, i hope i got it....
        // 1. khtml sets buttontext, windowtext and text to the fg color - what leads to trouble if e.g. button doesn't contrast window
        // 2. combolists need a special kick (but their palette seems ok, though it isn't...)
        // 3. css causes more trouble - esp. with semitransparent colors...
        if (widget->objectName() == "RenderFormElementWidget")
        {
            QPalette pal = widget->palette();
            bool paletteChanged = false;
            for (int g = 0; g < 3; ++g)
            {
                QPalette::ColorGroup group = (QPalette::ColorGroup)g;
                if (pal.color(group, QPalette::Window).alpha() < 64)
                    pal.setColor(group, QPalette::Window, qApp->palette().color(group, QPalette::Window));
                if (LACK_CONTRAST(QPalette::Window, QPalette::WindowText))
                {
                    paletteChanged = true;
                    pal.setColor(group, QPalette::WindowText, HARD_CONTRAST(QPalette::Window));
                }
                if (LACK_CONTRAST(QPalette::Button, QPalette::ButtonText))
                {
                    paletteChanged = true;
                    pal.setColor(group, QPalette::ButtonText, HARD_CONTRAST(QPalette::Button));
                }
                if (LACK_CONTRAST(QPalette::Base, QPalette::Text))
                {
                    paletteChanged = true;
                    pal.setColor(group, QPalette::Text, HARD_CONTRAST(QPalette::Base));
                }
            }

            if (paletteChanged)
            {
                widget->removeEventFilter(this);
                widget->setPalette(pal);
                widget->installEventFilter(this);

                // TODO: this might cause trouble with palettes with translucent backgrounds...
                if (QComboBox *box = qobject_cast<QComboBox*>(widget))
                if (box->view())
                    box->view()->setPalette(pal);
            }

            return false;
        }
        return false;
    }
#endif
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        if ( object->isWidgetType() && object->inherits("QWebView") )
            static_cast<QWidget*>(object)->update();
        return false;
    case QEvent::ApplicationPaletteChange: {
        if (object == qApp && originalPalette && !equal(qApp->palette(), *originalPalette)) {
            object->removeEventFilter(this);
            QPalette *pal = originalPalette;
            originalPalette = 0;
            QApplication::setPalette(*pal);
            delete pal;
        }
        return false;
    }
    default:
        return false;
    }
}


QPalette
Style::standardPalette() const
{   // hue = 207°
    QColor bg(241, 245, 248);
    QColor fg(27, 27, 28);
    QColor hg(0, 138, 255);
    QColor hgt(247, 252, 255);
    QColor mid(124, 126, 128);
//     QColor bg(2, 11, 16);
//     QColor fg(197, 235, 240);
//     QColor hg(120, 201, 230);
//     QColor hgt(16, 28, 32);
//     QColor mid(14, 67, 91);
    if (objectName().toLower() == "flynn") {
        bg.setRgb(2, 11, 16);
        fg.setRgb(197, 235, 240);
        hg.setRgb(120, 201, 230);
        hgt.setRgb(16, 28, 32);
        mid.setRgb(14, 67, 91);
    } else if (objectName().toLower() == "virtualbreeze") {
        bg.setRgb(239, 240, 241); // cardboard grey
        fg.setRgb(49, 54, 59); // charcoal grey
        hg.setRgb(61, 174, 233); // plasma blue
        hgt.setRgb(252, 252, 252); // paper white
        mid.setRgb(127, 127, 128);
    } else if (objectName().toLower() == "virtualarch") {
        bg.setRgb(250, 250, 250);
        fg.setRgb(51, 51, 51); // charcoal grey
        hg.setRgb(23, 147, 208); // plasma blue
        hgt.setRgb(252, 252, 252); // paper white
        mid.setRgb(127, 127, 128);
    }
    QPalette pal(fg, bg, // windowText, button
                        bg, fg, mid, //light, dark, mid
                        fg, mid, //text, bright_text
                        bg, bg ); //base, window
    pal.setColor(QPalette::ButtonText, fg);
    pal.setColor(QPalette::Highlight, hg);
    pal.setColor(QPalette::HighlightedText, hgt);
    pal.setColor(QPalette::ToolTipBase, fg);
    pal.setColor(QPalette::ToolTipText, bg);
    return pal;
}

int
Style::savePainter(QPainter *p, int flags)
{
    flags &= ~(gs_painterStore.flags);
    gs_painterStore.flags |= flags;
    if (flags & Alias)
        gs_painterStore.aliasing = (p->renderHints() & QPainter::Antialiasing);
    if (flags & Pen)
        gs_painterStore.pen = p->pen();
    if (flags & Brush)
        gs_painterStore.brush = p->brush();
    if (flags & Font)
        gs_painterStore.font = p->font();
    if (flags & Clip) {
        gs_painterStore.clipping = p->hasClipping();
        gs_painterStore.clipRegion = p->clipRegion();
    }
    return flags;
}

void
Style::restorePainter(QPainter *p, int flags)
{
    flags &= gs_painterStore.flags;
    if (flags & Alias)
        p->setRenderHint(QPainter::Antialiasing, gs_painterStore.aliasing);
    if (flags & Pen)
        p->setPen(gs_painterStore.pen);
    if (flags & Brush)
        p->setBrush(gs_painterStore.brush);
    if (flags & Font)
        p->setFont(gs_painterStore.font);
    if (flags & Clip) {
        p->setClipRegion(gs_painterStore.clipRegion);
        p->setClipping(gs_painterStore.clipping);
    }
    gs_painterStore.flags &= ~flags;
}

#undef PAL
#undef BESPIN_MOUSE_DEBUG

