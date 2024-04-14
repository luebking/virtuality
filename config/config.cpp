/*
 *   Bespin style config for Qt4 and swiss army cli tool
 *   Copyright 2009-2014 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QApplication>
#include <QColorDialog>
#include <QFileDialog>
#include <QDir>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QSettings>
#include <QStyleFactory>
#include <QTimer>
// #include <QDialogButtonBox>
// #include <QInputDialog>
// #include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QValidator>

#include "kdeini.h"
#include "../config.defaults"
#include "config.h"

typedef QMap<QString,QString> StringMap;

/** This function declares the kstyle config plugin, you may need to adjust it
for other plugins or won't need it at all, if you're not interested in a plugin */
extern "C"
{
    Q_DECL_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
    {
        /**Create our config dialog and reply it as plugin
        This is slightly different from the setup in a standalone dialog at the
        bottom of this file*/
        return new Config(parent);
    }
}

static QString defInfo1(
"<b>Virtuality</b><hr>"
"<p> Style Version : %style<br>"
"Config Version: %config"
"</p>"
"<p>"
"&copy;&nbsp;2009-2014<br>by Thomas&nbsp;L&uuml;bking<br>"
"Carved out of my Bespin style code"
"</p>"
// "Visit <a href=\"http://cloudcity.sourceforge.net\">CloudCity.SourceForge.Net</a>"
);


/** Intenal class for the PW Char entry - not of interest */

static ushort unicode(const QString &string)
{
    if (string.length() == 1)
        return string.at(0).unicode();
    uint n = string.toUShort();
    if (!n)
        n = string.toUShort(0,16);
    if (!n)
        n = string.toUShort(0,8);
    return n;
}

class UniCharValidator : public QValidator {
public:
    UniCharValidator( QObject * parent ) : QValidator(parent){}
    virtual State validate ( QString & input, int & ) const
    {
        if (input.length() == 0)
            return Intermediate;
        if (input.length() == 1)
            return Acceptable;
        if (input.length() == 2 && input.at(0) == '0' && input.at(1).toLower() == 'x')
            return Intermediate;
        if (unicode(input))
            return Acceptable;
        return Invalid;
    }
};

extern const QString virtuality_revision();

