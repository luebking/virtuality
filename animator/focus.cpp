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

#include <QTimerEvent>
#include <QWidget>

#define ANIMATOR_IMPL 1
#include "focus.h"

using namespace Animator;

INSTANCE(Focus)
SET_FPS(Focus)
RELEASE(Focus)
STEP(Focus)
#undef ANIMATOR_IMPL

static uint _maxSteps = 9;

int Focus::maxSteps() {
    return _maxSteps;
}

void
Focus::setDuration(uint ms)
{
    _maxSteps = ms/_timeStep;
}

Focus::Focus() : Basic()
{
    timeStep = _timeStep;
    setFPS(30);
}

bool
Focus::manage(QWidget *w)
{
    if (!w)
        return false;
    if (!instance)
        instance = new Focus;
    return instance->_manage(w);
}

void
Focus::Play(QWidget *widget, bool bwd)
{
    if (instance)
        instance->play(widget, bwd);
}

void
Focus::play(QWidget *widget, bool bwd)
{
    if (!widget)
        return;

    const bool needTimer = noAnimations(); // true by next lines
    Items::iterator it = items.find(widget);
    if (it == items.end())
        it = items.insert(widget, Info(bwd ? _maxSteps : 1, bwd));
    else
        it.value().backwards = bwd;
    if (needTimer)
        timer.start(timeStep, this);
}

#define FOCUS_IN_STEP 2
#define FOCUS_OUT_STEP 2

void
Focus::_setFPS(uint fps)
{
    _maxSteps = (1000 * _maxSteps) / (timeStep * fps);
    timeStep = 1000/fps;
    if (timer.isActive())
        timer.start(timeStep, this);
}

int
Focus::_step(const QWidget *widget, long int) const
{
    if (!widget || !widget->isEnabled())
        return 0;

    Items::const_iterator it = items.find(const_cast<QWidget*>(widget));
    if (it != items.end())
        return it.value().step() + !it.value().backwards; // (map 1,3,5 -> 2,4,6)
    if (widget->hasFocus())
        return _maxSteps;
    return 0;
}

void
Focus::timerEvent(QTimerEvent * event)
{
    if (event->timerId() != timer.timerId() || noAnimations())
        return;

    Items::iterator it = items.begin();
    int *step = 0;
    QWidget *widget = 0;
    while (it != items.end()) {
        widget = it.key().data();
        if (!widget) {
            it = items.erase(it);
            continue;
        }
        step = &it.value()._step;
        if (it.value().backwards) {   // fade OUT
            *step -= FOCUS_OUT_STEP;
            widget->update();
            if (*step < 1)
                it = items.erase(it);
            else
                ++it;
        } else {   // fade IN
            *step += FOCUS_IN_STEP;
            widget->update();
            if ((uint)(*step) > _maxSteps-2)
                it = items.erase(it);
            else
                ++it;
        }
    }
    if (noAnimations())
        timer.stop();
}


#define IF_WIDGET QWidget* widget = qobject_cast<QWidget*>(object); \
                  if (widget && widget->isVisible() && widget->isEnabled())

bool
Focus::eventFilter(QObject* object, QEvent *e)
{
    switch (e->type()) {
    case QEvent::FocusIn: {
        IF_WIDGET
            play(widget);
        return false;
    }
    case QEvent::FocusOut: {
        IF_WIDGET
            play(widget, true);
        return false;
    }
    default:
        return false;
    }
}

