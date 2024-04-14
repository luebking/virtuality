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

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#else
#include <cmath>
#endif
#include "animator/hovercomplex.h"
#include "draw.h"

#include <QtDebug>

#define THERMOMETER_FACTOR 4

void
Style::drawSliderHandle(const QRect &handle, const QStyleOption *option, QPainter *painter, int step) const
{
    OPT_FOCUS
    SAVE_PAINTER(Pen|Brush|Alias);

    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(FCOLOR(Window));
    painter->drawEllipse(handle);
    const int dx = handle.width() / THERMOMETER_FACTOR;
    const int dy = handle.height() / THERMOMETER_FACTOR;
    const QColor t = hasFocus ? FCOLOR(Highlight) : FCOLOR(WindowText);
    QColor c = FX::blend(FCOLOR(Window), t,4,5);
    c = FX::blend(c, t, 6-step, step);
    painter->setBrush(c);
    painter->drawEllipse(handle.adjusted(dx,dy,-dx,-dy));
    RESTORE_PAINTER
}

void
Style::drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(slider, Slider);
    SAVE_PAINTER(Pen|Brush|Alias);

    OPT_SUNKEN OPT_ENABLED OPT_HOVER

    QRect groove = subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
    QRect handle = subControlRect(CC_Slider, slider, SC_SliderHandle, widget);

    isEnabled = isEnabled && (slider->maximum > slider->minimum);
    hover = isEnabled && (appType == GTK || (hover && (slider->activeSubControls & SC_SliderHandle)));
    sunken = sunken && (slider->activeSubControls & SC_SliderHandle);

    int radius;

    // groove
    if ((slider->subControls & SC_SliderGroove) && groove.isValid()) {

        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing, true);

        // thermometer
        if (slider->minimum > -1 && slider->sliderPosition != slider->minimum) {
            painter->setBrush(THERMOMETER_COLOR);
            QRect thermometer = groove;
            if (slider->orientation == Qt::Horizontal)
                thermometer.adjust(0, thermometer.height()/THERMOMETER_FACTOR, 0, -thermometer.height()/THERMOMETER_FACTOR);
            else
                thermometer.adjust(thermometer.width()/THERMOMETER_FACTOR, 0, -thermometer.width()/THERMOMETER_FACTOR, 0);
            radius = qMin(thermometer.width(), thermometer.height())/2;
            const QPoint c = handle.center();
            if ( slider->orientation == Qt::Horizontal ) {
                bool ltr = slider->direction == Qt::LeftToRight;
                if (slider->upsideDown) ltr = !ltr;
                if (ltr) {
                    groove.setLeft(c.x());
                    thermometer.setRight(c.x());
                } else {
                    groove.setRight(c.x());
                    thermometer.setLeft(handle.left());
                }
            }
            else {
                if (slider->upsideDown) {
                    groove.setBottom(c.y());
                    thermometer.setTop(c.y());
                } else {
                    groove.setTop(c.y());
                    thermometer.setBottom(handle.bottom());
                }
            }
            painter->drawRoundedRect(thermometer, radius, radius);
        }

        if (slider->sliderPosition != slider->maximum || slider->minimum < 0 ) {
            radius = qMin(groove.width(), groove.height())/2;
            painter->setBrush(GROOVE_COLOR);
            painter->drawRoundedRect(groove, radius, radius);
        }

        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    // tickmarks - we paint a centered dotline, no stupid above/below/both nonsense
   if (isEnabled && (slider->subControls & SC_SliderTickmarks) && slider->tickPosition != QSlider::NoTicks) {
       int interval = slider->tickInterval;
       if (interval < 1)
           interval = slider->pageStep;
       if (interval > 0) {
           QPen oldPen = painter->pen();
           const qreal pw = F(1);
           QPen pen(FCOLOR(Window), pw);
           interval = (slider->maximum - slider->minimum) / interval;
           const int s = (slider->orientation == Qt::Horizontal) ? RECT.width() - handle.width() :  RECT.height() - handle.height();
           qreal space = s/(pw*interval) - 1.0f;
           pen.setDashPattern(QVector<qreal>() << 1.0f << space);
           space /= 2.0f;
           pen.setDashOffset(space - 0.5f + handle.width()*0.5f);
           painter->setPen(pen);
           QPoint c = groove.center();
           if (slider->orientation == Qt::Horizontal)
               painter->drawLine(RECT.x() + space, c.y(), RECT.right() - space, c.y());
           else
               painter->drawLine(c.x(), RECT.y() + space, c.x(), RECT.bottom() - space);
           painter->setPen(oldPen);
       }
   }

    // handle ======================================
    if (isEnabled && (slider->subControls & SC_SliderHandle)) {
        int step = 0;
        if (sunken)
            step = 6;
        else if (isEnabled) {
            const Animator::ComplexInfo *info = Animator::HoverComplex::info(widget, slider->activeSubControls & SC_SliderHandle);
            if (info && ( info->fades[Animator::In] & SC_SliderHandle || info->fades[Animator::Out] & SC_SliderHandle ))
                step = info->step(SC_SliderHandle);
            if (hover && !step)
                step = 6;
        }
        drawSliderHandle(handle, option, painter, step);
    }
    RESTORE_PAINTER
}