/** The Constructor - your first job! */
Config::Config(QWidget *parent) : BConfig(parent), loadedPal(0), infoIsManage(false)
{
    /** Setup the UI and geometry */
    ui.setupUi(this);
    connect (ui.tabAnimDuration, SIGNAL(valueChanged(int)), ui.tabAnimDurationLabel, SLOT(setNum(int)));

    /** Some special stuff */
    QEvent event(QEvent::PaletteChange);
    changeEvent(&event);
    ui.info->setOpenExternalLinks( true ); /** i've an internet link here */
    ui.info->setMinimumWidth(160);

    const QPalette::ColorGroup groups[3] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };
    ui.info->viewport()->setAutoFillBackground(false);
    QPalette pal = ui.info->palette();
    for (int i = 0; i < 3; ++i)
    {
        pal.setColor(groups[i], QPalette::Base, pal.color(groups[i], QPalette::Window));
        pal.setColor(groups[i], QPalette::Text, pal.color(groups[i], QPalette::WindowText));
    }
    ui.info->setPalette(pal);

    /** set up color page, not of interest */
    QColorDialog *cd = new QColorDialog(this);
    cd->hide();
    connect ( ui.colorButton, SIGNAL(clicked()), cd, SLOT(show()) );
    connect ( ui.colorButton, SIGNAL(clicked()), cd, SLOT(raise()) );
    ui.role_window->installEventFilter(this);
    ui.role_windowText->installEventFilter(this);
    ui.role_highlight->installEventFilter(this);
    ui.role_highlightedText->installEventFilter(this);
    QTimer::singleShot( 50, this, SLOT(initColors()) );

    /** fill some comboboxes, not of interest */

    QSettings csettings("BE", "Config");
    QStringList strList = csettings.value ( "UserPwChars", QStringList() ).toStringList();
    ushort n;
    foreach (QString str, strList)
    {
        n = str.toUShort(0,16);
        if (n)
            ui.pwEchoChar->addItem(QChar(n), n);
    }
    strList.clear();
    ui.pwEchoChar->addItem(QChar(0x26AB), 0x26AB);
    ui.pwEchoChar->addItem(QChar(0x2022), 0x2022);
    ui.pwEchoChar->addItem(QChar(0x2055), 0x2055);
    ui.pwEchoChar->addItem(QChar(0x220E), 0x220E);
    ui.pwEchoChar->addItem(QChar(0x224E), 0x224E);
    ui.pwEchoChar->addItem(QChar(0x25AA), 0x25AA);
    ui.pwEchoChar->addItem(QChar(0x25AC), 0x25AC);
    ui.pwEchoChar->addItem(QChar(0x25AC), 0x25AC);
    ui.pwEchoChar->addItem(QChar(0x25A0), 0x25A0);
    ui.pwEchoChar->addItem(QChar(0x25CF), 0x25CF);
    ui.pwEchoChar->addItem(QChar(0x2605), 0x2605);
    ui.pwEchoChar->addItem(QChar(0x2613), 0x2613);
    ui.pwEchoChar->addItem(QChar(0x26A1), 0x26A1);
    ui.pwEchoChar->addItem(QChar(0x2717), 0x2717);
    ui.pwEchoChar->addItem(QChar(0x2726), 0x2726);
    ui.pwEchoChar->addItem(QChar(0x2756), 0x2756);
    ui.pwEchoChar->addItem(QChar(0x2756), 0x2756);
    ui.pwEchoChar->addItem(QChar(0x27A4), 0x27A4);
    ui.pwEchoChar->addItem(QChar(0xa4), 0xa4);
    ui.pwEchoChar->addItem("|", '|');
    ui.pwEchoChar->addItem(":", ':');
    ui.pwEchoChar->addItem("*", '*');
    ui.pwEchoChar->addItem("#", '#');
    ui.pwEchoChar->lineEdit()-> setValidator(new UniCharValidator(ui.pwEchoChar->lineEdit()));
    connect (ui.pwEchoChar->lineEdit(), SIGNAL(returnPressed()), this, SLOT (learnPwChar()));
    ui.pwEchoChar->setInsertPolicy(QComboBox::NoInsert);

    /** 1. name the info browser, you'll need it to show up context help
    Can be any QTextBrowser on your UI form */
    setInfoBrowser(ui.info);
    /** 2. Define a context info that is displayed when no other context help is demanded */
    QVariant sv = qApp->style()->property("VirtualityStyleRevision");
    if (!sv.isValid()) {
        if (QStyle *virtuality = QStyleFactory::create("virtuality")) {
            sv = virtuality->property("VirtualityStyleRevision");
            delete virtuality;
        }
    }
    defInfo1.replace("%style", sv.toString());
    defInfo1.replace("%config", virtuality_revision());
    setDefaultContextInfo(defInfo1);

    /** handleSettings(.) tells BConfig to take care (save/load) of a widget
    In this case "ui.bgMode" is the widget on the form,
    "BackgroundMode" specifies the entry in the ini style config file and
    "3" is the default value for this entry*/
    handleSettings(ui.roundness, ROUNDNESS);
    handleSettings(ui.thinLines, STROKE_WIDTH);

    handleSettings(ui.windowOpacity, BG_OPACITY);
    connect(ui.windowOpacity, SIGNAL(valueChanged(int)), SLOT(alignLabel()));
    handleSettings(ui.modalOpacity, BG_MODAL_OPACITY);
    connect(ui.modalOpacity, SIGNAL(valueChanged(int)), SLOT(alignLabel()));
    handleSettings(ui.blurTranslucent, BG_BLUR);

    handleSettings(ui.btnMinHeight, BTN_MIN_HEIGHT);

    handleSettings(ui.pwEchoChar, INPUT_PWECHOCHAR);

    handleSettings(ui.leftHanded, LEFTHANDED);
    handleSettings(ui.macStyle, MACSTYLE);
    handleSettings(ui.dialogBtnLayout, DIALOG_BTN_LAYOUT);
    setContextHelp(ui.dialogBtnLayout, "<b>Dialog button layout</b><hr>\
    This controls the order of the Yes/No/Cancel buttons, use with care, you might accidently\
    misclick if you revert it from e.g. KDE to OS X");

    handleSettings(ui.dialogWatermark, BG_RING_OVERLAY);

    handleSettings(ui.invertModals, INVERT_MODALS);
    setContextHelp(ui.invertModals, "<b>Invert modal Dialogs</b><hr>\
    Invert colors of dialogs that block access to their main window");

    handleSettings(ui.invertMenubars, INVERT_MENUBARS);
    handleSettings(ui.invertToolbars, INVERT_TOOLBARS);
    handleSettings(ui.invertDocks, INVERT_DOCKS);
    setContextHelp(ui.invertDocks, "<b>Invert Docks</b><hr>\
    Invert colors of dock windows like eg. the dolphin side panels");

    handleSettings(ui.invertHeaders, INVERT_HEADERS);
    setContextHelp(ui.invertHeaders, "<b>Invert Headers</b><hr>\
    Invert colors of tabbars and itemview headers");

    handleSettings(ui.invertPopups, INVERT_MENUS);
    handleSettings(ui.invertTitlebars, INVERT_TITLEBARS);

    handleSettings(ui.tabAnimDuration, TAB_DURATION);
    handleSettings(ui.tabTransition, TAB_TRANSITION);

    handleSettings(ui.menuShowIcons, MENU_SHOWICONS);
    handleSettings(ui.menuIndent, MENU_INDENT);

    handleSettings(ui.hackFixGwenview, HACK_FIX_GWENVIEW);
    setContextHelp(ui.hackFixGwenview, "<b>Gwenview's Thumbview</b><hr>\
    There are two major issues with the thumbnail browsing in Gwenview.\
    <ol><li>It scrolls 3 lines on one wheel event and you waste time on redetecting context</li>\
    <li>It moves to another (upper) position when you leave the image view</li></ol>\
    If those were bugs, one could easily fix them, but they exists ever since...<br>\
    So you can fix them here >-)");

    handleSettings(ui.kTitleWidgets, HACK_TITLE_WIDGET);
    setContextHelp(ui.kTitleWidgets, "<b>KTitleWidget</b><hr>I'd' like my headers inverted and center aligned...");

    handleSettings(ui.hackWindowMove, HACK_WINDOWMOVE);
    setContextHelp(ui.hackWindowMove, "<b>Easy Window Draging</b><hr>\
    Usually you'll have to hit the titlebar in order to drag around a window.<br>\
    This one allows you to drag the window by clicking many empty spaces.<br>\
    Currently supported items:<br>\
    - Dialogs<br>\
    - Menubars<br>\
    - Toolbars (including disabled buttons)<br>\
    - Docks<br>\
    - Groupboxers<br>\
    - Mainwindows<br>\
    - Statusbars<br>\
    - SMPlayer/DragonPlayer Video areas<br>");

    handleSettings(ui.hackPanning, HACK_PANNING);
    setContextHelp(ui.hackPanning, "<b>Panning Viewports</b><hr>\
    <b>WARNING</b><br>\
    This will limit normal mouse usage on those viewports, discouraged but for touchscreens.<hr>\
    Affected are (atm.) IconViews, TextEdits, Dolphin and QWebView<br>\
    Pannig is skipped if a modifier (ctrl, shift or alt) is held or you tap the same element\
    twice withing 333ms<br>\
    Clicking items will remain operative, <b>dragging will not</b><br>\
    Use the double tap to cause a drag, ie. tap next to and then press and hold the icon");

    handleSettings(ui.suspendFsVideoCompositing, HACK_SUSPEND_FULLSCREEN_PLAYERS);
    setContextHelp(ui.suspendFsVideoCompositing, "<b>Suspend Fullscreen Video Compositing</b><hr>\
    Compositing, esp. the OpenGL variant, can have a significant conversion overhead, depending\
    on the window size and the update frequency - unfortunately that especially holds for fullscreen\
    video playback, when often the CPU is worried with decoding an 1080p x264 video as well.<br>\
    This will catch some video clients when entering the fullscreen mode and disable compositing until\
    fullscreen mode exits.<br>\
    Will likely cause some temporarily flicker when entering/leaving the fullscreen mode but can safe a lot\
    of cpu when it really matters.");

    handleSettings(ui.hackPowerdevilOSD, HACK_BRIGHTNESS_OSD);


    handleSettings(ui.lockToolbars, HACK_TOOLBAR_LOCKING);
    setContextHelp(ui.lockToolbars, "<b>Lock Toolbars</b><hr>\
    KDE toolbars allow you (among other) to lock their position, plain Qt Toolbars\
    (like in eg. arora) don't.<br>\
    This locks all Qt Toolbars and adds a config item, accessible by pressing CTRL\
    and rightclicking the Toolbar.");

    handleSettings(ui.lockDocks, HACK_DOCK_LOCKING);
    setContextHelp(ui.lockDocks, "<b>Lock Dockwidgets</b><hr>\
    Qt mainwindows provide a (quite ;-) powerfull way to arrange parts of the application\
    (like the sidebars in Dolphin or K3b) but while you can lock them in eg. Amarok or K3b,\
    they'll remain always movable in most other applications.<br>\
    Checking this will initially lock those docks and allow you to toggle them unlocked\
    by pressing CRTL+ALT+D.");

    /** setContextHelp(.) attaches a context help string to a widget on your form */

    strList <<
        "<b>Jump</b><hr>No transition at all - fastest but looks stupid" <<
        "<b>ScanlineBlend</b><hr>Similar to CrossFade, but won't require Hardware acceleration." <<
        "<b>SlideIn</b><hr>The new tab falls down from top" <<
        "<b>SlideOut</b><hr>The new tab rises from bottom" <<
        "<b>RollIn</b><hr>The new tab appears from Top/Bottom to center" <<
        "<b>RollOut</b><hr>The new tab appears from Center to Top/Bottom" <<
        "<b>OpenVertically</b><hr>The <b>old</b> Tab slides <b>out</b> to Top and Bottom" <<
        "<b>CloseVertically</b><hr>The <b>new</b> Tab slides <b>in</b> from Top and Bottom" <<
        "<b>OpenHorizontally</b><hr>The <b>old</b> Tab slides <b>out</b> to Left and Right" <<
        "<b>CloseHorizontally</b><hr>The <b>new</b> Tab slides <b>in</b> from Left and Right" <<
        "<b>CrossFade</b><hr>What you would expect - one fades out while the other fades in.<br>\
        This is CPU hungry - better have GPU Hardware acceleration.";

    setContextHelp(ui.tabTransition, strList);


    setContextHelp(ui.pwEchoChar, "<b>Pasword Echo Character</b><hr>\
                    The character that is displayed instead of the real text when\
                    you enter e.g. a password.<br>\
                    You can enter any char or unicode number here.\
                    <b>Notice:</b> That not all fontsets may provide all unicode characters!");

    setContextHelp(ui.tabAnimDuration, "<b>Tab Transition Duration</b><hr>\
                    How long the transition lasts.");


    setContextHelp(ui.leftHanded, "<b>Are you a Flanders?</b><hr>\
                    Some people (\"Lefties\") prefer a reverse orientation of eg.\
                    Comboboxes or Spinboxes.<br>\
                    If you are a Flanders, check this - maybe you like it.<br>\
                    (Not exported from presets)");

    setContextHelp(ui.macStyle, "<b>Mac Style Behaviour</b><hr>\
                    This currently affects the appereance of Wizzards and allows\
                    you to activate items with a SINGLE mouseclick, rather than\
                    the M$ DOUBLEclick thing ;)<br>\
                    (Not exported from presets)");


    /** setQSetting(.) tells BConfig to store values at
    "Company, Application, Group" - these strings are passed to QSettings */
    setQSetting("BE", "Style", "Current");

    /** you can call loadSettings() whenever you want, but (obviously)
    only items that have been propagated with handleSettings(.) are handled !!*/
    loadSettings();

    /** ===========================================
    You're pretty much done here - simple eh? ;)
        The following code reimplemets some BConfig functions
    (mainly to manage Qt color setttings)

    if you want a standalone app you may want to check the main() funtion
    at the end of this file as well - but there's nothing special about it...
        =========================================== */
    resize(1024,768);
}

