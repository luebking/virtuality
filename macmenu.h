/*
 *   Virtuality Style for Qt4 and Qt5
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

#ifndef MAC_MENU_H
#define MAC_MENU_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QPointer>

class QMenuBar;
class QAction;
class QActionEvent;


namespace BE
{

class FullscreenWatcher : public QObject
{
public:
    FullscreenWatcher() : QObject() {};
protected:
    bool eventFilter(QObject *o, QEvent *ev);
};

class MacMenu : public QObject
{
   Q_OBJECT
public:
    static void manage(QMenuBar *menu);
    static bool manages(const QMenuBar *menu);
    static void release(QMenuBar *menu);
    static bool isActive();
    void popup(qlonglong key, int idx, int x, int y);
    void hover(qlonglong key, int idx,  int x, int y);
    void popDown(qlonglong key);
    void raise(qlonglong key);
    typedef QPointer<QMenuBar> QMenuBar_p;
public slots:
    void activate();
    void deactivate();
protected:
    bool eventFilter(QObject *o, QEvent *ev);
protected:
    friend class FullscreenWatcher;
    void deactivate(QWidget *window);
    void activate(QWidget *window);
private:
    Q_DISABLE_COPY(MacMenu)
    MacMenu();
    void activate(QMenuBar *menu);
    void changeAction(QMenuBar *menu, QActionEvent *ev);
    void deactivate(QMenuBar *menu);
    typedef QList<QMenuBar_p> MenuList;
    MenuList items;
    QMenuBar *menuBar(qlonglong key);
    QMap< QMenuBar_p, QList<QAction*> > actions;
    bool usingMacMenu;
    QString service;
    QStringList m_titleSeperators;
private slots:
    void menuClosed();
    void _release(QObject *);
};

} // namespace

#endif //MAC_MENU_H
