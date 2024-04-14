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
Style::drawCapacityBar(const QStyleOption *option, QPainter *painter, const QWidget *) const
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
    QRect r = RECT;
    r.adjust(1, 0, -1, 0); // helps with blending artifacts
    int radius = qMin(r.width(), r.height())/2;
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(THERMOMETER_COLOR);
    painter->drawRoundedRect(r, radius, radius);

    r.adjust(-1, 0, 1, 0); // reset, see above
    cb->direction == Qt::LeftToRight ? r.setLeft(x) : r.setRight(x);
    radius = qMin(r.width(), r.height())/2;
    painter->setBrush(GROOVE_COLOR);
    painter->drawRoundedRect(r, radius, radius);

    if (cb->textVisible && !cb->text.isEmpty()) {
        const int tw = painter->fontMetrics().horizontalAdvance(cb->text);
        int align = Qt::AlignCenter;
        if (tw <= r.width()) {   // paint on free part
            painter->setPen(THERMOMETER_COLOR);
        } else if (tw <= RECT.width() - r.width()) { // paint on used part
            painter->setPen(GROOVE_COLOR);
            r = RECT;
            align = Qt::AlignLeft;
            cb->direction == Qt::LeftToRight ? r.setRight(x) : r.setLeft(x);
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
{
    OPT_ENABLED;
    if (isListView) // ktorrent/transmission-qt don't set the state
        isEnabled = true; // ....

    const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
    QPalette::ColorRole fg, bg;
    if (isListView) {
        if (option->state & State_Selected)
            { fg = QPalette::HighlightedText; bg = QPalette::Highlight; }
        else
            { fg = QPalette::Highlight; bg = QPalette::Base; }
    } else if (widget)
        { fg = widget->foregroundRole(); bg = widget->backgroundRole(); }
    else
        { fg = QPalette::WindowText; bg = QPalette::Window; }

    bool reverse = option->direction == Qt::RightToLeft;
    if (pb && pb->invertedAppearance)
        reverse = !reverse;
    const bool vertical = (pb && !(pb->state & QStyle::State_Horizontal));
    double val = option->progress / double(option->maximum - option->minimum);

    SAVE_PAINTER(Pen|Brush|Alias|Font);
    painter->setRenderHint(QPainter::Antialiasing);
    const int hght = (isListView ? F(2) : RECT.height()) & ~1;
    const QRect r(RECT.adjusted(hght/2, hght/2, -hght/2, -hght/2));
    painter->setPen(QPen(FX::blend(COLOR(fg), COLOR(bg), 1, 3), hght, Qt::SolidLine, Qt::RoundCap));
    if (vertical) {
        painter->drawLine(r.x(), r.top(), r.x(), r.bottom());
        painter->setPen(QPen(COLOR(fg), F(4) & ~1));
//         r.moveBottom(r.bottom() - val*(r.height()-r2.height()));
        painter->drawLine(r.x(), r.bottom()-val*r.height(), r.x(), r.bottom());
    } else {
        const int y = r.y() + r.height()/2;
        painter->drawLine(r.left(), y, r.right(), y);
        painter->setPen(QPen(COLOR(fg), hght, Qt::SolidLine, Qt::RoundCap));
        if (reverse) {
            painter->drawLine(r.right()-val*r.width(), y, r.right(), y);
        } else {
            painter->drawLine(r.left(), y, r.left()+val*r.width(), y);
        }
    }
    if (isListView && !vertical) {
        const QString text = option->text.isEmpty() ? " " + QString::number(val*100.0, 'f', 1) + "% " : " " + option->text + " ";
        QFont fnt = painter->font();
        fnt.setBold(true);
        painter->setFont(fnt);
        QRect r2 = painter->boundingRect(RECT, Qt::AlignLeft | Qt::AlignVCenter, text);
        const int d = val*(r.width()-r2.width());
        if (reverse)
            r2.moveRight(r.right() - d);
        else
            r2.moveLeft(r.left() + d);
        const QColor c = painter->pen().color();
        painter->setPen(Qt::NoPen);
        painter->setBrush(COLOR(bg));
        const int rnd = r2.height() / 4;
        painter->drawRoundedRect(r2, rnd, rnd);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(c);
        drawItemText(painter, r2, Qt::AlignCenter, PAL, isEnabled, text);
    }
    RESTORE_PAINTER
}

void
Style::drawProgressBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(pb, ProgressBar);
    OPT_HOVER

    bool listView = false;
    if (!widget && painter->device()->devType() == QInternal::Widget) {
        // kget, transmission-qt, ktorrent, apper and others use thi, but don't provide a widget pointer
        // so we investigate the paintDevice
        widget = static_cast<QWidget*>(painter->device());
        listView = (widget->objectName() == QLatin1String("qt_scrollarea_viewport") &&
                    qobject_cast<const QAbstractItemView*>(widget->parentWidget()));
    }
    if (!listView)
        listView = qobject_cast<const QAbstractItemView*>(widget);

    if (listView || RECT.height() < F(9)) { // if things get tiny, text won't work, nor will the dots be distinct
        // -> kinda inline progress in itemview
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

    if (!widget && appType == QBittorrent) {
        if (content)
            drawSimpleProgress(pb, painter, widget, true);
        return;
    }

    bool reverse = option->direction == Qt::RightToLeft;
    if (pb->invertedAppearance)
        reverse = !reverse;
    const bool vertical = !(pb->state & QStyle::State_Horizontal);

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
    ASSURE_OPTION(progress, ProgressBar);
    OPT_HOVER

    if (!(hover && progress->textVisible))
        return;

    painter->save();
    QRect rect = RECT;
    if (!(progress->state & QStyle::State_Horizontal))
    {   // vertical progresses have text rotated by 90° or 270°
        QTransform m;
        int h = rect.height(); rect.setHeight(rect.width()); rect.setWidth(h);
        if (progress->bottomToTop)
            { m.translate(0.0, RECT.height()); m.rotate(-90); }
        else
            { m.translate(RECT.width(), 0.0); m.rotate(90); }
        painter->setTransform(m);
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