void
Config::initColors()
{
    setColorsFromPalette( QApplication::palette() );
}

void
Config::changeEvent(QEvent *event)
{
    if (event->type() != QEvent::PaletteChange)
        return;

    myColorsHaveChanged = true;
}

bool
Config::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::DragEnter )
    {
        QDropEvent *de = static_cast<QDragMoveEvent*>(e);
        if (de->mimeData()->hasColor())
            de->accept();
        else
            de->ignore();
        return false;
    }
    if ( e->type() == QEvent::Drop )
    {
        QDropEvent *de = static_cast<QDropEvent*>(e);
        if (de->mimeData()->hasColor())
        {
            QColor c = qvariant_cast<QColor>(de->mimeData()->colorData());
            QWidget *w = static_cast<QWidget*>(o);
            QPalette pal = w->palette();
            bool fg = false;
            QWidget *counter = 0L;
            if ( w == ui.role_window ) counter = ui.role_windowText;
            else if ( w == ui.role_highlight ) counter = ui.role_highlightedText;
            else if ( (fg = (w == ui.role_windowText)) ) counter = ui.role_window;
            else if ( (fg = (w == ui.role_highlightedText)) ) counter = ui.role_highlight;
            else return false; // whatever this might be...

            pal.setColor( fg ? w->foregroundRole() : w->backgroundRole(), c );
            w->setPalette(pal);
            counter->setPalette(pal);
            // TODO this doesn't work (the app palette didn't change) - need to figure whether i want it to...
            // requires a palette in sync with the color dialog
//             myColorsHaveChanged = true;
            emit changed(true);
            emit changed();
        }
        return false;
    }
    return BConfig::eventFilter(o, e);
}


