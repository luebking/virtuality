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

#ifndef BESPIN_MAKROS_H
#define BESPIN_MAKROS_H

#define CLAMP(x,l,u) (x) < (l) ? (l) :\
(x) > (u) ? (u) :\
(x)

#define _IsNotHtmlWidget(w) ( w->objectName() != "__khtml" )
#define _IsHtmlWidget(w) ( w->objectName() == "__khtml" )
#define _IsViewportChild(w) w->parent() &&\
( w->parent()->objectName() == "qt_viewport" || \
  w->parent()->objectName() == "qt_clipped_viewport" )

#define _HighContrastColor(c) (qGray(c.rgb()) < 128 ) ? Qt::white : Qt::black

#define _IsTabStack(w) ( w->objectName() == "qt_tabwidget_stackedwidget" )

#define RECT option->rect
#define PAL option->palette
#define COLOR(_ROLE_) PAL.color(_ROLE_)
#define CONF_COLOR(_TYPE_, _FG_) PAL.color(Style::config._TYPE_##_role[_FG_])
#define CCOLOR(_TYPE_, _FG_) PAL.color(Style::config._TYPE_##_role[_FG_])
#define FCOLOR(_TYPE_) PAL.color(QPalette::_TYPE_)
#define BGCOLOR (widget ? COLOR(widget->backgroundRole()) : FCOLOR(Window))
#define FGCOLOR (widget ? COLOR(widget->foregroundRole()) : FCOLOR(WindowText))
#define GRAD(_TYPE_) Style::config._TYPE_.gradient
#define ROLES(_TYPE_) QPalette::ColorRole (*role)[2] = &Style::config._TYPE_##_role
#define ROLE (*role)

#define ASSURE_OPTION(_VAR_, _TYPE_) \
const QStyleOption##_TYPE_ *_VAR_ = qstyleoption_cast<const QStyleOption##_TYPE_ *>(option);\
if (!_VAR_) return

#define HAVE_OPTION(_VAR_, _TYPE_)\
(const QStyleOption##_TYPE_ *_VAR_ = qstyleoption_cast<const QStyleOption##_TYPE_ *>(option))

#define ASSURE_WIDGET(_VAR_, _TYPE_)\
const _TYPE_ *_VAR_ = qobject_cast<const _TYPE_*>(widget);\
if (!_VAR_) return

#define ASSURE(_VAR_) if (!_VAR_) return

#define F(_I_) BE::Dpi::target.f##_I_

#define IS_HTML_WIDGET (widget->objectName() == "RenderFormElementWidget")

#endif //BESPIN_MAKROS_H
