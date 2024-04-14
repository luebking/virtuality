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

#include <QLayoutItem>
#include <QListView>
#include <QFormLayout>
#include <QPainterPath>
#include <QTableView>
#include <QTreeView>
#include <QTextEdit>

#include <QStyleOptionFrame>

#include "draw.h"

#include "debug.h"

#include <QtDebug>

QPainterPath framePath(const QRect &rect, char corners, int rnd, bool hasFocus = false)
{
    STROKED_RECT(r, rect);
    const int dx = hasFocus ? intMin(intMax(r.width()/4, F(32)), r.width()/2) : intMin(r.width()/4,F(32));
    const int dy = hasFocus ? intMin(intMax(r.height()/4, F(32)), r.height()/2) : intMin(r.height()/4,F(32));
    const int rnd_2 = 2*rnd;

    QPainterPath path;

    if (corners & Corner::TopLeft) {
        if (dy > rnd) {
            path.moveTo(r.x(), r.y() + dy);
            path.lineTo(r.x(), r.y() + rnd);
        } else {
            path.moveTo(r.x(), r.y() + rnd);
        }
        path.arcTo(r.x(), r.y(), rnd_2, rnd_2, 180, -90);
        if (dx > rnd) {
            path.lineTo(r.x() + dx, r.y());
        }
    }

    if (corners & Corner::TopRight) {
        if (dy > rnd) {
            path.moveTo(r.right(), r.y() + dy);
            path.lineTo(r.right(), r.y() + rnd);
        } else {
            path.moveTo(r.right(), r.y() + rnd);
        }
        path.arcTo(r.right() - rnd_2, r.y(), rnd_2, rnd_2, 0, 90);
        if (dx > rnd) {
            path.lineTo(r.right() - dx, r.y());
        }
    }

    if (corners & Corner::BottomRight) {
        if (dy > rnd) {
            path.moveTo(r.right(), r.bottom() - dy);
            path.lineTo(r.right(), r.bottom() - rnd);
        } else {
            path.moveTo(r.right(), r.bottom() - rnd);
        }
        path.arcTo(r.right() - rnd_2, r.bottom() - rnd_2, rnd_2, rnd_2, 0, -90);
        if (dx > rnd) {
            path.lineTo(r.right() - dx, r.bottom());
        }
    }

    if (corners & Corner::BottomLeft) {
        if (dy > rnd) {
            path.moveTo(r.x(), r.bottom() - dy);
            path.lineTo(r.x(), r.bottom() - rnd);
        } else {
            path.moveTo(r.x(), r.bottom() - rnd);
        }
        path.arcTo(r.x(), r.bottom() - rnd_2, rnd_2, rnd_2, 180, 90);
        if (dx > rnd) {
            path.lineTo(r.x() + dx, r.bottom());
        }
    }

    return path;
}


void
Style::drawFocusFrame(const QStyleOption *option, QPainter *painter, const QWidget *w) const
{
    if (option->state & State_Selected || option->state & State_MouseOver)
        return; // looks crap...
    if ( w && w->style() != this && w->inherits("QAbstractButton"))
        return; // from QtCssStyle...

    SAVE_PAINTER(Pen);
    painter->setPen(QPen(FCOLOR(Highlight), F(2)));
    const int d = qMin(config.frame.roundness, qMin(RECT.height()/2, RECT.width()/3));
    painter->drawLine(RECT.x() + d, RECT.bottom(), RECT.right() - d, RECT.bottom());
    RESTORE_PAINTER
}