QStringList
Config::colors(const QPalette &pal, QPalette::ColorGroup group)
{
    QStringList list;
    for (int i = 0; i < QPalette::NColorRoles; i++)
        list << pal.color(group, (QPalette::ColorRole) i).name();
    return list;
}

QVariant
Config::variant(const QObject *w) const
{
    if (w == ui.thinLines)
        return QVariant(ui.thinLines->isChecked() ? 1 : 2);
    return BConfig::variant(w);
}

bool Config::setVariant(QObject *w, const QVariant &v) const
{
    if (w == ui.thinLines) {
        ui.thinLines->setChecked(v.toInt() == 1);
        return true;
    }
    return BConfig::setVariant(w, v);
}

void
Config::updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list)
{
    int max = QPalette::NColorRoles;
    if (max > list.count())
    {
        qWarning("The demanded palette seems to be incomplete!");
        max = list.count();
    }
    for (int i = 0; i < max; i++)
        pal.setColor(group, (QPalette::ColorRole) i, list.at(i));
}


class BStyle : public QStyle
{
    public:
        BStyle() : QStyle (){}
        virtual void init(const QSettings *settings) = 0;
};


bool
Config::save()
{
    if ( !BConfig::save() )
        return false;

    /** save the palette loaded from store to qt configuration */
    if (!loadedPal)
        loadedPal = new QPalette;
    applyPalette( loadedPal );
    savePalette(*loadedPal);
    return true;
}

