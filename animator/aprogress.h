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

#ifndef PROGRESS_ANIMATOR_H
#define PROGRESS_ANIMATOR_H

#include "basic.h"

namespace Animator {

class Progress : public Basic {
    Q_OBJECT
public:
    static bool manage(QWidget *w);
    static void release(QWidget *w);
    static int step(const QWidget *w);
    static float speed();
protected:
    Progress() : Basic() {};
    int _step(const QWidget *widget, long int index = 0) const;
protected slots:
    void timerEvent(QTimerEvent * event);
private:
    Q_DISABLE_COPY(Progress)
};

}

#endif //PROGRESS_ANIMATOR_H