void
Style::drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    OPT_FOCUS
    SAVE_PAINTER(Pen|Brush|Alias);
    // lines via CE_ShapedFrame instead the ugly eventfilter override
    painter->setPen(hasFocus ? FOCUS_FRAME_PEN : FRAME_PEN);
    painter->setBrush(Qt::NoBrush);

    if HAVE_OPTION(frame, Frame) {
        if (frame->frameShape == QFrame::NoFrame) {
            RESTORE_PAINTER
            return;
        }
        painter->setRenderHint(QPainter::Antialiasing, false);
        if (frame->frameShape == QFrame::VLine || frame->frameShape == QFrame::HLine) {
            QPoint p1 = RECT.center();
            if (frame->frameShape == QFrame::HLine && widget && widget->parentWidget())
            if (QFormLayout *fl = qobject_cast<QFormLayout*>(widget->parentWidget()->layout())) {
                for (int i = 0; i < fl->rowCount(); ++i) {
                    if (QLayoutItem *li = fl->itemAt(i, QFormLayout::FieldRole)) {
                        if (li == fl->itemAt(i, QFormLayout::SpanningRole))
                            continue;
                        p1.setX(li->geometry().x() - fl->spacing()/2);
                        break;
                    }
                }
            }
            QPoint p2 = p1;
            if (frame->frameShape == QFrame::VLine) {
                const int d = RECT.height()/8;
                p1.ry() -= d;
                p2.ry() += d;
            } else {
                const int d = RECT.width()/8;
                p1.rx() -= d;
                p2.rx() +=d;
            }
            painter->drawLine(p1, p2);
            RESTORE_PAINTER
            return;
        }
    }

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (widget) {
        QWidget *parent = widget->parentWidget();
        bool fill = widget->autoFillBackground() && parent &&
                    widget->palette().color(widget->backgroundRole()) != parent->palette().color(parent->backgroundRole());
        QColor autoFill;
        if (fill) {
            autoFill = widget->palette().color(widget->backgroundRole());
            painter->setBrush(parent->palette().color(parent->backgroundRole()));
        }
        if (!fill) {
            QWidget *viewport(0);
            if (const QAbstractScrollArea *area = qobject_cast<const QAbstractScrollArea*>(widget))
                viewport = area->viewport();
            else { // Konversation *STILL* uses Qt3Support !!! - unbelievable.
//                 viewport = widget->findChild<QWidget*>("qt_viewport", Qt::FindDirectChildrenOnly); // Qt5 only
                foreach (QObject *o, widget->children()) {
                    if (o->objectName() == "qt_viewport") {
                        viewport = qobject_cast<QWidget*>(o);
                        break;
                    }
                }
            }
            if (viewport) {
                const QWidget *compare = parent && !widget->autoFillBackground() ? parent : widget;
                fill = viewport->autoFillBackground() &&
                    viewport->palette().color(viewport->backgroundRole()) != compare->palette().color(compare->backgroundRole());
                if (fill) {
                    autoFill = viewport->palette().color(viewport->backgroundRole());
                    painter->setBrush(compare->palette().color(compare->backgroundRole()));
                }
            }
        }
        if (fill) {
            painter->setPen(Qt::NoPen);
            painter->drawRect(RECT);
            painter->setBrush(autoFill);
            painter->drawRoundedRect(RECT.adjusted(1,1,-1,-1), config.frame.roundness, config.frame.roundness);
            if (!hasFocus) {
                RESTORE_PAINTER
                return;
            }
            painter->setPen(FOCUS_FRAME_PEN);
            painter->setBrush(Qt::NoBrush);
        }
    }

    char corners = option->direction == Qt::LeftToRight ? Corner::BottomRight : Corner::BottomLeft;
    const QTreeView *view = qobject_cast<const QTreeView*>(widget);
    if (!view || view->isHeaderHidden()) {
        corners |= (option->direction == Qt::LeftToRight ? Corner::TopLeft : Corner::TopRight);
    }
    painter->drawPath(framePath(RECT, corners, config.frame.roundness, hasFocus));
    RESTORE_PAINTER
}

void
Style::drawGroupBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(groupBox, GroupBox);
    OPT_ENABLED
    SAVE_PAINTER(Pen|Font);

    QRect textRect;
    // Title
    if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty())
    {
        QColor textColor = groupBox->textColor;
        QPalette::ColorRole role = QPalette::WindowText;
        // NOTICE, WORKAROUND: groupBox->textColor is black by def. and should be invalid - but it's not
        // so assuming everything is optimized for a black on white world, we assume the
        // CUSTOM groupBox->textColor to be only valid if it's != Qt::black
        // THIS IS A HACK!
        if (textColor.isValid() && textColor != Qt::black) {
            if (!isEnabled)
                textColor.setAlpha(48);
            painter->setPen(textColor);
            role = QPalette::NoRole;
        }
        QStyleOptionGroupBox copy = *groupBox;
        QFont fnt = painter->font();
        if (fnt.pointSize() > 0) {
            fnt.setPointSize(13*fnt.pointSize()/10);
            fnt.setBold(true);
            painter->setFont(fnt);
        }
        copy.fontMetrics = QFontMetrics(painter->font());
        textRect = subControlRect(CC_GroupBox, &copy, SC_GroupBoxLabel, widget);
        drawItemText(painter, textRect, BESPIN_MNEMONIC | Qt::AlignBottom, groupBox->palette, isEnabled, groupBox->text, role);
    }

    // Frame
    if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
        if (textRect.isValid()) {
            QRect oldRect(RECT);
            const_cast<QStyleOptionComplex*>(option)->rect.setTop(textRect.top() + textRect.height()/2);
            drawGroupBoxFrame(option, painter, widget);
            const_cast<QStyleOptionComplex*>(option)->rect = oldRect;
        } else
            drawGroupBoxFrame(option, painter, widget);
    }

    // Checkbox
    // TODO: doesn't hover - yet.
    if (groupBox->subControls & SC_GroupBoxCheckBox) {
        QStyleOptionButton box;
        box.QStyleOption::operator=(*groupBox);
        box.rect = subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
//       box.state |= State_HasFocus; // focus to signal this to the user
        if (groupBox->activeSubControls & SC_GroupBoxCheckBox)
            box.state |= State_MouseOver;
        drawCheckBox(&box, painter, 0L);
    }
    RESTORE_PAINTER
}

void
Style::drawGroupBoxFrame(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    SAVE_PAINTER(Pen|Brush|Alias);
    painter->setPen(FRAME_PEN);
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint(QPainter::Antialiasing, true);

    const char corners = option->direction == Qt::LeftToRight ? Corner::TopRight|Corner::BottomLeft :
                                                                Corner::TopLeft|Corner::BottomRight;
    painter->drawPath(framePath(RECT, corners, config.frame.roundness));
    RESTORE_PAINTER
}
