/*
 *   Virtuality Style for Qt4 and Qt5
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

#ifndef HOVER_ANIMATOR_H
#define HOVER_ANIMATOR_H

#include "basic.h"

namespace Animator {

class Hover : public Basic {
Q_OBJECT
    public:
    static bool manage(QWidget *w, bool isScrollArea = false);
    static bool managesArea(QWidget *area);
    static void release(QWidget *w);
    static void setDuration(uint ms);
    static void setFPS(uint fps);
    static int step(const QWidget *widget);
    static int maxSteps();
    static void Play(QWidget *widget, bool bwd = false);
protected:
    Hover();
    virtual bool manageScrollArea(QWidget *w);
    void _setFPS(uint fps);
    virtual int _step(const QWidget *widget, long int index = 0) const;
    virtual void play(QWidget *widget, bool bwd = false);
protected slots:
    virtual bool eventFilter( QObject *object, QEvent *event );
    virtual void timerEvent(QTimerEvent * event);
private:
    Q_DISABLE_COPY(Hover)
    QObjectList _scrollAreas;
};

}

#endif //HOVER_ANIMATOR_H





