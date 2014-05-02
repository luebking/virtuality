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

#ifndef BESPIN_HACKS_H
#define BESPIN_HACKS_H

#include <QObject>

class QWidget;
class QScrollBar;

namespace BE
{

class Hacks : public QObject
{
    Q_OBJECT
public:
    Hacks() {}
    enum HackAppType { Unknown = 0, SMPlayer, Dragon, KDM, Gwenview, VLC, Okular, KDED };
    static bool add(QWidget *w);
    static void releaseApp();
    static void remove(QWidget *w);
    static struct Config
    {
        bool windowMovement, fixGwenview, lockToolBars, lockDocks,
             suspendFullscreenPlayers, titleWidgets, panning, suppressBrightnessOSD;
    } config;
protected:
    bool eventFilter( QObject *, QEvent *);
    void timerEvent(QTimerEvent *te);
private slots:
    void toggleToolBarLock();
    void fixGwenviewPosition();
private:
    Q_DISABLE_COPY(Hacks)
};
} // namespace
#endif // BESPIN_HACKS_H
