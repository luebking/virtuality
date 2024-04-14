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

#include <QAbstractScrollArea>
#include <QApplication>
#include <QDropEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QStyleOption>
#include <QStackedWidget>

#include <cmath>

#include "../FX.h"

#define ANIMATOR_IMPL 1
#include "tab.h"

#include <QtDebug>

using namespace BE;
using namespace Animator;

INSTANCE(Tab)
MANAGE(Tab)
RELEASE(Tab)
SET_FPS(Tab)

#undef ANIMATOR_IMPL

static inline QAbstractScrollArea*
scrollAncestor(QWidget *w, QWidget *root)
{
    QWidget *parent = w;
    while (parent != root && (parent = parent->parentWidget()))
    {
        if (qobject_cast<QAbstractScrollArea*>(parent))
            break;
    }
    if (parent != root)
        return static_cast<QAbstractScrollArea*>(parent);
    return 0L;
}

// to get an idea about what the bg of out tabs looks like - seems as if we
// need to paint it
static QPixmap
dumpBackground(QWidget *target, const QRect &r, const QStyle *style, bool _32bit = false )
{
    if (!target)
        return QPixmap();

    QPixmap pix(r.size());
    if (_32bit)
        pix.fill(Qt::transparent);

    QWidgetList widgets; widgets << target;
    QWidget *w = target->parentWidget();
    while (w)
    {
        if (!w->isVisible())
        {
            w = w->parentWidget();
            continue;
        }
        widgets << w;
        if (w->isTopLevel() || w->autoFillBackground())
            break;
        w = w->parentWidget();
    }
    if (!w)
        w = target;

    QPainter p(&pix);
    const QBrush bg = w->palette().brush(w->backgroundRole());
    if (bg.style() == Qt::TexturePattern)
        p.drawTiledPixmap(pix.rect(), bg.texture(), target->mapTo(w, r.topLeft()));
    else
        p.fillRect(pix.rect(), bg);

    if (w->isTopLevel() && w->testAttribute(Qt::WA_StyledBackground))
    {
        QStyleOption opt; opt.initFrom(w); opt.rect = r;
        opt.rect.translate(target->mapTo(w, r.topLeft()));
        p.translate(-opt.rect.topLeft());
        style->drawPrimitive ( QStyle::PE_Widget, &opt, &p, w);
    }
    p.end();

    QPoint zero(0,0);
    QPaintEvent e(r); int i = widgets.size();
    while (i)
    {
        w = widgets.at(--i);
#if true
        // QWidget::render does not include eventfilters, but QPainter::setRedirected is deprecated
        w->render(&pix, -target->mapTo(w, zero), w->rect(), QWidget::RenderFlags());
#else
        QPainter::setRedirected( w, &pix, target->mapTo(w, r.topLeft()) );
        e = QPaintEvent(QRect(zero, r.size()));
        QCoreApplication::sendEvent(w, &e);
        QPainter::restoreRedirected(w);
#endif
    }
    return pix;
}


#define WORKAROUND_SCROLLAREAS 0

