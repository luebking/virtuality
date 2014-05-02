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

#include "bconfig.h"
#include <QApplication>
#include <QAbstractSlider>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

BConfigDialog::BConfigDialog(BConfig *config, uint btns, QWidget *parent) :
QDialog(parent, Qt::Window), _config(config) {

   QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
   QWidget *btn;

   if (btns & Ok) {
      btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Ok );
      connect(btn, SIGNAL(clicked(bool)), this, SLOT(accept()));
      btn->setDisabled(true);
      connect(config, SIGNAL(changed(bool)), btn, SLOT(setEnabled(bool)));
   }

   if (btns & Save) {
      btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Save );
      connect(btn, SIGNAL(clicked(bool)), config, SLOT(save()));
      btn->setDisabled(true);
      connect(config, SIGNAL(changed(bool)), btn, SLOT(setEnabled(bool)));
   }

   if (btns & Demo) {
      btn = (QWidget*)buttonBox->addButton ( tr("Demo"), QDialogButtonBox::ApplyRole );
      connect(btn, SIGNAL(clicked(bool)), config, SLOT(showDemo()));
   }

   if (btns & Export) {
      btn = (QWidget*)buttonBox->addButton ( tr("Export..."), QDialogButtonBox::ApplyRole );
      connect(btn, SIGNAL(clicked(bool)), config, SLOT(saveAs()));
   }

   if (btns & Import) {
      btn = (QWidget*)buttonBox->addButton ( tr("Import..."), QDialogButtonBox::ActionRole );
      connect(btn, SIGNAL(clicked(bool)), config, SLOT(import()));
   }

   if (btns & Reset) {
      btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Reset );
      connect(btn, SIGNAL(clicked(bool)), config, SLOT(reset()));
      btn->setDisabled(true);
      connect(config, SIGNAL(changed(bool)), btn, SLOT(setEnabled(bool)));
   }

   if (btns & Defaults) {
      btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::RestoreDefaults );
      connect(btn, SIGNAL(clicked(bool)), config, SLOT(defaults()));
   }

   if (btns & Cancel) {
      btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Cancel );
      connect(btn, SIGNAL(clicked(bool)), this, SLOT(reject()));
   }


   QVBoxLayout *vl = new QVBoxLayout;
   vl->addWidget(config);
   vl->addWidget(buttonBox);
   setLayout(vl);
}


void
BConfigDialog::accept()
{
    if ( ( _config && _config->save() ) ||
        QMessageBox::warning( this, "Close anyway?",
        "<qt>Writing the config has failed. Do you want to close the dialog anyway?<hr>"
        "<b>You will loose all setting changes if you click \"Close\"!</qt>",
        QMessageBox::Cancel, QMessageBox::Close ) == QMessageBox::Close )
        QDialog::accept();
}

BConfig::BConfig(QWidget *parent) : QWidget(parent) {
   infoItemHovered = infoDirty = false;
}

void
BConfig::saveAs()
{
    QString filename = QFileDialog::getSaveFileName(parentWidget(),
                       tr("Save Configuration"), QDir::home().path(), tr("Config Files (*.conf *.ini)"));
    QSettings settings(filename, QSettings::IniFormat);
    _save(&settings, false);
}

void BConfig::import() {
   QString filename = QFileDialog::getOpenFileName(parentWidget(),
      tr("Import Configuration"), QDir::home().path(), tr("Config Files (*.conf *.ini)"));
   QSettings settings(filename, QSettings::IniFormat);
   loadSettings(&settings, false);
}

QVariant BConfig::defaultValue(QObject *w) const {
   return _settings.value(w).defaultValue;
}

QVariant BConfig::initialValue(QObject *w) const {
   return _settings.value(w).initialValue;
}

QVariant BConfig::savedValue(QObject *w) const {
   return _settings.value(w).savedValue;
}

void BConfig::handleSettings(QObject *w, QString entry, QVariant value) {
   SettingInfo info;
   info.defaultValue = value;
   info.initialValue = info.savedValue = QVariant();
   info.entry = entry;
   _settings[w] = info;
   if (qobject_cast<QAbstractButton*>(w) || qobject_cast<QGroupBox*>(w))
      connect (w, SIGNAL(toggled(bool)), this, SLOT(checkDirty()));
   else if (qobject_cast<QButtonGroup*>(w))
       connect (w, SIGNAL(buttonClicked(int)), this, SLOT(checkDirty()));
   else if (qobject_cast<QComboBox*>(w))
      connect (w, SIGNAL(currentIndexChanged(int)), this, SLOT(checkDirty()));
   else if (qobject_cast<QAbstractSlider*>(w) || qobject_cast<QSpinBox*>(w))
      connect (w, SIGNAL(valueChanged(int)), this, SLOT(checkDirty()));
   else if (qobject_cast<QLineEdit*>(w) || qobject_cast<QTextEdit*>(w))
      connect (w, SIGNAL(textChanged(const QString &)), this, SLOT(checkDirty()));
}

