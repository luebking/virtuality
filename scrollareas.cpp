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
#include <QTimer>
#include <QtDebug>
#include "draw.h"
#include "animator/hover.h"
#include "animator/hovercomplex.h"

#define MAX_STEPS Animator::Hover::maxSteps()

inline static bool
scrollAreaHovered(const QWidget* slider)
{
//     bool scrollerActive = false;
    if (!slider) return true;
    QWidget *scrollWidget = const_cast<QWidget*>(slider);
    if (!scrollWidget->isEnabled())
        return false;
    while (scrollWidget && !(qobject_cast<QAbstractScrollArea*>(scrollWidget) || Animator::Hover::managesArea(scrollWidget)))
        scrollWidget = const_cast<QWidget*>(scrollWidget->parentWidget());
    bool isActive = true;
    if (scrollWidget)
    {
        if (!scrollWidget->underMouse())
            return false;
//         QAbstractScrollArea* scrollWidget = (QAbstractScrollArea*)daddy;
        QPoint tl = scrollWidget->mapToGlobal(QPoint(0,0));
        QRegion scrollArea(tl.x(), tl.y(), scrollWidget->width(), scrollWidget->height());
        QList<QAbstractScrollArea*> scrollChilds = scrollWidget->findChildren<QAbstractScrollArea*>();
        for (int i = 0; i < scrollChilds.size(); ++i)
        {
            QPoint tl = scrollChilds[i]->mapToGlobal(QPoint(0,0));
            scrollArea -= QRegion(tl.x(), tl.y(), scrollChilds[i]->width(), scrollChilds[i]->height());
        }
//         scrollerActive = scrollArea.contains(QCursor::pos());
        isActive = scrollArea.contains(QCursor::pos());
    }
    return isActive;
}