// QPixmap::grabWidget(.) currently fails on the background offset,
// so we use our own implementation
static void
grabWidget(QWidget * root, QPixmap &pix)
{
    if (!root)
        return;

    QPoint zero(0,0);
//     QSize sz = root->window()->size();

   // resizing (in case) -- NOTICE may be dropped for performance...?!
//    if (root->testAttribute(Qt::WA_PendingResizeEvent) ||
//        !root->testAttribute(Qt::WA_WState_Created)) {
//       QResizeEvent e(root->size(), QSize());
//       QApplication::sendEvent(root, &e);
//    }
//    foreach (QWidget *w, widgets) {
//       if (root->testAttribute(Qt::WA_PendingResizeEvent) ||
//          !root->testAttribute(Qt::WA_WState_Created)) {
//          QResizeEvent e(w->size(), QSize());
//          QApplication::sendEvent(w, &e);
//       }
//    }

    // painting ------------
#if true
    // QWidget::render does not include eventfilters, but QPainter::setRedirected is deprecated
    root->render(&pix, zero, root->rect(), QWidget::RenderFlags());
#else
    QPainter::setRedirected( root, &pix );
    QPaintEvent e(QRect(zero, root->size()));
    QCoreApplication::sendEvent(root, &e);
    QPainter::restoreRedirected(root);
#endif

#if WORKAROUND_SCROLLAREAS
    bool hasScrollAreas = false;
    QAbstractScrollArea *scrollarea = 0;
#endif
    QPainter p; QRegion rgn;
    QPixmap *saPix = 0L;

    QWidgetList widgets = root->findChildren<QWidget*>();
    QList<WidgetPtr> widgets2;
    foreach (QWidget *w, widgets)
        widgets2.append(WidgetPtr(w));

    for(QList<WidgetPtr>::const_iterator it = widgets2.constBegin(), end = widgets2.constEnd(); it != end; ++it)
    {
        QWidget *w = it->data();
        if (w && w->isVisibleTo(root))
        {
            // solids
            if (w->autoFillBackground())
            {
                const QBrush bg = w->palette().brush(w->backgroundRole());
                p.begin(&pix);
                QRect wrect = QRect(zero, w->size()).translated(w->mapTo(root, zero));
                if (bg.style() == Qt::TexturePattern)
                    p.drawTiledPixmap(wrect, bg.texture(), w->mapTo(root->window(), zero));
                else
                    p.fillRect(wrect, bg);
                p.end();
            }

            // scrollarea workaround
#if WORKAROUND_SCROLLAREAS
            if ((scrollarea = qobject_cast<QAbstractScrollArea*>(w)))
                hasScrollAreas = true;

            if ( hasScrollAreas && w != scrollarea && !qobject_cast<QScrollBar*>(w) && ( scrollarea = scrollAncestor(w, root) ) )
            {
                // lately causes segfaults on QWidget::render() if painted through eventfilter
                // and otherwise the redirected painting doesn't look different...
                if ( w->objectName() == "qt_scrollarea_viewport" && w->parentWidget() && w->parentWidget()->inherits( "KHTMLView" ) )
                    continue;
                // repaints recursive...
                if ( w->objectName() == "RenderFormElementWidget" )
                    continue;
//                if ( w->metaObject()->className() == "KOrg::MonthGraphicsView" )
//                    continue;
                QRect rect = scrollarea->frameRect();
                if (rect.isValid())
                {
                    rect.translate(scrollarea->mapTo(root, zero));
                    if (!saPix || saPix->size() != rect.size())
                    {
                        delete saPix;
                        saPix = new QPixmap(rect.size());
                    }
                    p.begin(saPix);
                    p.drawPixmap(zero, pix, rect);
                    p.end();
                    const QPoint &pt = scrollarea->frameRect().topLeft();
#if 0
                    if ( false )
                    {
                        QPainter::setRedirected( w, saPix, w->mapFrom(scrollarea, pt) );
                        w->repaint();
                        QPainter::restoreRedirected( w );
                    }
                    else
#endif
                        w->render(saPix, w->mapTo(scrollarea, pt), w->rect(), QWidget::RenderFlags());
                    p.begin(&pix);
                    p.drawPixmap(rect.topLeft(), *saPix);
                    p.end();
                }
            }
            else
#endif //WORKAROUND_SCROLLAREAS
            {   // default painting redirection
                w->render(&pix, w->mapTo(root, zero), w->rect(), QWidget::RenderFlags());
            }
        }
    }
    delete saPix;
}

static uint _duration = 350;
static Transition _transition = SlideIn;

class Animator::Curtain : public QWidget
{
public:
    Curtain(TabInfo *info, QWidget *parent) : QWidget(parent), _info(info)
    {
        setAutoFillBackground( false );
        setAcceptDrops( true );
//         setAttribute(Qt::WA_NoSystemBackground);
        setAttribute( Qt::WA_OpaquePaintEvent );
        raise();
    }
protected:
    void dragEnterEvent( QDragEnterEvent *dee ) { propagate( (QDropEvent*)dee ); }
    void dragLeaveEvent ( QDragLeaveEvent *dle ) { if ( isVisible() ) propagate( (QDropEvent*)dle ); }
    void dragMoveEvent ( QDragMoveEvent *dme ) { propagate( (QDropEvent*)dme ); }
    void dropEvent ( QDropEvent *de )  { propagate( de ); }

