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
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QProcess>
#include <QSettings>
#include <QTimer>

#include <cmath>

#include "animator/tab.h"
#include "virtuality.h"
#include "hacks.h"
#include "makros.h"
#include "config.defaults"

#include <QtDebug>

using namespace BE;

#define SCALE(_N_) qRound((_N_)*config.scale)

static int clamp(int x, int l, int u)
{
    return CLAMP(x,l,u);
}

#if 1
static int fontOffset(bool bold = false, int *extend = 0)
{
    QString string = "qtipdfghjklöäyxbQWRTZIPÜSDFGHJKLÖÄYXVBN!()?ß\"";
    QFont font;
    font.setBold(bold);
    QFontMetrics metrics(font);
    QImage img(metrics.size(0, string) + QSize(4,4), QImage::Format_ARGB32);
    img.fill(0xffffffff);
    QPainter p(&img); p.setPen(Qt::black); p.setFont(font); p.drawText(img.rect(), Qt::AlignCenter, string); p.end();
    int y1 = 0, y2 = 0;
    for (int y = 0; y < img.height(); ++y)
    {
        QRgb *scanLine = (QRgb*)img.scanLine(y);
        for (int x = 0; x < img.width(); ++x)
        {
            if (qRed(*scanLine++) < 128)
                { y1 = y; goto descent; }
        }
    }
descent:
    for (int y = img.height()-1; y > -1; --y)
    {
        QRgb *scanLine = (QRgb*)img.scanLine(y);
        for (int x = 0; x < img.width(); ++x)
        {
            if (qRed(*scanLine++) < 128)
                { y2 = img.height()-y-1; goto offset; }
        }
    }
offset:
    if ( extend )
    {
        *extend = y1 + y2 - 4;
        *extend -= qAbs(*extend)%2;
//         qDebug() << y1 << y2 << *extend;
    }
//     qDebug() << y1 << y2;
    if ( y1 == y2) // font is centered ... hopefully ...
        return 0;
    int off = (y2-y1)/2;
    if ( !off )
    {
        off += (y1 < 2); // the font has a negtive extent to the top
        off -= (y2 < 2); // the font has a negtive extent to the bottom
    }
//     qDebug() << "->" << off;
    return off;
//     return y2 - y1/2;
//     return (y2-y1)/2;
}
#endif

#define readInt(_DEF_) iSettings->value(_DEF_).toInt()
#define readBool(_DEF_) iSettings->value(_DEF_).toBool()
#define readRole(_VAR_, _DEF_)\
config._VAR_##_role[0] = (QPalette::ColorRole) iSettings->value(_DEF_).toInt();\
FX::counterRole(config._VAR_##_role[0], config._VAR_##_role[1])
//, QPalette::_DEF_, FX::counterRole(QPalette::_DEF_))

void
Style::removeAppEventFilter()
{
    // for plain qt apps that don't need the palette fix, otherwise we'd keep filtering the app for no reason
    delete originalPalette;
    originalPalette = 0;
    qApp->removeEventFilter(this);
}

void
Style::setMenuIconsVisible(bool vis)
{
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus, !vis);
}