void BConfig::setDefaultContextInfo(QString info) {
   _defaultContextInfo = info;
}

void BConfig::setContextHelp(QObject *w, QString help) {
   _contextHelps[w] = help;
   w->installEventFilter(this);
}

void BConfig::setContextHelp(QComboBox *c, QStringList & strings) {
   _comboHelps[c] = strings;
   ((QWidget*)c->view())->installEventFilter(this);
   c->installEventFilter(this);
   connect(c, SIGNAL(highlighted(int)), this, SLOT(setComboListInfo(int)));
   connect(c, SIGNAL(activated(int)), this, SLOT(setComboListInfo(int)));
}

void BConfig::setInfoBrowser(QTextBrowser *browser) {
   _infoBrowser = browser;
   _infoBrowser->installEventFilter(this);
}

void BConfig::checkDirty()
{
    bool dirty = false;
    QMap<QObject*, SettingInfo>::iterator i;
    for (i = _settings.begin(); i != _settings.end(); ++i)
    {
//       if (!sender() || (sender() == i.key()))
//       {
            SettingInfo *info = &(i.value());
            dirty = dirty || variant(i.key()) != info->savedValue;

            if (/*sender() ||*/ dirty)
                break;
//       }
    }
    emit changed(dirty);
    if (dirty) emit changed();
}

void BConfig::reset() {
    QMap<QObject*, SettingInfo>::iterator i;
    for (i = _settings.begin(); i != _settings.end(); ++i)
    {
        if (sender() == i.key())
        {
            setVariant(i.key(), (&(i.value()))->defaultValue);
            break;
        }
    }
}

void BConfig::defaults() {
    QMap<QObject*, SettingInfo>::iterator i;
    for (i = _settings.begin(); i != _settings.end(); ++i)
    {
        if (sender() == i.key())
        {
            setVariant(i.key(), (&(i.value()))->defaultValue);
            break;
        }
    }
}

void BConfig::setComboListInfo(int index)
{
    if (index < 0)
        return;
    if (QComboBox *box = qobject_cast<QComboBox*>(sender())) {
        if (index < _comboHelps.value(box).count())
            _infoBrowser->setHtml(_comboHelps.value(box).at(index));
        else
            _infoBrowser->setHtml(_defaultContextInfo);
        infoItemHovered = true;
    }
}

void BConfig::resetInfo() {
   if (infoItemHovered || !infoDirty)
      return;
   _infoBrowser->setHtml(_defaultContextInfo);
   infoDirty = false;
}

void BConfig::setQSetting(const QString organisation, const QString application, const QString group) {
   _qsetting[0] = organisation;
   _qsetting[1] = application;
   _qsetting[2] = group;
}

void BConfig::loadSettings(QSettings *settings, bool updateInit, bool merge) {
   _infoBrowser->setHtml(_defaultContextInfo);
   bool delSettings = false;
   if (!settings) {
      delSettings = true;
      settings = new QSettings(_qsetting[0], _qsetting[1]);
   }

   settings->beginGroup(_qsetting[2]);

   QMap<QObject*, SettingInfo>::iterator i;
   SettingInfo *info; QVariant value;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      info = &(i.value());
      value = settings->value( info->entry, merge ?
                               variant(i.key()) : info->defaultValue);
      if (updateInit)
         info->savedValue = info->initialValue = value;
      setVariant(i.key(), value);
   }

   settings->endGroup();
   if (delSettings)
      delete settings;
}

bool
BConfig::save()
{
    QSettings settings(_qsetting[0], _qsetting[1]);
    return _save(&settings);
}

QVariant
BConfig::variant(const QObject *w) const
{
    if (const QComboBox *box = qobject_cast<const QComboBox*>(w))
    {
        if (box->itemData(box->currentIndex()).isValid())
            return box->itemData(box->currentIndex());
        return box->currentIndex();
    }
    else if (const QCheckBox *box = qobject_cast<const QCheckBox*>(w))
        return box->isChecked();
    else if (const QGroupBox *box = qobject_cast<const QGroupBox*>(w))
        return box->isCheckable() && box->isChecked();
    else if (const QButtonGroup *group = qobject_cast<const QButtonGroup*>(w))
        return group->checkedId();
    else if (const QAbstractSlider *slider = qobject_cast<const QAbstractSlider*>(w))
        return slider->value();
    else if (const QSpinBox *spin = qobject_cast<const QSpinBox*>(w))
        return spin->value();
    else if (const QLineEdit *lineEdit = qobject_cast<const QLineEdit*>(w))
        return lineEdit->text();
    else if (const QTextEdit *textEdit = qobject_cast<const QTextEdit*>(w))
        return textEdit->toPlainText();

    qWarning("%s is not supported yet, feel free tro ask", w->metaObject()->className());
    return QVariant();
}