    void paintEvent( QPaintEvent *  )
    {
        if ( !_info->clock.isValid() )
            return; // should not happen
        QPainter p( this );
        p.drawPixmap( 0, 0, _info->tabPix[2] );
        p.end();
    }
private:
    void propagate( QDropEvent *de )
    {
        QWidget *receiver = 0;
        QWidget *container = parentWidget();
        const QPoint pos = mapToParent( de->pos() );
        while ( container )
        {
            const QObjectList &kids = container->children();
            container = 0;
            for ( int i = kids.count()-1; i>-1; --i )
            {
                if ( kids.at(i) != this && kids.at(i)->isWidgetType() )
                {
                    QWidget *w = static_cast<QWidget*>(kids.at(i));
                    if ( w->isVisibleTo( parentWidget() ) && w->rect().contains( w->mapFromParent( pos ) ) )
                    {
                        receiver = container = w;
                        break;
                    }
                }
            }
        }
        if ( receiver )
            QCoreApplication::sendEvent( receiver, de );
    }
    TabInfo *_info;
};

class StdChildAdd : public QObject
{
   public:
      bool eventFilter( QObject *, QEvent *ev) {
         return (ev->type() == QEvent::ChildAdded);
      }
};


TabInfo::TabInfo(QObject* parent, QWidget *current, int idx) :
QObject(parent), progress(0.0), currentWidget(current), index(idx),
tabPosition(QTabWidget::North), isBackSwitch(false) {}

bool
TabInfo::proceed()
{
   if (!clock.isValid()) // this tab is currently not animated
      return false;

   // check if our desired duration has exceeded and stop this in case
   uint ms = clock.elapsed();
   if (ms > duration - _timeStep) {
      rewind();
      return false;
   }

   // normal action
   updatePixmaps(_transition, ms);
   if (Curtain *c = curtain.data())
       c->repaint();
   return true;  // for counter
}

void
TabInfo::rewind()
{
    clock.invalidate(); // reset clock, this is IMPORTANT!
    QWidget *cWidget = currentWidget.data();
    if (cWidget)
        cWidget->setUpdatesEnabled(false);
    delete curtain.data(); curtain = NULL; // get rid of curtain, and RESHOW CONTENT!
    if (cWidget)
    {
        cWidget->setUpdatesEnabled(true);
        cWidget->repaint();
    }
    tabPix[0] = tabPix[1] = tabPix[2] = QPixmap(); // reset pixmaps, saves space
}

#define TOO_SLOW clock.elapsed() > maxRenderTime

void
TabInfo::switchTab(QStackedWidget *sw, int newIdx)
{
    progress = 0.0;
    // update from/to indices
    //    const int oldIdx = tai->index; // just for debug out later on
    QWidget *ow = sw->widget(index);
    QWidget *cw = sw->widget(newIdx);
    currentWidget = cw;
    isBackSwitch = newIdx < index;
    index = newIdx;

    if (!(sw->isVisible() && ow && cw))
        return;

    int maxRenderTime = qMin(200, (int)(_duration - _timeStep));

#define AVOID(_COND_) if (_COND_) { rewind(); return; } //

    AVOID(!ow); // this is the first time the tab changes, nothing to blend
    AVOID(ow == cw); // this can happen on destruction etc... and thus lead to segfaults...

    // prepare the pixmaps we use to pretend the animation
    QRect contentsRect(ow->mapTo(sw, QPoint(0,0)), ow->size());
    tabPix[1] = dumpBackground(sw, contentsRect, qApp->style(), _transition == CrossFade );

    if (!clock.isValid())
    {
        clock.start();
        tabPix[0] = tabPix[1];
        grabWidget(ow, tabPix[0]);
//         tabPix[0] = QPixmap::grabWidget(ow);
        tabPix[2] = tabPix[0];
        AVOID(TOO_SLOW);
    }
    else
    {   // humm?? very fast tab change... maybe the user changed his mind...
        clock.restart();
        tabPix[0] = tabPix[2];
    }

    grabWidget(cw, tabPix[1]);
//     tabPix[1] = QPixmap::grabWidget(cw);
    AVOID(TOO_SLOW);

    duration = _duration - clock.elapsed() + _timeStep;
    clock.restart(); // clock.addMSecs(_timeStep);
    updatePixmaps(_transition, _timeStep);

    // make curtain and first update ----------------
    if (curtain.isNull())
    {   // prevent w from doing freaky things with the curtain
        // (e.g. QSplitter would add a new section...)
        StdChildAdd *stdChildAdd = new StdChildAdd;
        sw->installEventFilter(stdChildAdd);

        Curtain *c;
        curtain = c = new Curtain(this, sw);
        c->move(contentsRect.topLeft());
        c->resize(contentsRect.size());
        c->show();

        sw->removeEventFilter(stdChildAdd);
        delete stdChildAdd;
    }
    else
        curtain.data()->raise();
}