void
Style::drawDial(const QStyleOptionComplex *option, QPainter *painter, const QWidget *) const
{
    ASSURE_OPTION(dial, Slider);

    OPT_ENABLED OPT_HOVER OPT_FOCUS
    SAVE_PAINTER(Pen|Brush|Alias|Font);

    QRect rect = RECT;
    if (rect.width() > rect.height()) {
        rect.setLeft(rect.x()+(rect.width()-rect.height())/2);
        rect.setWidth(rect.height());
    } else {
        rect.setTop(rect.y()+(rect.height()-rect.width())/2);
        rect.setHeight(rect.width());
    }

    int d = qMin(2*rect.width()/5, config.slider.thickness);
    int r = (rect.width()-d)/2;

    // angle calculation from qcommonstyle.cpp (c) Trolltech 1992-2007, ASA.
    float a;
    if (dial->maximum == dial->minimum)
        a = M_PI / 2;
    else if (dial->dialWrapping)
        a = M_PI * 3 / 2 - (dial->sliderValue - dial->minimum) * 2 * M_PI / (dial->maximum - dial->minimum);
    else
        a = (M_PI * 8 - (dial->sliderValue - dial->minimum) * 10 * M_PI / (dial->maximum - dial->minimum)) / 6;

    QPoint cp = rect.center() + QPoint(qRound(r * cos(a)), -qRound(r * sin(a)));

    // the huge ring =======================================
    r = d/2+1; rect.adjust(r,r,-r,-r);

    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint( QPainter::Antialiasing );
    const int start = -(dial->dialWrapping ? 90 : 120)*16;
    const int span = -16*(dial->dialWrapping ? 360 : 300)*(dial->sliderValue - dial->minimum)/(dial->maximum - dial->minimum);
    QPen pen;
    if (dial->sliderValue != dial->maximum) {
        QPen pen(GROOVE_COLOR, d);
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->drawArc(rect, start + span, -((dial->dialWrapping ? 360 : 300)*16 + span));
    }
    if (dial->sliderValue != dial->minimum) {
        QPen pen(THERMOMETER_COLOR, d - 2*(d/THERMOMETER_FACTOR));
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->drawArc(rect, start, span);
    }

    // the value ==============================================
    QFont fnt = painter->font();
    int h = rect.height()/2;
    h -= 2 * (h - qMin(h, painter->fontMetrics().xHeight())) / 3;
    fnt.setPixelSize( h );
    painter->setFont(fnt);
    painter->setPen(FX::blend(PAL.window().color(), PAL.windowText().color(),!hasFocus,2));
    drawItemText(painter, rect,  Qt::AlignCenter, PAL, isEnabled, QString::number(dial->sliderValue));

    // the drop ===========================
    if (isEnabled) {
        rect = QRect(0,0,d,d);
        rect.moveCenter(cp);
        drawSliderHandle(rect, option, painter, hover * 6);
    }

    RESTORE_PAINTER
}
