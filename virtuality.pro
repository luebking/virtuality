HEADERS = animator/basic.h animator/aprogress.h animator/hover.h \
          animator/hoverindex.h animator/hovercomplex.h animator/tab.h \
          FX.h shapes.h dpi.h shadows.h\
          virtuality.h draw.h config.h types.h debug.h hacks.h

SOURCES = animator/basic.cpp animator/aprogress.cpp animator/hover.cpp \
          animator/hoverindex.cpp animator/hovercomplex.cpp animator/tab.cpp \
          dpi.cpp FX.cpp shapes.cpp shadows.cpp \
          virtuality.cpp stylehint.cpp sizefromcontents.cpp qsubcmetrics.cpp \
          pixelmetric.cpp stdpix.cpp  init.cpp genpixmaps.cpp polish.cpp \
          buttons.cpp docks.cpp frames.cpp input.cpp menus.cpp progress.cpp \
          scrollareas.cpp indicators.cpp slider.cpp tabbing.cpp toolbars.cpp \
          views.cpp window.cpp hacks.cpp revision.cpp

TEMPLATE = lib
PLUGIN = true
CONFIG += qt plugin
DEFINES += BLIB_EXPORT=""


# customizable part you may skip if you're on OS-X or M$ ------------------------------------
unix {
#    on unix systems (linux)
   CONFIG += x11
# this can talk to kwin
   HEADERS += xproperty.h
   SOURCES += xproperty.cpp
# not interested in a nice macalike menubar? comment the 3 lines below with a '#'
   HEADERS += macmenu.h macmenu-dbus.h
   SOURCES += macmenu.cpp
   QT += dbus
}
# ------------------------------------------------------------

VERSION       = 0.1
target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