inline static int length(const QPixmap &p, char t) {
    if (t == QTabWidget::North || t == QTabWidget::South)
        return p.height();
    return p.width();
}

void
TabInfo::updatePixmaps(Transition transition, uint ms)
{
    switch (transition)
    {
        default:
        case CrossFade:
        {
            // belive it or not: linear and will end up at a fully blended pixmap, as
            // progress = (1-quote)*progress + quote; // !
            float quote = (float)_timeStep / (duration-ms);
            FX::blend(tabPix[1], tabPix[2], quote);
            break;
        }
        case ScanlineBlend:
        {
            QPainter p(&tabPix[2]);
            const int numStep = duration/_timeStep;
            const int h = qRound(_timeStep * (numStep-progress) / (duration-ms));
            for (int i = (int)progress; i < tabPix[2].height(); i+=numStep)
                p.drawPixmap(0, i, tabPix[1], 0, i, tabPix[1].width(), h);
            progress += h;
            break;
        }
        case Slide:
            updatePixmaps(isBackSwitch ? SlideOut : SlideIn, ms);
            break;
        case SlideIn:
        case SlideOut:
        {
            const QPixmap &srcPix = transition == SlideOut ? tabPix[0] : tabPix[1];
            if (transition == SlideOut)
                tabPix[2] = tabPix[1];
            QPainter p(&tabPix[2]);
            const int l = (transition == SlideOut ? (duration - ms) : ms)*length(srcPix, tabPosition)/duration;
            switch(tabPosition) {
                case QTabWidget::North:
                default:
                    p.drawPixmap(0, 0, srcPix, 0, srcPix.height() - l, srcPix.width(), l);
                    break;
                case QTabWidget::South:
                    p.drawPixmap(0, srcPix.height() - l, srcPix, 0, 0, srcPix.width(), l);
                    break;
                case QTabWidget::West:
                    p.drawPixmap(0, 0, srcPix, srcPix.width() - l, 0, l, srcPix.height());
                    break;
                case QTabWidget::East:
                    p.drawPixmap(srcPix.width() - l, 0, srcPix, 0, 0, l, srcPix.height());
                    break;
            }
            break;
        }
        case Roll:
            updatePixmaps(isBackSwitch ? RollOut : RollIn, ms);
            break;
        case RollIn:
        {
            QPainter p(&tabPix[2]);
            int h = ms*tabPix[1].height()/(2*duration);
            p.drawPixmap(0, 0, tabPix[1], 0, 0, tabPix[1].width(), h);
            p.drawPixmap(0, tabPix[1].height()-h, tabPix[1], 0, tabPix[1].height()-h, tabPix[1].width(), h);
            break;
        }
        case RollOut:
        {
            QPainter p(&tabPix[2]);
            int h = ms*tabPix[1].height()/duration;
            int y = (tabPix[1].height()-h)/2;
            p.drawPixmap(0, y, tabPix[1], 0, y, tabPix[1].width(), h);
            break;
        }
        case Door:
            updatePixmaps(isBackSwitch ? CloseHorizontally : OpenHorizontally, ms);
            break;
        case OpenVertically:
        {
            tabPix[2] = tabPix[1];
            QPainter p(&tabPix[2]);
            const int off = ms*tabPix[0].height()/(2*duration);
            const int h2 = tabPix[0].height()/2;
            p.drawPixmap(0,0, tabPix[0], 0,off, tabPix[0].width(),h2 - off);
            p.drawPixmap(0,h2+off, tabPix[0], 0,h2, tabPix[0].width(),tabPix[0].height()-off);
            break;
        }
        case CloseVertically:
        {
            QPainter p(&tabPix[2]);
            int h = ms*tabPix[1].height()/(2*duration);
            p.drawPixmap(0, 0, tabPix[1], 0, tabPix[1].height()/2-h, tabPix[1].width(), h);
            p.drawPixmap(0, tabPix[1].height()-h, tabPix[1], 0, tabPix[1].height()/2, tabPix[1].width(), h);
            break;
        }
        case OpenHorizontally:
        {
            tabPix[2] = tabPix[1];
            QPainter p(&tabPix[2]);
            const int off = ms*tabPix[0].width()/(2*duration);
            const int w2 = tabPix[0].width()/2;
            p.drawPixmap(0,0,tabPix[0],off,0, w2-off,tabPix[0].height());
            p.drawPixmap(w2+off,0,tabPix[0], w2,0,tabPix[0].width()-off,tabPix[0].height());
            break;
        }
        case CloseHorizontally:
        {
            QPainter p(&tabPix[2]);
            int w = ms*tabPix[1].width()/(2*duration);
            p.drawPixmap(0, 0, tabPix[1], tabPix[1].width()/2-w, 0, w, tabPix[1].height());
            p.drawPixmap(tabPix[1].width()-w, 0, tabPix[1], tabPix[1].width()/2, 0, w, tabPix[1].height());
            break;
        }
   }
}

