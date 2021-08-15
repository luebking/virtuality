/*
 *   Bespin style config for Qt4 and swiss army cli tool
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

#include "config.h"
#include "ui_uiDemo.h"
#include "dialog.h"
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QSettings>
#include <QStyleFactory>
#include <QTimer>

#define CHAR(_QSTRING_) _QSTRING_.toLocal8Bit().data()
#define RETURN_APP_ERROR int ret = app->exec(); delete window; delete app; return ret

class BStyle : public QStyle
{
public:
    BStyle() : QStyle (){}
    virtual void init(const QSettings *settings) = 0;
};

static int
error(const QString & string)
{
    qWarning("Error: %s", CHAR(string));
    return 1;
}

static int
usage(const char* appname)
{
   printf(
"Usage:\n"
"==========================\n"
"%s [config]\t\t\t\t\t\tConfigure the Virtuality Style\n"
"%s presets\t\t\t\t\t\tList the available Presets\n"
"%s demo [style]\t\t\t\t\tLaunch a demo, you can pass other stylenames\n"
"%s show <some_preset>\t\t\t\tOpen demo dialog with a preset\n"
"%s try <some_config.virtuality>\t\t\t\tTry out an exported setting\n"
"%s sshot <some_file.png> [preset] [width]\t\tSave a screenshot\n"
"%s load <some_preset>\t\t\t\tLoad a preset to current settings\n"
"%s import <some_config.virtuality>\t\t\tImport an exported setting\n"
"%s update <some_config.virtuality>\t\t\tLike import, but overrides existing\n"
"%s export <some_preset> <some_config.virtuality>\tExport an imported setting\n"
"%s listStyles \t\t\t\t\tList all styles on this System\n"
"%s loadPaletteFrom <style>\t\t\t\tLoad and store the default palette of a style\n"
"%s read <style|deco> <key> [value]\t\t\tRead a config key (defaulting to optional value)\n"
"%s write <style|deco> <key> <value>\t\t\tWrite a config key to value\n"
"%s delete <style|deco> <key>\t\t\tRemove a config key\n"
"%s list <style|deco> [filter]\t\t\tList config keys matching the optional filter\n"
"%s revision \t\t\t\tPrint virtuality version string\n",
appname, appname, appname, appname, appname, appname, appname, appname,
appname, appname, appname, appname, appname, appname, appname, appname, appname );
   return 1;
}

enum Mode
{
    Invalid = 0, Configure, Presets, Import, Update, Export, Load, Demo, Try, Screenshot, ListStyles, Show, LoadPalette, ReadSetting, WriteSetting, DeleteSetting, ListSettings, Revision
};

extern const QString virtuality_revision();

static QPixmap logo(int s, QColor c)
{
    QPixmap logoPix(s*4,s*4);
    QPainterPath path;
    path.moveTo(logoPix.rect().center());
    path.arcTo(logoPix.rect(), 90, 270);
    path.lineTo(logoPix.rect().right(), logoPix.rect().y()+4*s/3);
    path.lineTo(logoPix.rect().right()-s, logoPix.rect().y()+4*s/3);
    path.lineTo(logoPix.rect().center().x() + s/2, logoPix.rect().center().y());
    path.lineTo(logoPix.rect().center());
    path.closeSubpath();
    path.addEllipse(logoPix.rect().right()-3*s/2, logoPix.rect().y(), s, s);
    logoPix.fill(Qt::transparent);
    QPainter p(&logoPix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawPath(path);
    p.end();
    return logoPix;
}

int
main(int argc, char *argv[])
{
    Mode mode = Configure;
    QApplication *app = 0;
    QString title;
    if (argc > 1)
    {
        mode = Invalid;
        if (!qstricmp( argv[1], "config" )) mode = Configure;
        else if (!qstricmp( argv[1], "presets" )) mode = Presets;
        else if (!qstricmp( argv[1], "import" )) mode = Import;
        else if (!qstricmp( argv[1], "update" )) mode = Update;
        else if (!qstricmp( argv[1], "export" )) mode = Export;
        else if (!qstricmp( argv[1], "demo" )) mode = Demo;
        else if (!qstricmp( argv[1], "try" )) mode = Try;
        else if (!qstricmp( argv[1], "show" )) mode = Show;
        else if (!qstricmp( argv[1], "sshot" )) mode = Screenshot;
        else if (!qstricmp( argv[1], "load" )) mode = Load;
        else if (!qstricmp( argv[1], "listStyles" )) mode = ListStyles;
        else if (!qstricmp( argv[1], "loadPaletteFrom" )) mode = LoadPalette;
        else if (!qstricmp( argv[1], "read" )) mode = ReadSetting;
        else if (!qstricmp( argv[1], "write" )) mode = WriteSetting;
        else if (!qstricmp( argv[1], "delete" )) mode = DeleteSetting;
        else if (!qstricmp( argv[1], "list" )) mode = ListSettings;
        else if (!qstricmp( argv[1], "revision" )) mode = Revision;
    }

    switch (mode)
    {
    case Configure:
    {
        app = new QApplication(argc, argv);
        Config *config = new Config;
        BConfigDialog *window = new BConfigDialog(config, BConfigDialog::All &
                                                                  ~(BConfigDialog::Demo | BConfigDialog::Import | BConfigDialog::Export));
        QColor c(window->palette().color(QPalette::Active, QPalette::WindowText));
        config->setLogo(logo(32, c));
        window->show();
        RETURN_APP_ERROR;
    }
    case LoadPalette:
    {
        if (argc < 3)
            return error(QString("you lack <some_style>. Try \"%1 listStyles\"").arg(argv[0]));
        app = new QApplication(argc, argv);
        QStyle *style = QStyleFactory::create( argv[2] );
        if (!style)
        {
            delete app;
            return error(QString("Style \"%1\" does not exist. Try \"%2 listStyles\"").arg(argv[2]).arg(argv[0]));
        }
        Config::savePalette(style->standardPalette());
        delete app; delete style;
        break;
    }
    case Demo:
    {
        if (title.isEmpty())
            title = "Virtuality Demo";

        if (mode == Demo && argc > 2) {   // allow setting another style
            if (!app) app = new QApplication(argc, argv);
            app->setStyle(argv[2]);
            app->setPalette(app->style()->standardPalette());
            title = QString("Virtuality Demo / %1 Style").arg(argv[2]);
        }
        if (!app)
            app = new QApplication(argc, argv);
        Dialog *window = new Dialog;
        Ui::Demo ui;
        ui.setupUi(window);
        QIcon icn(logo(8, ui.toolButton->palette().color(QPalette::Active, ui.toolButton->foregroundRole())));
        ui.toolButton->setIcon(icn);
        ui.toolButton_2->setIcon(icn);
        ui.toolButton_3->setIcon(icn);
        ui.toolButton_4->setIcon(icn);
        ui.toolButton_5->setIcon(icn);
        ui.tabWidget->setCurrentIndex(0);
        QObject::connect (ui.rtl, SIGNAL(toggled(bool)), window, SLOT(setLayoutDirection(bool)));
        window->setWindowTitle ( title );
        window->show();
        RETURN_APP_ERROR;
    }
    case Screenshot:
    {
        if (argc < 3)
            return error("you lack <some_output.png>");
        app = new QApplication(argc, argv);
        Dialog *window = new Dialog;
        Ui::Demo ui;
        ui.setupUi(window);
        ui.tabWidget->setCurrentIndex(0);
        ui.buttonBox->button(QDialogButtonBox::Ok)->setAttribute( Qt::WA_UnderMouse );
        ui.demoBrowser->setAttribute( Qt::WA_UnderMouse );
//       ui.demoLineEdit->setFocus(Qt::MouseFocusReason);

        QImage image(window->size(), QImage::Format_RGB32);
        window->showMinimized();
        window->QDialog::render(&image);
        if (argc > 4)
            image = image.scaledToWidth(atoi(argv[4]), Qt::SmoothTransformation);
        image.save ( argv[2], "png" );
        delete window;
        delete app;
        return 0;
    }
    case ListStyles:
    {
        foreach (QString string, QStyleFactory::keys())
            printf("%s\n", CHAR(string));
        return 0;
    }
    case ReadSetting:
    case WriteSetting:
    case DeleteSetting:
    case ListSettings:
    {
        bool e = false;
        bool deco = false;
        if (argc < (mode == WriteSetting ? 5 : (mode == ListSettings ? 3 : 4)))
            e = true;
        else if (!qstricmp( argv[2], "deco" ))
            deco = true;
        else if (qstricmp( argv[2], "style" ))
            e = true;
        if (e)
            return error(QString("try %1 %2 <style|deco> <key> [value]").arg(argv[0]).arg(argv[1]));
        QSettings config("BE", "Style");
        config.beginGroup(deco ? "Deco" : "Current");
        const QString key = QString::fromLocal8Bit(argv[3]);
        if (mode == ReadSetting)
            printf("%s\n", CHAR(config.value(key, argc > 4 ? argv[4] : QVariant()).toString()));
        else if (mode == DeleteSetting) {
            if (config.contains(key))
                config.remove(key);
            else
                return error(QString("there is no key \"%1\" in \"%2\"").arg(key).arg(argv[2]));
        }
        else if (mode == ListSettings) {
            QStringList keys = config.childKeys();
            if (argc > 3) {
                keys = keys.filter(key, Qt::CaseInsensitive);
            }
            foreach (const QString &k, keys)
                printf("%s : %s\n", CHAR(k), CHAR(config.value(k).toString()));
        }
        else
        {
            const bool added = !config.contains(QString::fromLocal8Bit(argv[3]));
            config.setValue(QString::fromLocal8Bit(argv[3]), QString::fromLocal8Bit(argv[4]));
            if (added)
                printf("added new key \"%s\" set to \"%s\"\n", argv[3], argv[4]);
            if (deco) // update kwin
                QProcess::startDetached("qdbus", QStringList() << "org.kde.kwin" << "/KWin" << "reconfigure" );
        }
        return 0;
    }
    case Revision: {
        QApplication a(argc, argv);
        QVariant sv = a.style()->property("VirtualityStyleRevision");
        if (!sv.isValid()) {
            if (QStyle *virtuality = QStyleFactory::create("virtuality")) {
                sv = virtuality->property("VirtualityStyleRevision");
                delete virtuality;
            }
        }
        printf("Revision (Config): %s\n"
               "Revision (Style):  %s\n", CHAR(virtuality_revision()), CHAR(sv.toString()));
        return 0;
    }
    default:
        return usage(argv[0]);
    }
}