void
Style::readSettings(QString appName)
{
    QSettings *iSettings = new QSettings("BE", "Style");
    iSettings->beginGroup("Current");
    //BEGIN read some user personal settings (i.e. not preset related)
    config.leftHanded = readBool(LEFTHANDED) ? Qt::RightToLeft : Qt::LeftToRight;
    // item single vs. double click, wizard appereance
    config.macStyle = readBool(MACSTYLE);
    config.dialogBtnLayout = readInt(DIALOG_BTN_LAYOUT);
    config.fadeInactive = readBool(FADE_INACTIVE);
    // Hacks ==================================
    Hacks::config.windowMovement = readBool(HACK_WINDOWMOVE);
    Hacks::config.fixGwenview = readBool(HACK_FIX_GWENVIEW);
    Hacks::config.lockToolBars = readBool(HACK_TOOLBAR_LOCKING);
    Hacks::config.lockDocks = readBool(HACK_DOCK_LOCKING);
    Hacks::config.panning = readBool(HACK_PANNING);
    Hacks::config.suspendFullscreenPlayers = readBool(HACK_SUSPEND_FULLSCREEN_PLAYERS);
    if (Hacks::config.suspendFullscreenPlayers)
        Hacks::config.suspendFullscreenPlayers = (  appName == "dragonplayer" || appName == "smplayer" ||
                                                    appName == "minitube" || appName == "vlc"  );
    Hacks::config.titleWidgets = readBool(HACK_TITLE_WIDGET);
    Hacks::config.suppressBrightnessOSD = readBool(HACK_BRIGHTNESS_OSD);

    // Font fixing offsets
    config.fontOffset[0] = fontOffset(false, &config.fontExtent);
    config.fontExtent = iSettings->value( "FontExtent", config.fontExtent ).toInt();
    config.fontOffset[1] = fontOffset(true);
    QStringList lst = iSettings->value( "FontOffset", QStringList() ).toStringList();
    if ( lst.count() > 0 )
    {
        config.fontOffset[0] = lst.at(0).toInt();
        if ( lst.count() > 1 )
            config.fontOffset[1] = lst.at(1).toInt();
        else
            config.fontOffset[1] = config.fontOffset[0];
    }

    // PW Echo Char ===========================
    config.input.pwEchoChar = ushort(iSettings->value(INPUT_PWECHOCHAR).toUInt());

    config.menu.delay = readInt(MENU_DELAY);
    const bool menuIconsVis = readBool(MENU_SHOWICONS);
    // allow (K)Application to manipulate this first
    QMetaObject::invokeMethod(this, "setMenuIconsVisible", Qt::QueuedConnection, Q_ARG(bool, menuIconsVis));
    config.menu.indent = readBool(MENU_INDENT);

    config.winBtnStyle = 2; // this is a kwin deco setting, TODO: read from there?

    Animator::Tab::setTransition((Animator::Transition) readInt(TAB_TRANSITION));
    if (appType == Falkon)
        Animator::Tab::setTransition(Animator::Jump);
    Animator::Tab::setDuration(clamp(iSettings->value(TAB_DURATION).toUInt(), 150, 4000));
    //END personal settings
    //NOTICE we do not end group here, but below. this way it's open if we don't make use of presets

    // Background ===========================
    config.invert.docks = readBool(INVERT_DOCKS);
    config.invert.headers = readBool(INVERT_HEADERS);
    if (appName == "sqriptor")
        config.invert.headers = false;
    config.invert.menubars = readBool(INVERT_MENUBARS);
    config.invert.menus = readBool(INVERT_MENUS);
    config.invert.modals = (appType != KDM) && readBool(INVERT_MODALS);
    config.invert.titlebars = readBool(INVERT_TITLEBARS);
    config.invert.toolbars = readBool(INVERT_TOOLBARS);

    config.bg.opacity = readInt(BG_OPACITY);
    config.bg.modal.opacity = readInt(BG_MODAL_OPACITY);
    if (config.bg.opacity || config.bg.modal.opacity)
        config.bg.blur = readBool(BG_BLUR);
    else
        config.bg.blur = false;

    config.bg.ringOverlay = readInt(BG_RING_OVERLAY);


    // Buttons ===========================
    config.btn.minHeight = readInt(BTN_MIN_HEIGHT);

    // .Tool
    config.btn.tool.disabledStyle = readInt(BTN_DISABLED_TOOLS);

    //--------
    config.slider.buttons = readBool(SLIDER_BUTTONS);

    if (readBool(SHOW_MNEMONIC))
        config.mnemonic = Qt::TextShowMnemonic;
    else
        config.mnemonic = Qt::TextHideMnemonic;

    // Tabs ===========================
    Animator::Tab::setFPS(25);

    // General ===========================
    QWidget dummy;  // calling out to qApp->desktopWidget() would seem natural
                    // BUT BREAKS THE CLIPBOARD!
    config.scale = (dummy.logicalDpiX() + dummy.logicalDpiY()) /  170.0f;
    QFont fnt = qApp->font();
    if (fnt.pointSizeF() > -1)
        config.scale *= fnt.pointSizeF()/10.0f;
    if (const char *scale = getenv("VIRTUALITY_SCALE")) {
        bool ok = false;
        const float envScale = QString(scale).toFloat(&ok); // CLAMP(envScale, 1.0f, 3.0f);
        if (ok) {
            if (envScale != config.scale) {
                scale = getenv("VIRTUALITY_SCALE_FONT");
                if (!qstrcmp(scale, "true")) {
                    if (fnt.pointSize() > -1)
                        fnt.setPointSizeF(fnt.pointSizeF()*envScale/config.scale);
                    else
                        fnt.setPixelSize(fnt.pixelSize()*envScale/config.scale);
                    qApp->setFont(fnt);
                }
            }
            config.scale = envScale;
        }
    }

    config.slider.thickness = SCALE(qMax(2,readInt(SLIDER_THICKNESS)));
    config.frame.roundness = SCALE(qMax(0,readInt(ROUNDNESS)));
//     config.strokeWidth = qMin(config.frame.roundness, SCALE(readInt(STROKE_WIDTH)));
    config.strokeWidth = SCALE(qMax(0,readInt(STROKE_WIDTH)));
    halfStroke = 0.5*config.strokeWidth;

    //NOTICE gtk-qt fails on several features
    // a key problem seems to be fixed text colors
    // also it will segfault if we hide scrollbar buttons
    // so we adjust some settings here
    if (appType == GTK || appType == OpenOffice)
    {
        config.bg.modal.opacity = 255;
        config.invert.docks = config.invert.menubars = config.invert.menus = config.invert.modals =
        config.invert.toolbars = config.invert.headers = false;
    }

    delete iSettings;

}