void
Tab::setDuration(uint ms)
{
   _duration = ms;
}

void
Tab::setTransition(Transition t)
{
   _transition = t;
}

Tab::Tab() : Basic(), _activeTabs(0)
{
   timeStep = _timeStep; // yes! otherwise we'd inherit general timestep
}

bool
Tab::_manage (QWidget* w)
{
   // the tabs need to be kept in a list, as currentChanged() does not allow us
   // to identify the former tab... unfortunately.
   QStackedWidget *sw = qobject_cast<QStackedWidget*>(w);
   if (!sw)
       return false;

   connect(sw, SIGNAL(destroyed(QObject*)), SLOT(release_s(QObject*)));
   connect(sw, SIGNAL(widgetRemoved(int)), SLOT(widgetRemoved(int)));
   connect(sw, SIGNAL(currentChanged(int)), SLOT(changed(int)));
   items.insert(sw, new TabInfo(this, sw->currentWidget(), sw->currentIndex()));
   return true;
}

void
Tab::_release(QWidget *w)
{
   QStackedWidget *sw = qobject_cast<QStackedWidget*>(w);
   if (!sw)
       return;

   disconnect(sw, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
   disconnect(sw, SIGNAL(widgetRemoved(int)), this, SLOT(widgetRemoved(int)));
   Items::iterator it = items.begin(), end = items.end();
   while (it != end) {
       if (it.key().isNull()) {
           it = items.erase(it);
           continue;
       }
       if (it.key().data() == sw) {
           items.erase(it);
           break;
       }
       ++it;
   }

   if (items.isEmpty())
       timer.stop();
}

void
Tab::changed(int index)
{
    if (_transition == Jump || QCoreApplication::closingDown())
        return; // ugly nothing ;)

   // ensure this is a qtabwidget - we'd segfault otherwise
   QStackedWidget *sw = qobject_cast<QStackedWidget*>(sender());
   if (!(sw && sw->isVisible())) return;

   // find matching tabinfo
   Items::iterator i = items.find(sw);
   if (i == items.end())
      return; // not handled... why ever (i.e. should not happen by default)
   // update position
   if (QTabWidget *tw = qobject_cast<QTabWidget*>(sw->parentWidget()))
       i.value()->tabPosition = tw->tabPosition();
   // init transition
   i.value()->switchTab(sw, index);

   // _activeTabs is counted in the timerEvent(), so if this is the first
   // changing tab in a row, it's currently '0'
   if (!_activeTabs) timer.start(timeStep, this);
}

void
Tab::widgetRemoved(int index)
{
    if (_transition == Jump)
        return; // ugly nothing ;)

    // ensure this is a qtabwidget - we'd segfault otherwise
    QStackedWidget *sw = qobject_cast<QStackedWidget*>(sender());
    if (!(sw && sw->isVisible())) return;

    // find matching tabinfo
    Items::iterator i = items.find(sw);
    if (i == items.end())
        return;
    if (i.value()->index == index)
        i.value()->index = -1;
}


void
Tab::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != timer.timerId() || items.isEmpty())
        return;

    Items::iterator i;
    _activeTabs = 0; // reset counter
    bool mkProper = false;
    for (i = items.begin(); i != items.end(); i++)
    {
        if (i.key().isNull())
        {
            mkProper = true;
            continue;
        }
        if (i.value()->proceed())
            ++_activeTabs;
    }
    if (mkProper)
        _release(NULL);
    if (!_activeTabs)
        timer.stop();
}
