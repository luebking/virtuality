/*
 *   Bespin kdeini writer (to write color schemes)
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

#ifndef KDE_INI_H
#define KDE_INI_H

#include <QMap>
#include <QString>
#include <QVariant>

class KdeIni {
public:
   static KdeIni *open(const QString &name);
   bool close();
   QStringList groups() const;
   bool setGroup(const QString &group);
   void setValue(const QString &key, const QVariant &value);
   QColor value(const QString &key, QColor def);
   QString value(const QString &key);
private:
   KdeIni(const QString &name);
   typedef QMap<QString, QString> Entries;
   typedef QMap<QString, Entries> Config;
   Config local, global;
   Config::iterator localGroup;
   Config::const_iterator globalGroup;
   QString localFile;
};

#endif //KDE_INI
