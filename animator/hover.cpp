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

#include <QAbstractButton>
#include <QComboBox>
#include <QAbstractScrollArea>
#include <QPushButton>
#include <QScrollBar>
#include <QTimerEvent>
#include <QWidget>

#define ANIMATOR_IMPL 1
#include "hover.h"

using namespace Animator;

INSTANCE(Hover)
SET_FPS(Hover)
RELEASE(Hover)
STEP(Hover)
#undef ANIMATOR_IMPL

static uint _maxSteps = 9;

int Hover::maxSteps() {
    return _maxSteps;
}

void
Hover::setDuration(uint ms)
{
    _maxSteps = ms/_timeStep;
}

Hover::Hover() : Basic()
{
    timeStep = _timeStep;
    setFPS(30);
}

bool
Hover::manage(QWidget *w, bool isScrollArea)
{
    if (!w)
        return false;
    if (!instance)
        instance = new Hover;
    if (isScrollArea)
        return instance->manageScrollArea(w);
    else
        return instance->_manage(w);
}

bool
Hover::managesArea(QWidget *area)
{
    if (!instance)
        return false;
    return instance->_scrollAreas.contains(area);
}

bool
Hover::manageScrollArea(QWidget *area)
{
    if (!area)
        return false;
    area->removeEventFilter(this); // just to be sure...
    area->installEventFilter(this);

    if (qobject_cast<QAbstractScrollArea*>(area))
        return true; // nope, catched by qobject_cast

    if (_scrollAreas.contains(area))
        return false; // we already have this as scrollarea

    _scrollAreas.append(area); // manage by itemlist
    return true;
}

void
Hover::Play(QWidget *widget, bool bwd)
{
    if (instance)
        instance->play(widget, bwd);
}

void
Hover::play(QWidget *widget, bool bwd)
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

// works, cpu load is ok, but REALLY annoying!
#define WOBBLE_HOVER 0

#if WOBBLE_HOVER
#define HOVER_IN_STEP 1
#else
#define HOVER_IN_STEP 2
#endif

void
Hover::_setFPS(uint fps)
{
    _maxSteps = (1000 * _maxSteps) / (timeStep * fps);
    timeStep = 1000/fps;
    if (timer.isActive())
        timer.start(timeStep, this);
}

int
Hover::_step(const QWidget *widget, long int) const
{
    if (!widget || !widget->isEnabled())
        return 0;

    Items::const_iterator it = items.find(const_cast<QWidget*>(widget));
    if (it != items.end())
        return it.value().step() + !it.value().backwards; // (map 1,3,5 -> 2,4,6)
    if (widget->testAttribute(Qt::WA_UnderMouse))
        return _maxSteps;
    return 0;
}

void
Hover::timerEvent(QTimerEvent * event)
{
    if (event->timerId() != timer.timerId() || noAnimations())
        return;

    Items::iterator it = items.begin();
    int *step = 0;
    QWidget *widget = 0;
    while (it != items.end())
    {
        widget = it.key().data();
        if (!widget)
        {
            it = items.erase(it);
            continue;
        }
        step = &it.value()._step;
        if (it.value().backwards)
        {   // fade OUT
            --(*step);
            widget->update();
            if (*step < 1)
            {
#if WOBBLE_HOVER
                if (widget->testAttribute(Qt::WA_UnderMouse))
                    it.value().backwards = false;
                else
#endif
                    it = items.erase(it);
            }
            else
                ++it;
        }
        else
        {   // fade IN
            *step += HOVER_IN_STEP;
            widget->update();
            if ((uint)(*step) > _maxSteps-2)
            {
#if WOBBLE_HOVER
                if (widget->testAttribute(Qt::WA_UnderMouse))
                    it.value().backwards = true;
                else
#endif
                    it = items.erase(it);
            }
            else
                ++it;
        }
    }
    if (noAnimations())
        timer.stop();
}

#define HANDLE_SCROLL_AREA_EVENT(_DIR_) \
if (area->horizontalScrollBar()->isVisible())\
    play(area->horizontalScrollBar(), _DIR_);\
if (area->verticalScrollBar()->isVisible())\
    play(area->verticalScrollBar(), _DIR_)

#define isAttachedScrollbar\
/*if*/ (kid && kid->parent() == object)\
if ((sb = qobject_cast<QScrollBar*>(kid)))


bool
Hover::eventFilter( QObject* object, QEvent *e )
{
    QWidget* widget = qobject_cast<QWidget*>(object);
    if (!(widget && widget->isVisible() && widget->isEnabled()))
        return false;

    switch (e->type())
    {
    case QEvent::Timer:
    case QEvent::Move:
    case QEvent::Paint:
    case QEvent::MouseMove:
    case QEvent::UpdateRequest:
    case QEvent::MouseButtonPress:
    case QEvent::Wheel:
        return false; // just for performance - they can occur really often
    case QEvent::WindowActivate:
    case QEvent::Enter:
    {
        if (QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(object))
        {
            if (!area->isEnabled())
                return false;
            HANDLE_SCROLL_AREA_EVENT(false);
            return false;
        }
        else if (_scrollAreas.contains(object))
        {
            QObjectList kids = object->children();
            QWidget *sb;
            foreach (QObject *kid, kids)
            {
                if isAttachedScrollbar
                    play(sb);
            }
            return false;
        }
        if (e->type() == QEvent::WindowActivate)
            return false;
        play(widget);
        return false;
    }
    case QEvent::WindowDeactivate:
    case QEvent::Leave:
    {
        if (QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(object))
        {
            if (!area->isEnabled())
                return false;
            HANDLE_SCROLL_AREA_EVENT(true);
            return false;
        }
        else if (_scrollAreas.contains(object))
        {
            QObjectList kids = object->children();
            QWidget *sb;
            foreach (QObject *kid, kids)
            {
                if isAttachedScrollbar
                    play(sb, true);
            }
            return false;
        }
        if (e->type() == QEvent::WindowDeactivate)
            return false;
        play(widget, true);
        return false;
    }
#undef HANDLE_SCROLL_AREA_EVENT

#if 0
    case QEvent::FocusIn:
        if (qobject_cast<QPushButton*>(object) || qobject_cast<QComboBox*>(object)) {
            QWidget *widget = (QWidget*)object;
            if (!widget->isEnabled())
                return false;
            if (widget->testAttribute(Qt::WA_UnderMouse))
                widget->update();
            else
                play(widget);
            return false;
        }
        return false;
    case QEvent::FocusOut:
        if (qobject_cast<QPushButton*>(object) || qobject_cast<QComboBox*>(object)) {
            QWidget *widget = (QWidget*)object;
            if (!widget->isEnabled())
                return false;
            if (widget->testAttribute(Qt::WA_UnderMouse))
                widget->update();
            else
                play(widget, true);
            return false;
        }
        return false;
#endif
    default:
        return false;
    }
}

