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

#include <QHash>
#include <QPixmap>
#include <QStackedWidget>
#include <QElapsedTimer>
#include "basic.h"

namespace Animator {

enum Transition {
    Jump = 0, ScanlineBlend, SlideIn, SlideOut,
    RollIn, RollOut, OpenVertically, CloseVertically,
    OpenHorizontally, CloseHorizontally, CrossFade, Slide, Roll, Door
};

class Curtain;
class Tab;
typedef QPointer<QStackedWidget> StackWidgetPtr;

class TabInfo : public QObject
{
public:
    TabInfo(QObject* parent, QWidget *currentWidget = 0, int index = -1);
    bool proceed();
    void switchTab(QStackedWidget *sw, int index);
protected:
    QPointer<Curtain> curtain;
    float progress;
    QPointer<QWidget> currentWidget;
    friend class Tab;
    int index;
    uint duration;
    QElapsedTimer clock;
    char tabPosition;
    bool isBackSwitch;
protected:
    friend class Curtain;
    QPixmap tabPix[3];
private:
    void rewind();
    void updatePixmaps(Transition transition, uint ms = 0);
};

class Tab : public Basic {
    Q_OBJECT
public:
    static bool manage(QWidget *w);
    static void release(QWidget *w);
    static void setDuration(uint ms);
    static void setFPS(uint fps);
    static void setTransition(Transition t);
protected:
    Tab();
    virtual bool _manage(QWidget *w);
    virtual void _release(QWidget *w);
    virtual void timerEvent(QTimerEvent * event);
    typedef QMap<StackWidgetPtr, TabInfo*> Items;
    Items items;
    int _activeTabs;
protected slots:
    void changed(int);
    void widgetRemoved(int);
private:
    Q_DISABLE_COPY(Tab)
};

} //namespace

#ifndef STACK_WIDGET_PTR_LESSER
#define STACK_WIDGET_PTR_LESSER
inline bool operator< (const Animator::StackWidgetPtr &ptr1, const Animator::StackWidgetPtr &ptr2) {
    return ptr1.data() < ptr2.data();
}
#endif
