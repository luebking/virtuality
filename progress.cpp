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

#include <QAbstractItemView>
#include "draw.h"
#include "animator/aprogress.h"

#include <QtDebug>

static int step = -1;

void
Style::drawCapacityBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(cb, ProgressBar);
    if (cb->maximum == cb->minimum)
        return;

    OPT_ENABLED
    SAVE_PAINTER(Pen|Brush|Alias);
    const_cast<QStyleOption*>(option)->rect.adjust(F(1),0,-F(1),0);
    int x = RECT.width()*cb->progress/(cb->maximum - cb->minimum);
    if (cb->direction == Qt::LeftToRight)
        x = RECT.right() - x;
    else
        x = RECT.x() + x;
    QRect r(RECT.adjusted(F(1), F(1), -F(1), -F(1)));
    int radius = qMin(r.width(), r.height())/2;
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(GROOVE_COLOR, F(2) & ~1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(r, radius, radius);

    r.adjust(F(1), F(1), -F(1), -F(1));
    if (cb->direction == Qt::LeftToRight) r.setRight(x); else r.setLeft(x);
    radius = qMin(r.width(), r.height())/2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(THERMOMETER_COLOR);
    painter->drawRoundedRect(r, radius, radius);

    if (cb->textVisible && !cb->text.isEmpty()) {
        const int tw = painter->fontMetrics().width(cb->text);
        int align = Qt::AlignCenter;
        if (tw <= r.width()) {   // paint on free part
            painter->setPen(GROOVE_COLOR);
            r = RECT;
            if (cb->direction == Qt::LeftToRight) r.setRight(x); else r.setLeft(x);
        }
        else if (tw <= RECT.width() - r.width()) { // paint on used part
            painter->setPen(THERMOMETER_COLOR);
            r = RECT;
            align = Qt::AlignLeft;
            if (cb->direction == Qt::LeftToRight) r.setLeft(x + F(2)); else r.setRight(x - F(2));
        } else {   // paint centered
            r = RECT;
            painter->setPen(FX::blend(FCOLOR(Window), THERMOMETER_COLOR, 1, 1));
        }
        drawItemText(painter, r, align, PAL, isEnabled, cb->text);
    }
    const_cast<QStyleOption*>(option)->rect.adjust(-F(1),0,F(1),0);
    RESTORE_PAINTER
}

void
Style::drawSimpleProgress(const QStyleOptionProgressBar *option, QPainter *painter, const QWidget *widget, bool isListView) const
{   // TODO: widget doesn't set a state - make bug report!
    OPT_ENABLED;
    if (appType == KTorrent)
        isEnabled = true; // ....

    const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2*>(option);
    QPalette::ColorRole fg, bg;
    if (isListView) {
        if (option->state & State_Selected)
            { fg = QPalette::HighlightedText; bg = QPalette::Highlight; }
        else
            { fg = QPalette::Text; bg = QPalette::Base; }
    } else {
        if (widget)
            { fg = widget->foregroundRole(); bg = widget->backgroundRole(); }
        else
            { fg = QPalette::WindowText; bg = QPalette::Window; }
    }

    bool reverse = option->direction == Qt::RightToLeft;
    if (pb2 && pb2->invertedAppearance)
        reverse = !reverse;
    const bool vertical = (pb2 && pb2->orientation == Qt::Vertical);
    double val = option->progress / double(option->maximum - option->minimum);

    QString text = option->text.isEmpty() ? QString(" %1% ").arg((int)(val*100)) : " " + option->text + " ";
    QRect r = painter->boundingRect(RECT, Qt::AlignLeft | Qt::AlignVCenter, text);

    SAVE_PAINTER(Pen);
    const int hght = isListView ? F(3) : RECT.height();
    painter->setPen(QPen(FX::blend(COLOR(fg), COLOR(bg), 1, 3), hght));
    if (vertical)
    {
        painter->drawLine(RECT.x(), RECT.top(), RECT.x(), RECT.bottom());
        painter->setPen(QPen(COLOR(fg), F(3)));
        r.moveBottom(RECT.bottom() - val*(RECT.height()-r.height()));
        painter->drawLine(RECT.x(), RECT.bottom()-val*RECT.height(), RECT.x(), RECT.bottom());
    }
    else
    {
        painter->drawLine(RECT.left(), RECT.bottom(), RECT.right(), RECT.bottom());
        painter->setPen(QPen(COLOR(fg), hght));
        const int d = val*(RECT.width()-r.width());
        if (reverse)
        {
            r.moveRight(RECT.right() - d);
            painter->drawLine(RECT.right()-val*RECT.width(), RECT.bottom(), RECT.right(), RECT.bottom());
        }
        else
        {
            r.moveLeft(RECT.left() + d);
            painter->drawLine(RECT.left(), RECT.bottom(), RECT.left()+val*RECT.width(), RECT.bottom());
        }
    }
    if (isListView)
        drawItemText(painter, r, Qt::AlignHCenter|Qt::AlignTop, PAL, isEnabled, text);
    RESTORE_PAINTER
}