#undef readRole
#undef gradientType

void Style::initMetrics()
{
    BE::Dpi::target.f1 = SCALE(1); BE::Dpi::target.f2 = SCALE(2);
    BE::Dpi::target.f3 = SCALE(3); BE::Dpi::target.f4 = SCALE(4);
    BE::Dpi::target.f5 = SCALE(5); BE::Dpi::target.f6 = SCALE(6);
    BE::Dpi::target.f7 = SCALE(7); BE::Dpi::target.f8 = SCALE(8);
    BE::Dpi::target.f9 = SCALE(9); BE::Dpi::target.f10 =SCALE(10);
    BE::Dpi::target.f12 = SCALE(12); BE::Dpi::target.f13 = SCALE(13);
    BE::Dpi::target.f16 = SCALE(16); BE::Dpi::target.f17 = SCALE(17);
    BE::Dpi::target.f18 = SCALE(18); BE::Dpi::target.f20 = SCALE(20);
    BE::Dpi::target.f22 = SCALE(22); BE::Dpi::target.f32 = SCALE(32);
    BE::Dpi::target.f64 = SCALE(64); BE::Dpi::target.f80 = SCALE(80);
}

#undef SCALE

extern const QString virtuality_revision();

void
Style::init()
{
    QElapsedTimer time; time.start();
    // various workarounds... ==========================
    appType = Unknown;
    QString appName;
    if (!qApp->inherits("KApplication") && getenv("GTK_QT_ENGINE_ACTIVE"))
        { appType = GTK; /*qWarning("BESPIN: Detected GKT+ application");*/ }
    else if (qApp->inherits("GreeterApp"))
        appType = KDM;
    else
    {
        appName = QCoreApplication::applicationName();
        if (appName.isEmpty() && !QCoreApplication::arguments().isEmpty())
            appName = QCoreApplication::arguments().at(0).section('/', -1);
        if (appName == "dolphin")
            appType = Dolphin;
        else if (appName == "konversation")
            appType = Konversation;
        else if (appName == "be.shell")
            appType = BEshell;
        else if (appName.startsWith("plasma") || appName == "krunner" || appName == "kscreenlocker_greet")
            appType = Plasma;
        else if (appName == "qbittorrent")
            appType = QBittorrent;
        else if (appName == "Designer" || appName == "designer")
            appType = QtDesigner;
        else if (appName == "kdevelop")
            appType = KDevelop;
        else if (appName == "kwin")
            appType = KWin;
        else if (appName == "amarok")
            appType = Amarok;
        else if ( appName == "gwenview" )
            appType = Gwenview;
        else if (appName == "OpenOffice.org" || appName == "soffice.bin")
            appType = OpenOffice;
        else if (appName == "vlc")
            appType = VLC;
        else if (appName == "kmail")
            appType = KMail;
        else if (appName == "arora")
            appType = Arora;
        else if ( appName == "konqueror")
            appType = Konqueror;
        else if ( appName == "falkon")
            appType = Falkon;
        else if ( appName == "Kde4ToolkitLibrary" )
        {
            appName = "opera";
            appType = Opera;
        }
    }

    // ==========================

    readSettings(appName);

    if (objectName() == "sienar") {
        config.invert.docks = true;
        config.invert.menubars = true;
        config.invert.menus = true;
        config.invert.modals = true;
        config.invert.titlebars = true;
        config.invert.toolbars = true;
        config.bg.opacity = 0xff;
        config.bg.modal.opacity = 250;
        config.bg.blur = false;
        config.frame.roundness = 6;
        config.bg.ringOverlay = 2;
    } else if (objectName() == "flynn") {
        config.invert.docks = false;
        config.invert.menubars = false;
        config.invert.menus = false;
        config.invert.modals = false;
        config.invert.titlebars = false;
        config.invert.toolbars = false;
        config.bg.ringOverlay = 3;
//         config.bg.opacity = readInt(BG_OPACITY);
//         config.bg.modal.opacity = readInt(BG_MODAL_OPACITY);
//         config.bg.blur = false;
//         config.frame.roundness = SCALE(readInt(ROUNDNESS));
    } else if (objectName() == "virtualbreeze") {
        config.invert.docks = false;
        config.invert.menubars = false;
        config.invert.menus = true;
        config.invert.modals = true;
        config.invert.titlebars = false;
        config.invert.toolbars = false;

        config.bg.opacity = 0xff;
        config.bg.modal.opacity = 250;
        config.bg.blur = false;
//         config.btn.tool.disabledStyle = readInt(BTN_DISABLED_TOOLS);
        config.frame.roundness = 2;
        config.bg.ringOverlay = 4;
    }
    initMetrics();
    setProperty("VirtualityStyleRevision", virtuality_revision());
}