#define PAINT_ELEMENT(_E_)\
if (scrollbar->subControls & SC_ScrollBar##_E_)\
{\
    optCopy.rect = scrollbar->rect;\
    optCopy.state = saveFlags;\
    optCopy.rect = subControlRect(CC_ScrollBar, &optCopy, SC_ScrollBar##_E_, widget);\
    if (optCopy.rect.isValid())\
    {\
        if (!(scrollbar->activeSubControls & SC_ScrollBar##_E_))\
            optCopy.state &= ~(State_Sunken | State_MouseOver);\
        if (info && (info->fades[Animator::In] & SC_ScrollBar##_E_ ||\
                    info->fades[Animator::Out] & SC_ScrollBar##_E_))\
            complexStep = info->step(SC_ScrollBar##_E_);\
        else \
            complexStep = 0; \
        drawScrollBar##_E_(&optCopy, cPainter, widget);\
    }\
}//

static bool isComboDropDownSlider, scrollAreaHovered_;
static int complexStep, widgetStep;


static QPixmap *scrollBgCache = 0;
static QTimer cacheCleaner;
const static QWidget *cachedScroller = 0;
static QPainter *cPainter = 0;

void
Style::clearScrollbarCache()
{
    cacheCleaner.stop(); cachedScroller = 0L;
    delete scrollBgCache; scrollBgCache = 0L;
}

enum SA_Flags { WebKit = 1, ComboBox = 2 };

static const QWidget *last_widget = 0;
static int last_flags = 0;

static void updateLastWidget( const QWidget *widget )
{
    if (widget != last_widget)
    {
        last_widget = widget;
        last_flags = 0;
        if (widget->inherits("QWebView"))
            last_flags |= WebKit;
        else if (widget->testAttribute(Qt::WA_OpaquePaintEvent) &&
                                    widget->parentWidget() &&
                    widget->parentWidget()->parentWidget() &&
                    widget->parentWidget()->parentWidget()->inherits("QComboBoxListView"))
            last_flags |= ComboBox;
    }
}

void
Style::drawScrollAreaCorner(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // ouchhh...!
    if (widget)
    {
        updateLastWidget(widget);
        if (last_flags & WebKit)
            erase(option, painter, widget);
    }
}

void
Style::drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{

    ASSURE_OPTION(scrollbar, Slider);
    SAVE_PAINTER(Pen|Brush|Alias);

    cPainter = painter;
    bool useCache = false, needsPaint = true;
    bool isWebKit = false; // ouchhh... needs some specials
    if (widget)
    {
        updateLastWidget(widget);
        if ((isWebKit = last_flags & WebKit))
        {
            const bool isFrame = RECT.height() == widget->height() || RECT.height() == widget->height() - RECT.width() ||
                                 RECT.width() == widget->width() || RECT.width() == widget->width() - RECT.height();
            if (isFrame)
                last_flags &= ~ComboBox;
            else
                last_flags |= ComboBox;
        }
    }

    // we paint the slider bg ourselves, as otherwise a frame repaint would be
    // triggered (for no sense)
    if (!widget) // fallback ===========
        painter->fillRect(RECT, FCOLOR(Window));

    else if (isWebKit || widget->testAttribute(Qt::WA_OpaquePaintEvent))
    {   /// fake a transparent bg (real transparency leads to frame22 painting overhead)
        // i.e. we erase the bg with the window background or any autofilled element between

        if ( last_flags & ComboBox )
        {   /// catch combobox dropdowns ==========
            painter->fillRect(RECT, PAL.brush(QPalette::Base));
            isComboDropDownSlider = true;
        }

        else
        {   /// default scrollbar ===============
            isComboDropDownSlider = false;

            if (option->state & State_Sunken)
            {   /// use caching for sliding scrollers to gain speed
                useCache = true;
                if (widget != cachedScroller)
                {   // update cache
                    cachedScroller = widget;
                    delete scrollBgCache; scrollBgCache = 0L;
                }
                if (!scrollBgCache || scrollBgCache->size() != RECT.size())
                {   // we need a new cache pixmap
                    cacheCleaner.disconnect();
                    connect(&cacheCleaner, SIGNAL(timeout()), this, SLOT(clearScrollbarCache()));
                    delete scrollBgCache;
                    scrollBgCache = new QPixmap(RECT.size());
                    cPainter = new QPainter(scrollBgCache);
                }
                else
                    needsPaint = false;
            }
            if (needsPaint)
            {
                QPoint offset(0,0);
                if (isWebKit && (option->state & QStyle::State_Horizontal))
                    offset.setY(RECT.height() - widget->height());
                erase(option, cPainter, widget, &offset);
            }
        }
    }
    // =================

    //BEGIN real scrollbar painting                                                                -

    // Make a copy here and reset it for each primitive.
    QStyleOptionSlider optCopy = *scrollbar;
    if (isWebKit)
        optCopy.palette = QApplication::palette();
    State saveFlags = optCopy.state;
    if (scrollbar->minimum == scrollbar->maximum)
        saveFlags &= ~State_Enabled; // there'd be nothing to scroll anyway...

    /// hover animation =================
    if (scrollbar->activeSubControls & SC_ScrollBarSlider)
        { widgetStep = 0; scrollAreaHovered_ = true; }
    else
    {
        widgetStep = Animator::Hover::step(widget);
        scrollAreaHovered_ = !isWebKit && scrollAreaHovered(widget);
    }

    SubControls hoverControls = scrollbar->activeSubControls &
                                (SC_ScrollBarSubLine | SC_ScrollBarAddLine | SC_ScrollBarSlider);
    const Animator::ComplexInfo *info = isWebKit ? 0L : Animator::HoverComplex::info(widget, hoverControls);
    /// ================

    if (cPainter != painter) // unwrap cache painter
        { cPainter->end(); delete cPainter; cPainter = painter; }

    /// Background and groove have been painted
    if (useCache) //flush the cache
    {   cacheCleaner.start(1000);
        painter->drawPixmap(RECT.topLeft(), *scrollBgCache);
    }

    if (config.slider.buttons)
    {   // nasty useless "click-to-scroll-one-single-line" buttons
        PAINT_ELEMENT(SubLine);
        PAINT_ELEMENT(AddLine);
    }

    if ((saveFlags & State_Enabled) && (scrollbar->subControls & SC_ScrollBarSlider))
    {
        optCopy.rect = scrollbar->rect;
        optCopy.state = saveFlags;
        optCopy.rect = subControlRect(CC_ScrollBar, &optCopy, SC_ScrollBarSlider, widget);
        optCopy.rect.adjust(F(2),F(2),-F(2),-F(2));

        if (optCopy.rect.isValid())
        {
            if (!(scrollbar->activeSubControls & SC_ScrollBarSlider))
                optCopy.state &= ~(State_Sunken | State_MouseOver);

            if (scrollbar->state & State_HasFocus)
                optCopy.state |= (State_Sunken | State_MouseOver);

            if (info && (   (info->fades[Animator::In] & SC_ScrollBarSlider) ||
                            (info->fades[Animator::Out] & SC_ScrollBarSlider)   ))
                complexStep = info->step(SC_ScrollBarSlider);
            else
                complexStep = 0;

            drawScrollBarSlider(&optCopy, cPainter, widget);
        }
    }

    isComboDropDownSlider = scrollAreaHovered_ = false;
    widgetStep = complexStep = 0;
    RESTORE_PAINTER
}
#undef PAINT_ELEMENT

void
Style::drawScrollBarButton(const QStyleOption *option, QPainter *painter, const QWidget*, bool up) const
{
    ASSURE_OPTION(opt, Slider);
    SAVE_PAINTER(Pen|Brush|Alias);
    const Navi::Direction dir = (option->state & QStyle::State_Horizontal) ? (up?Navi::W:Navi::E) : (up?Navi::N:Navi::S);

    if (isComboDropDownSlider) {   // gets a classic triangular look and is allways shown
        OPT_HOVER

        painter->setPen(Qt::NoPen);
        painter->setBrush(hover ? FCOLOR(Text) : FX::blend(FCOLOR(Base), FCOLOR(Text)));
//         const int dx = RECT.width()/4, dy = RECT.height()/4;
//         QRect rect = RECT.adjusted(dx, dy, -dx, -dy);
        drawArrow(dir, RECT, painter);
        RESTORE_PAINTER
        return;
    }

    if (!config.slider.buttons)
        return;

    OPT_ENABLED OPT_HOVER
    bool alive = isEnabled && // visually inactivate if an extreme position is reached
                ((up && opt->sliderValue > opt->minimum) || (!up && opt->sliderValue < opt->maximum));
    hover = hover && alive;
    const int step = !alive ? 0 : ((hover && !complexStep) ? MAX_STEPS : complexStep);
    const QColor c = FX::blend(FCOLOR(Window), FCOLOR(WindowText), MAX_STEPS-step, MAX_STEPS);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawEllipse(RECT.adjusted(F(1),F(1),-F(1),-F(1)));
    RESTORE_PAINTER
}

void
Style::drawScrollBarSlider(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    OPT_SUNKEN OPT_ENABLED OPT_HOVER
    const bool horizontal = option->state & QStyle::State_Horizontal ||
                             // at least opera doesn't propagate this
                             RECT.width() > RECT.height();

    if (isComboDropDownSlider) {   //gets a special slimmer and simpler look
        SAVE_PAINTER(Pen|Brush|Alias);
        QRect r = RECT;
        if (horizontal) {
            const int d = RECT.height()/3;
            r.adjust(F(2), d, -F(2), -d);
        } else {
            const int d = RECT.width()/3;
            r.adjust(d, F(2), -d, -F(2));
        }
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);
        if (sunken || (hover && !complexStep))
            complexStep = MAX_STEPS;

        painter->setBrush(FX::blend(FCOLOR(Base), FCOLOR(Text), MAX_STEPS-complexStep, complexStep+1));
        painter->drawRoundedRect(r, F(4), F(4));
        RESTORE_PAINTER
        return;
    }

    if (!isEnabled)
        return;

    // --> we need to paint a slider
    SAVE_PAINTER(Pen|Brush|Alias);

    // COLOR: the hover indicator (inside area)
#define SCROLL_COLOR(_X_) (widgetStep ? FX::blend(  bgC, fgC, MAX_STEPS - _X_, _X_) : bgC)

    if (scrollAreaHovered_ && !widgetStep)
        widgetStep = MAX_STEPS;

    QColor c, bgC = FCOLOR(Window), fgC = FCOLOR(WindowText);

    if ( widget && widget->isActiveWindow() ) {
        if (!widgetStep)
            widgetStep = 2;
        else if (!(hover || scrollAreaHovered_))
            widgetStep = qMax(2, widgetStep);
    }

    if (sunken) {
        c = SCROLL_COLOR(MAX_STEPS);
        complexStep = MAX_STEPS;
    }
    else if (complexStep)
    {
        c = FX::blend(bgC, SCROLL_COLOR(widgetStep));
        c = FX::blend(c, SCROLL_COLOR(complexStep), MAX_STEPS-complexStep, complexStep);
    }
    else if (hover)
        { complexStep = MAX_STEPS; c = SCROLL_COLOR(MAX_STEPS); }
    else if (widgetStep)
        c = FX::blend(bgC, SCROLL_COLOR(widgetStep));
    else
        c = bgC;
    c.setAlpha(255); // bg could be transparent, i don't want scrollers translucent, though.
#undef SCROLL_COLOR

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->setRenderHint(QPainter::Antialiasing, true);
    const int radius = qMin(RECT.width(), RECT.height()) / 2;
    painter->drawRoundedRect(RECT, radius, radius);
    painter->setRenderHint(QPainter::Antialiasing, false);
    RESTORE_PAINTER
}

//    case CE_ScrollBarFirst: // Scroll bar first line indicator (i.e., home).
//    case CE_ScrollBarLast: // Scroll bar last line indicator (i.e., end).