void
Style::drawProgressBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(pb, ProgressBar);
    OPT_HOVER

    bool listView = (!widget && (appType == KGet || appType == KTorrent || appType == Apper)) || qobject_cast<const QAbstractItemView*>(widget);
    if (listView || RECT.height() < F(9)) // if things get tiny, text will not work, neither will the dots be really visible
    {   // kinda inline progress in itemview (but unfortunately kget doesn't send a widget)
        drawSimpleProgress(pb, painter, widget, listView);
        return;
    }

    // groove + contents ======
    if (widget && widget->testAttribute(Qt::WA_OpaquePaintEvent))
        erase(option, painter, widget);
    step = Animator::Progress::step(widget);
    drawProgressBarGroove(pb, painter, widget);
    drawProgressBarContents(pb, painter, widget);
    // label? =========
    if (hover && pb->textVisible)
        drawProgressBarLabel(pb, painter, widget);
    // reset step!
    step = -1;
}

void
Style::drawProgressBarGC(const QStyleOption *option, QPainter *painter, const QWidget *widget, bool content) const
{
    if (appType == GTK && !content)
        return; // looks really crap

    ASSURE_OPTION(pb, ProgressBar);
    const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2*>(pb);

    bool reverse = option->direction == Qt::RightToLeft;
    if (pb2 && pb2->invertedAppearance)
        reverse = !reverse;
    const bool vertical = (pb2 && pb2->orientation == Qt::Vertical);

    const bool busy = pb->maximum == 0 && pb->minimum == 0;
    int x,y,l,t;
    RECT.getRect(&x,&y,&l,&t);

    if (vertical) // swap width & height...
        { int h = x; x = y; y = h; l = RECT.height(); t = RECT.width(); }

    double val = 0.0;
    if (busy && content) {   // progress with undefined duration / stepamount
        if (step < 0)
            step = Animator::Progress::step(widget);
        val = - Animator::Progress::speed() * step / l;
    }
    else
        val = pb->progress / double(pb->maximum - pb->minimum);

    // maybe there's nothing to do for us
    if (content)
        { if (val == 0.0) return; }
    else if (val == 1.0)
        return;

    // calculate chunk dimensions - minimal 16px or space for 10 chunks, maximal the progress thickness
    int s = qMin(qMax(l/10, F(16)), qMin(t, F(20)));
    if (!s) return;
    int ss = (2*s)/3;
    int n = l/s;
    if (!n) return;
    if (vertical || reverse) {
        x = vertical ? RECT.bottom() : RECT.right();
        x -= ((l - n*s) + (s - ss))/2 + ss;
        s = -s;
    }
    else
        x += (l - n*s + s - ss)/2;
    y += (t-ss)/2;
    --x; --y;

    static QPixmap renderPix;
    // cause most chunks will look the same we render ONE into a buffer and then just dump that multiple times...
    if (renderPix.width() != ss+2) {
        renderPix = QPixmap(ss+2, ss+2);
        renderPix.fill(Qt::transparent);
    }

    QPainter p(&renderPix);
    p.setRenderHint(QPainter::Antialiasing);

    // draw a chunk
    const int miniSize = F(4) + (renderPix.rect().width() & 1);
    int nn = (val < 0) ? 0 : int(n*val);
    p.setBrush(THERMOMETER_COLOR);
    p.setPen(Qt::NoPen);
    if (content)
        p.drawEllipse(renderPix.rect());
    else {   // this is the "not-yet-done" part - in case we're currently painting it...
        if (busy)
            nn = n;
        else
            { x += nn*s; nn = n - nn; }
        QRect r(0,0,miniSize,miniSize);
        r.moveCenter(renderPix.rect().center());
        p.drawEllipse(r);
    }
    p.end();

    if (vertical) // x is in fact y!
        for (int i = 0; i < nn; ++i)
            { painter->drawPixmap(y,x, renderPix); x+=s; }
    else // x is as expected... - gee my math teacher was actually right: "always label the axis!"
        for (int i = 0; i < nn; ++i)
            { painter->drawPixmap(x,y, renderPix); x+=s; }

    // cleanup for later
    renderPix.fill(Qt::transparent);

    // if we're painting the actual progress, ONE chunk may be "semifinished" - that's done below
    if (content) {
        bool b = (nn < n);
        if (busy) {   // the busy indicator has always a semifinished item, but we need to calculate which first
            b = true;
            val = -val; nn = int(n*val); x += nn*s;
            double o = n*val - nn;
            if (o < .5)
                val += o/n;
            else
                val += (1.0-2*o)/n;
        }
        if (b) {
            int q = int((10*n)*val) - 10*nn;
            if (q) {
                const QColor c = busy ? THERMOMETER_COLOR : FX::blend(FCOLOR(Window), THERMOMETER_COLOR, 4-qAbs(q-5), q);
                const int d = ((9-q)*(ss-miniSize)/18);

                if (vertical) // swap again, we abuse 'q' from above
                    { q = x; x = y; y = q; }

                SAVE_PAINTER(Pen|Brush|Alias);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setBrush(c);
                painter->setPen(Qt::NoPen);
                painter->drawEllipse(x+1+d,y+1+d,ss-2*d,ss-2*d);
                RESTORE_PAINTER
            }
        }
    }
}

