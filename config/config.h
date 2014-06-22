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

#ifndef CONFIG_H
#define CONFIG_H

class QDialog;

#include "bconfig.h"
#include "ui_config.h"

class Config : public BConfig /** <-- inherit BConfig */
{
    Q_OBJECT
public:
    /** The constructor... */
    Config(QWidget *parent = 0L);
    static QStringList colors(const QPalette &pal, QPalette::ColorGroup group);
    static void updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list);
    static void savePalette(const QPalette &pal);
    void setLogo(const QPixmap &pix);
protected:
    void changeEvent(QEvent *event);
    bool eventFilter( QObject *o, QEvent *e );
    QVariant variant(const QObject *w) const;
    bool setVariant(QObject *w, const QVariant &v) const;
public slots:
    /** We'll reimplement the im/export functions to handle color settings as well*/
    bool save(); // to store colors to qt configuration - in case
private:
    Q_DISABLE_COPY(Config)
    /** This is the UI created with Qt Designer and included by ui_config.h */
    Ui::Config ui;
    bool myColorsHaveChanged;

    QPalette *loadedPal;
    bool infoIsManage;
    void applyPalette( QPalette *pal );
    void setColorsFromPalette( const QPalette &pal );
private slots:
    void initColors();
    void learnPwChar();
    void alignLabel();
};

#endif
