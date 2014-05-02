/*
 *   Bespin managed confg dialog
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

#ifndef BCONFIG_H
#define BCONFIG_H

#include <QDialog>
#include <QMap>
#include <QVariant>
class QComboBox;
class QTextBrowser;
class QSettings;

class BConfig : public QWidget
{
    Q_OBJECT
public:
    BConfig(QWidget *parent = 0L);

    /**
    * Query the default value of a handled Widget (that you set initially)
    *
    * @param w The widget of interest
    */
    virtual QVariant defaultValue(QObject *w) const;

    /**
    * Handle the settings for a QWidget
    *
    * @param w The widget of interest
    * @param entry The String that is used as key in the config file
    * @param defaultValue The value the setting should have by default (i.e. if the user yet did not set one)
    */
    virtual void handleSettings(QObject *w, const QString entry, QVariant defaultValue);

    /**
    * Query the initial value of a handled Widget (the value that was read from the config by loadSettings() or the default value if the entry wasn't set yet)
    *
    * @param w The widget of interest
    */
    virtual QVariant initialValue(QObject *w) const;

    /**
    * Query the currently saved value - same as initialValue() *unless* save() has been called
    *
    * @param w The widget of interest
    */
    virtual QVariant savedValue(QObject *w) const;

    /**
    * Set the (richtext) string that should be displayd as help when the config widget is hovered
    *
    * @param w The widget of interest
    * @param help the help string
    */
    virtual void setContextHelp(QObject *w, QString help);

    /**
    * Same as above, but allows to set a string for each entry of a combobox
    *
    * @param w The widget of interest
    * @param strings the help strings
    */
    virtual void setContextHelp(QComboBox *c, QStringList & strings);

    /**
    * Define the QTextBrowser that shall be used as help viewer, i.e. where the help strings for config widgets are shown
    *
    * @param browser Pointer to a QTExtBrowser - if NULL, the online help is omitted (but the set help strings remain, so you can setInfoBrowser(NULL); setInfoBrowser(anotherBrowser);)
    */
    virtual void setInfoBrowser(QTextBrowser *browser);

    /**
    * Set which config parameters shall be used for managing the config entries - they're simply passesd to QSettings
    * i.e. setQSetting("MyCompany", "MyProduct", "Product group");
    */
    virtual void setQSetting(const QString organisation, const QString application, const QString group);

    /**
    * Set the default contexthelp, i.e. what is displayed in the infoBrowser while no handled config widget is hovered
    *
    * @param info The default context help - good place for shameless selfpromotion! ;)
    */
    virtual void setDefaultContextInfo(QString info);
protected:
    typedef struct {
        QVariant defaultValue;
        QVariant initialValue;
        QVariant savedValue;
        QString entry;
    } SettingInfo;
    virtual bool eventFilter ( QObject * watched, QEvent * event );
    virtual void loadSettings(QSettings *settings = 0, bool updateInitValue = true, bool merge = false);
    virtual bool _save(QSettings *settings = 0, bool makeDirty = true);
    QVariant variant(const QObject *w) const;
    bool setVariant(QObject *w, const QVariant &v) const;
signals:
    void changed(bool);
    void changed();
public slots:
    virtual bool save();
    virtual void defaults();
    virtual void reset();
    virtual void import();
    virtual void saveAs();
protected slots:
    void checkDirty();
    void resetInfo();
private slots:
    void setComboListInfo(int index);
private:
    Q_DISABLE_COPY(BConfig)
    bool infoItemHovered, infoDirty;
    QTextBrowser *_infoBrowser;
    QMap<QObject*, SettingInfo> _settings;
    QMap<QObject*, QString> _contextHelps;
    QMap<QComboBox*, QStringList> _comboHelps;
    QString _qsetting[3];
    QString _defaultContextInfo;
};

class BConfigDialog : public QDialog {
    Q_OBJECT
public:
    enum ButtonType {
        Ok = 1, Cancel = 2, Save = 4, Reset = 8,
            Defaults = 16, Import = 32, Export = 64, Demo = 128, All = 255
    };
    BConfigDialog(BConfig *config, uint btns = All, QWidget *parent = 0L);
public slots:
    void accept();
private:
    BConfig *_config;
    Q_DISABLE_COPY(BConfigDialog)
};

#endif