static QColor mid(const QColor &c1, const QColor &c2, int w1 = 1, int w2 = 1)
{
    int sum = (w1+w2);
    return QColor((w1*c1.red() + w2*c2.red())/sum,
                    (w1*c1.green() + w2*c2.green())/sum,
                    (w1*c1.blue() + w2*c2.blue())/sum,
                    (w1*c1.alpha() + w2*c2.alpha())/sum);
}


void
Config::setColorsFromPalette( const QPalette &pal )
{
    QPalette p = ui.role_window->palette();
    p.setColor( ui.role_window->backgroundRole(), pal.color( QPalette::Active, QPalette::Window ) );
    p.setColor( ui.role_windowText->foregroundRole(), pal.color( QPalette::Active, QPalette::WindowText ) );
    ui.role_window->setPalette( p );
    ui.role_windowText->setPalette( p );

    p = ui.role_highlight->palette();
    p.setColor( ui.role_highlight->backgroundRole(), pal.color( QPalette::Active, QPalette::Highlight ) );
    p.setColor( ui.role_highlightedText->foregroundRole(), pal.color( QPalette::Active, QPalette::HighlightedText ) );
    ui.role_highlight->setPalette( p );
    ui.role_highlightedText->setPalette( p );
}

void
Config::applyPalette( QPalette *pal )
{
    if ( !pal )
    {
        if (!loadedPal)
            loadedPal = new QPalette;
        pal = loadedPal;
    }
    pal->setColor( QPalette::Window, ui.role_window->palette().color( ui.role_window->backgroundRole() ) );
    pal->setColor( QPalette::WindowText, ui.role_windowText->palette().color( ui.role_windowText->foregroundRole() ) );
    pal->setColor( QPalette::Button, ui.role_window->palette().color( ui.role_window->backgroundRole() ) );
    pal->setColor( QPalette::ButtonText, ui.role_windowText->palette().color( ui.role_windowText->foregroundRole() ) );
    pal->setColor( QPalette::Base, ui.role_window->palette().color( ui.role_window->backgroundRole() ) );
    pal->setColor( QPalette::Text, ui.role_windowText->palette().color( ui.role_windowText->foregroundRole() ) );
    pal->setColor( QPalette::Highlight, ui.role_highlight->palette().color( ui.role_highlight->backgroundRole() ) );
    pal->setColor( QPalette::HighlightedText, ui.role_highlightedText->palette().color( ui.role_highlightedText->foregroundRole() ) );
    if ( pal == loadedPal )
    {
        QApplication::setPalette( *loadedPal );
        emit changed(true);
        emit changed();
    }
}

