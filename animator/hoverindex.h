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

#ifndef HOVER_INDEX_ANIMATOR_H
#define HOVER_INDEX_ANIMATOR_H

#include <QBasicTimer>
#include <QMap>
#include <QPointer>
#include <QWidget>


namespace Animator {

   enum Dir { In = 0, Out };

class IndexInfo {
public:
   IndexInfo(long int idx) {index = idx;}
   virtual ~IndexInfo() {}
   virtual int step(long int idx = 0) const;
protected:
   friend class HoverIndex;
   typedef QMap<long int, int> Fades;
   Fades fades[2];
   long int index;
};

typedef QPointer<QWidget> WidgetPtr;

class HoverIndex : public QObject {
   Q_OBJECT
public:
   static const IndexInfo *info(const QWidget *widget, long int index);
   static int maxSteps();
   static void setDuration(uint ms);
   static void setFPS(uint fps);
protected:
   HoverIndex();
   virtual ~HoverIndex(){}
   virtual const IndexInfo *_info(const QWidget *widget, long int index) const;
   virtual void _setFPS(uint fps);
   virtual void timerEvent(QTimerEvent * event);
   QBasicTimer timer;
   uint timeStep, count, m_maxSteps;
   typedef QMap<WidgetPtr, IndexInfo> Items;
   Items items;
protected slots:
   void release(QObject *o);
private:
    Q_DISABLE_COPY(HoverIndex)
};

}

#ifndef WIDGET_PTR_LESSER
#define WIDGET_PTR_LESSER
inline bool operator< (const Animator::WidgetPtr &ptr1, const Animator::WidgetPtr &ptr2) {
    return ptr1.data() < ptr2.data();
}
#endif

#ifndef ANIMATOR_IMPL
#define ANIMATOR_IMPL 0
#endif

#if ANIMATOR_IMPL

#define INSTANCE(_CLASS_) static _CLASS_ *instance = 0;

#define SET_FPS(_CLASS_)\
static uint _timeStep = 50;\
void _CLASS_::setFPS(uint fps)\
{\
_timeStep = fps/1000;\
if (instance) instance->_setFPS(fps);\
   }
   #define SET_DURATION(_CLASS_)\
   static uint _duration = 300;\
   void _CLASS_::setDuration(uint ms)\
   {\
   _duration = ms;\
   if (instance) instance->m_maxSteps = ms/_timeStep;\
   }

#undef ANIMATOR_IMPL

#endif //ANIMATOR_IMPL

#endif //HOVER_INDEX_ANIMATOR_H
