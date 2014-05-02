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

#ifndef DIALOG_H
#define DIALOG_H
#include <QDialog>

class Dialog : public QDialog {
   Q_OBJECT
public:
   Dialog() : QDialog(0, Qt::Window){}
public slots:
   void setLayoutDirection(bool rtl) {
      QDialog::setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
   }
};

#endif // DIALOG_H