void
Style::drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget*) const
{
    ASSURE_OPTION(progress, ProgressBarV2);
    OPT_HOVER

    if (!(hover && progress->textVisible))
        return;

    painter->save();
    QRect rect = RECT;
    if (progress->orientation == Qt::Vertical)
    {   // vertical progresses have text rotated by 90° or 270°
        QMatrix m;
        int h = rect.height(); rect.setHeight(rect.width()); rect.setWidth(h);
        if (progress->bottomToTop)
            { m.translate(0.0, RECT.height()); m.rotate(-90); }
        else
            { m.translate(RECT.width(), 0.0); m.rotate(90); }
        painter->setMatrix(m);
    }
    int flags = Qt::AlignCenter | Qt::TextSingleLine;
    QRect tr = painter->boundingRect(rect, flags, progress->text);
    if (!tr.isValid())
        { painter->restore(); return; }
    tr.adjust(-F(6), -F(3), F(6), F(3));
    int radius = qMin(tr.width(), tr.height())/2;
    QColor bg = FCOLOR(Window); bg.setAlpha(200);
    painter->setBrush(bg);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(tr, radius, radius);
    painter->setPen(FCOLOR(WindowText));
    painter->drawText(rect, flags, progress->text);
    painter->restore();
}

//    case PE_IndicatorProgressChunk: // Section of a progress bar indicator; see also QProgressBar.
