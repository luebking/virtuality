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
#include "hoverindex.h"

using namespace Animator;

int
IndexInfo::step(long int index) const
{
    Fades::const_iterator i;
    for (int dir = 0; dir < 2; ++dir)
    {
        for (i = fades[dir].begin(); i != fades[dir].end(); ++i)
            if (i.key() == index)
                return i.value();
    }
    return 0;
}

INSTANCE(HoverIndex)
SET_FPS(HoverIndex)
SET_DURATION(HoverIndex)
#undef ANIMATOR_IMPL

HoverIndex::HoverIndex() : QObject(),
timeStep(_timeStep), count(0), m_maxSteps(_duration/_timeStep) {}

void
HoverIndex::_setFPS(uint fps)
{
   m_maxSteps = (1000 * m_maxSteps) / (timeStep * fps);
   timeStep = 1000/fps;
   if (timer.isActive())
      timer.start(timeStep, this);
}

const IndexInfo *
HoverIndex::info(const QWidget *widget, long int idx)
{
   if (!widget) return 0;
   if (!instance)
       instance = new HoverIndex;
   return instance->_info(widget, idx);
}

const IndexInfo *
HoverIndex::_info(const QWidget *widget, long int idx) const
{
    HoverIndex *that = const_cast<HoverIndex*>(this);
    QWidget *w = const_cast<QWidget*>(widget);
    Items::iterator it = that->items.find(w);
    if (it == items.end())
    {   // we have no entry yet
        if (idx == 0L)
            return 0L;
        // ... but we'll need one
        it = that->items.insert(w, IndexInfo(0L));
        connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(release(QObject*)));
//         if (!timer.isActive())
            that->timer.start(timeStep, that);
    }
    // we now have an entry - check for validity and update in case
    IndexInfo &info = it.value();
    if (info.index != idx)
    {   // sth. changed
        info.fades[In][idx] = 1;
        if (info.index)
        {
            int v = m_maxSteps;
            IndexInfo::Fades::iterator old = info.fades[In].find(info.index);
            if (old != info.fades[In].end())
            {
                v = old.value();
                info.fades[In].erase(old);
            }
            info.fades[Out][info.index] = v;
        }
        info.index = idx;
    }
    return &info;
}

void
HoverIndex::release(QObject *o)
{
    QWidget *w = qobject_cast<QWidget*>(o);
    if (!w)
        return;
    Items::iterator it = items.begin(), end = items.end();
    while (it != end) {
        if (it.key().isNull()) {
            it = items.erase(it);
            continue;
        }
        if (it.key().data() == w) {
            items.erase(it);
            break;
        }
        ++it;
    }
    if (items.isEmpty())
        timer.stop();
}


void
HoverIndex::timerEvent(QTimerEvent * event)
{
    if (event->timerId() != timer.timerId() || items.isEmpty())
        return;

    Items::iterator it;
    IndexInfo::Fades::iterator step;
    it = items.begin();
    QWidget *w;
    while (it != items.end())
    {
        if (it.key().isNull())
        {
            it = items.erase(it);
            continue;
        }
        w = const_cast<QWidget*>(it.key().data());
        IndexInfo &info = it.value();
        if (info.fades[In].isEmpty() && info.fades[Out].isEmpty())
        {
            ++it; continue;
        }

        step = info.fades[In].begin();
        while (step != info.fades[In].end())
        {
            step.value() += 2;
            if ((uint)step.value() > (m_maxSteps-2))
                step = info.fades[In].erase(step);
            else
                ++step;
        }

        step = info.fades[Out].begin();
        while (step != info.fades[Out].end())
        {
            step.value() -= 2;
            if (step.value() < 1)
                step = info.fades[Out].erase(step);
            else
                ++step;
        }

        w->update();

        if (info.index == 0L && // nothing actually hovered
            info.fades[In].isEmpty() && // no fade ins
            info.fades[Out].isEmpty()) // no fade outs
            it = items.erase(it); // so remove this item
        else
            ++it;
    }

    if (items.isEmpty())
        timer.stop();
}