void
Config::savePalette(const QPalette &pal)
{

    // for Qt =====================================
    const char* configs[2] = { "Trolltech", "QtProject" };
    for (int i = 0; i < 2; ++i) {
        QSettings settings(configs[i]);
        settings.beginGroup("Qt");
        settings.beginGroup("Palette");

        settings.setValue ( "active", colors(pal, QPalette::Active) );
        settings.setValue ( "inactive", colors(pal, QPalette::Inactive) );
        settings.setValue ( "disabled", colors(pal, QPalette::Disabled) );

        settings.endGroup(); settings.endGroup();
    }

    // and KDE ==== I'm now gonna mourn a bit and not allways be prudent...:
    //
    // yeah - 5000 new extra colors for a style that relies on very restricted
    // color assumptions (kstyle, plastik and thus oxygen...) - sure...
    //
    // [ UPDATE:this is meanwhile fixed... ]
    // and please don't sync the Qt palette, ppl. will certainly be happy to
    // make color setting in KDE first and then in qtconfig for QApplication...
    // [ End update ]
    //
    // Ok, KDE supports extra DecorationFocus and DecorationHover
    //                        --------------      ---------------
    // -- we don't so we won't sync
    //
    // next, there's ForegroundLink and ForegroundVisited in any section
    //               -------------      ----------------
    // -- we just map them to QPalette::Link & QPalette::LinkVisited
    //
    // third, there're alternate backgroundcolors for all sections - sure:
    //                 ------------------------        ---------
    // my alternate button color: 1st button blue, second red, third blue...
    // -- we'll do what we do for normal alternate row colors and slightly shift
    // to the foreground....
    //
    // there's a ForegroundActive color that is by default... PINK???
    //          -----------------
    // what a decent taste for aesthetics... &-}
    // -- we just leave it as it is, cause i've no idea what it shall be good for
    // (active palette text - that's gonna be fun ;-)
    //
    // last there're ForegroundNegative, ForegroundNeutral and ForegroundPositive
    //               ------------------  ----------------      -----------------
    // what basically means: shifted to red, yellow and green...
    //
    // oh, and of course there NEEDS to be support for special chars in the
    // KDE ini files - plenty. who could ever life without keys w/o ':' or '$'
    // so we cannot simply use QSettings on a KDE ini file, thus we'll use our
    // own very... slim... ini parser, ok, we just read the file group it by
    // ^[.* entries, replace the color things and than flush the whole thing back
    // on disk

    KdeIni *kdeglobals = KdeIni::open("kdeglobals");
    if (!kdeglobals)
    {
        qWarning("Warning: kde4-config not found or \"--path config\" flag does not work\nWarning: No KDE support!");
        return;
    }
    const QString prefix("Colors:");
    const int numItems = 5;
    static const char *items[numItems] =
    {
        "Button", "Selection", "View", "Window", "Tooltip"
    };
    static const QPalette::ColorRole roles[numItems][2] =
    {
        {QPalette::Button, QPalette::ButtonText},
        {QPalette::Highlight, QPalette::HighlightedText},
        {QPalette::Base, QPalette::Text},
        {QPalette::Window, QPalette::WindowText},
        {QPalette::ToolTipBase, QPalette::ToolTipText}
    };
    for (int i = 0; i < numItems; ++i)
    {
        kdeglobals->setGroup(prefix + items[i]);
        kdeglobals->setValue("BackgroundAlternate", mid(pal.color(QPalette::Active, roles[i][0]),
                                                        pal.color(QPalette::Active, roles[i][1]), 15, 1));
        kdeglobals->setValue("BackgroundNormal", pal.color(QPalette::Active, roles[i][0]));
        kdeglobals->setValue("ForegroundInactive", pal.color(QPalette::Disabled, roles[i][1]));
        kdeglobals->setValue("ForegroundLink", pal.color(QPalette::Active, QPalette::Link));
        kdeglobals->setValue("ForegroundNegative", mid(pal.color(QPalette::Active, roles[i][1]), Qt::red));
        kdeglobals->setValue("ForegroundNeutral", mid(pal.color(QPalette::Active, roles[i][1]), Qt::yellow));
        kdeglobals->setValue("ForegroundNormal", pal.color(QPalette::Active, roles[i][1]));
        kdeglobals->setValue("ForegroundPositive", mid(pal.color(QPalette::Active, roles[i][1]), Qt::green));
        kdeglobals->setValue("ForegroundVisited", pal.color(QPalette::Active, QPalette::LinkVisited));
    }
    kdeglobals->close();
    delete kdeglobals;

}

void Config::setLogo(const QPixmap &pix)
{
    ui.logo->setPixmap(pix);
}

void
Config::learnPwChar()
{
    ushort n = unicode(ui.pwEchoChar->lineEdit()->text());
    if (ui.pwEchoChar->findData(n) != -1)
        return;
    ui.pwEchoChar->insertItem(0, QString(QChar(n)), QVariant(n));
    QSettings settings("BE", "Config");
    QStringList list = settings.value ( "UserPwChars", QStringList() ).toStringList();
    list << QString::number( n, 16 );
    settings.setValue("UserPwChars", list);
}

inline static QString percent(int c)
{
    QString ret = (c == 0xff ? "" : " ");
    ret += QString::number(c / 2.55, 'f', 2) + " %";
    return ret;
}

void
Config::alignLabel()
{
    if (sender() == ui.windowOpacity)
        ui.windowOpacityLabel->setText(percent(ui.windowOpacity->value()));
    else if (sender() == ui.modalOpacity)
        ui.modalOpacityLabel->setText(percent(ui.modalOpacity->value()));
}

