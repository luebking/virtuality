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

#include <QColor>
#include <QFile>
#include <QProcess>
#include <QTextStream>

#include "kdeini.h"

static QString confPath[2];

KdeIni*
KdeIni::open(const QString &name)
{
   if (confPath[0].isNull()) { // acquire global and local kde config paths
      QString configFile;
      QProcess kde4_config;
      kde4_config.start("kde4-config --path config");
      if (kde4_config.waitForFinished()) {
         configFile = kde4_config.readAllStandardOutput().trimmed();
         confPath[0] = configFile.section(':', 0, 0); // local
         confPath[1] = configFile.section(':', 1, 1); // global
      }
      if (confPath[0].isNull())
         return 0L; // no paths available, maybe even not KDE4
   }
   return new KdeIni(name);
}

KdeIni::KdeIni(const QString &name)
{
   QString buffer;
   localFile = confPath[0] + name;
   QFile lfile(localFile);
   if (lfile.open(QIODevice::ReadOnly)) {
      localGroup = local.end();
      QTextStream stream(&lfile);
      do {
         buffer = stream.readLine().trimmed();
         if (buffer.startsWith('[')) // group
            localGroup = local.insert(buffer.mid(1,buffer.length()-2), Entries());
         else if (!(buffer.isEmpty() || localGroup == local.end()))
            localGroup.value().insert(buffer.section('=',0,0), buffer.section('=',1));
      } while (!buffer.isNull());
      lfile.close();
   }

   QFile gfile(confPath[1] + name);
   if (gfile.open(QIODevice::ReadOnly)) {
      localGroup = global.end();
      QTextStream stream(&gfile);
      do {
         buffer = stream.readLine().trimmed();
         if (buffer.startsWith('[')) // group
            localGroup = global.insert(buffer.mid(1,buffer.length()-2), Entries());
         else if (!(buffer.isEmpty() || localGroup == global.end()))
            localGroup.value().insert(buffer.section('=',0,0), buffer.section('=',1));
      } while (!buffer.isNull());
      gfile.close();
   }
   localGroup = local.end();
   globalGroup = global.constEnd();
}

QStringList
KdeIni::groups() const
{
   return QStringList();
}

bool
KdeIni::setGroup(const QString &group)
{
   localGroup = local.find(group);
   if (localGroup == local.end())
      localGroup = local.insert(group, Entries());
   globalGroup = global.constFind(group);
   return true; //(localGroup != local.end() && globalGroup = global.constEnd());
}

void
KdeIni::setValue(const QString &key, const QVariant &value)
{
   if (localGroup == local.end()) {
      qWarning("KdeIni::setValue(): You must first set a group!");
      return;
   }
   QString val;
   switch(value.type()) {
   case QVariant::Color: {
      QColor c = value.value<QColor>();
      val = QString::number( c.red() ) + ',' + QString::number( c.green() ) + ',' + QString::number( c.blue() );
      break;
   }
   default:
      val = value.toString();
   }
   (*localGroup)[key] = val;
}

QColor color( const QString &s, QColor c )
{
    QStringList rgb = s.split(',');
    if (rgb.count() > 0)
        c.setRed( rgb.at(0).toUInt() );
    if (rgb.count() > 1)
        c.setGreen( rgb.at(1).toUInt() );
    if (rgb.count() > 2)
        c.setBlue( rgb.at(2).toUInt() );
    if (rgb.count() > 3)
        c.setAlpha( rgb.at(3).toUInt() );
    return c;
}

QColor
KdeIni::value(const QString &key, QColor def)
{
    Entries::const_iterator it = localGroup->constFind(key);
    if (it != localGroup->constEnd())
        return color(*it, def);
    if (globalGroup == global.constEnd())
        return def;
    it = globalGroup->constFind(key);
    if (it != globalGroup->constEnd())
        return color(*it, def);
    return def;
}

QString
KdeIni::value(const QString &key)
{
   Entries::const_iterator it = localGroup->constFind(key);
   if (it != localGroup->constEnd())
      return *it;
   if (globalGroup == global.constEnd())
       return QString();
   it = globalGroup->constFind(key);
   if (it != globalGroup->constEnd())
      return *it;
   return QString();
}

bool
KdeIni::close()
{
   QFile file(localFile);
   if (!file.open(QIODevice::WriteOnly))
      return false;
   Config::const_iterator group = local.constBegin();
   Entries::const_iterator entry;
   QTextStream stream(&file);
   while (group != local.constEnd()) {
      stream << '[' << group.key() << ']' << Qt::endl;
      entry = group.value().constBegin();
      while (entry != group.value().constEnd()) {
         stream << entry.key() << '=' << entry.value() << Qt::endl;
         ++entry;
      }
      stream << Qt::endl;
      ++group;
   }
   stream.flush();
   file.close();
   return true;
}