bool
BConfig::setVariant(QObject *w, const QVariant &v) const
{
    if (QComboBox *box = qobject_cast<QComboBox*>(w))
    {
        int idx = box->findData(v);
        if (idx == -1)
        {
            idx = v.toInt();
            if (idx >= box->count())
                idx = box->count()-1;
        }
        box->setCurrentIndex(idx);
    }
    else if (QButtonGroup *group = qobject_cast<QButtonGroup*>(w))
    {
            if (QAbstractButton *btn = group->button(v.toInt()))
                btn->setChecked(true);
    }
    else if (QCheckBox *box = qobject_cast<QCheckBox*>(w))
        box->setChecked(v.toBool());
    else if (QGroupBox *box = qobject_cast<QGroupBox*>(w))
        box->setChecked(v.toBool());
    else if (QAbstractSlider *slider = qobject_cast<QAbstractSlider*>(w))
        slider->setValue(v.toInt());
    else if (QSpinBox *spin = qobject_cast<QSpinBox*>(w))
        spin->setValue(v.toInt());
    else if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(w))
        lineEdit->setText(v.toString());
    else if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(w))
        textEdit->setPlainText(v.toString());
    else {
        qWarning("%s is not supported yet, feel free tro ask", w->metaObject()->className());
        return false;
    }
    return true;
}

bool
BConfig::_save(QSettings *settings, bool makeDirty)
{

    bool delSettings = false;
    if (!settings) {
        delSettings = true;
        settings = new QSettings(_qsetting[0], _qsetting[1]);
    }

    if ( !settings->isWritable() )
    {
        QMessageBox::critical( parentWidget(), "Cannot write :-(",
                               QString ( "<qt>Sorry, the file<br><b>%1</b><br>could not be written<hr>"
                               "On unix systems, you can test if you own this file:<br>"
                               "<b>stat %1</b><br>"
                               "In case, you can make it writable<br>"
                               "<b>chmod +w %1</b><hr>"
                               "Or (also on Windows) use a filemanager like Dolphin, Nautilus, "
                               "TotalCommander or Explorer, navigate to the file, rightclick it and "
                               "usually select \"Properties\"<br>"
                               "In the dialog, find the permission section and ensure your avatar "
                               "is allowed to write on it.<hr>"
                               "<b>You do not need to close this configurator meanwhile!</b><br>"
                               "Just retry saving afterwards.</qt>" ).arg( settings->fileName() ) );
        return false;
    }

    settings->beginGroup(_qsetting[2]);

    QMap<QObject*, SettingInfo>::iterator i;
    SettingInfo *info;
    for (i = _settings.begin(); i != _settings.end(); ++i)
    {
        QVariant value = variant(i.key());
        if (value.isValid()) {
            info = &(i.value());
            settings->setValue(info->entry, value);
            if (makeDirty)
            info->savedValue = value;
        }
    }
    settings->endGroup();

    if (delSettings)
        delete settings;

    if (makeDirty)
        emit changed(true);
    return true;
}

bool BConfig::eventFilter ( QObject * o, QEvent * e) {
    if (e->type() == QEvent::Enter)
    {
        if (o == _infoBrowser)
        {
            infoItemHovered = true;
            return false;
        }
        infoItemHovered = false;
        if (QComboBox *box = qobject_cast<QComboBox*>(o))
        {
            QMap<QComboBox*, QStringList>::iterator i;
            for (i = _comboHelps.begin(); i != _comboHelps.end(); ++i)
                if (o == i.key())
                {
                    infoItemHovered = true;
                    if (box->currentIndex() < i.value().count())
                        _infoBrowser->setHtml(i.value().at(box->currentIndex()));
                    else
                        _infoBrowser->setHtml(_defaultContextInfo);
                    infoDirty = true;
                    return false;
                }
        }
        QMap<QObject*, QString>::iterator i;
        for (i = _contextHelps.begin(); i != _contextHelps.end(); ++i)
            if (o == i.key())
            {
                infoItemHovered = true;
                _infoBrowser->setHtml(i.value());
                infoDirty = true;
                return false;
            }
        return false;
    }
    else if (e->type() == QEvent::Leave)
    {
        infoItemHovered = false;
        QTimer::singleShot(300, this, SLOT(resetInfo()));
        return false;
    }
    return false;
}
