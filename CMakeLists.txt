project(virtuality)
cmake_minimum_required(VERSION 2.6)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )
if(WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
#     add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif(WIN32)

option(ENABLE_KDE "Enable Bespin support for KDE4" ON)
# option(ENABLE_KWIN "Build Bespin KWin decoration" OFF)
option(WITH_QT5 "Build Qt5 Style instead of Qt4" OFF)

if (WITH_QT5)
    set(ENABLE_KDE OFF)
endif (WITH_QT5)

if (ENABLE_KDE)
    find_package(KDE4)
endif (ENABLE_KDE)

find_package(X11)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(revision.h ALL COMMAND ${CMAKE_COMMAND} ARGS -P ${CMAKE_CURRENT_SOURCE_DIR}/revision.cmake)
set_property(SOURCE revision.cpp APPEND PROPERTY OBJECT_DEPENDS revision.h)

# add_definitions (-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

if (KDE4_FOUND)
   include (KDE4Defaults)
   include (MacroLibrary)
   add_definitions ( ${KDE4_DEFINITIONS} )
   include_directories (${KDE4_INCLUDES})
else (KDE4_FOUND)
    message (STATUS "WARNING: *** KDE4 not found, just the style will be built ***")
    if (WITH_QT5)
        find_package(Qt5Core REQUIRED)
        find_package(Qt5Gui REQUIRED)
        find_package(Qt5Widgets REQUIRED)
        find_package(Qt5X11Extras)
        find_package(Qt5DBus)
        include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS})
        add_definitions(${Qt5Widgets_DEFINITIONS})
        set (QT_QTDBUS_FOUND ${Qt5DBus_FOUND})
    else (WITH_QT5)
        add_definitions( ${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} )
        set (QT_MIN_VERSION "4.8.0")
        find_package (Qt4 REQUIRED)
        include (UseQt4)
    endif (WITH_QT5)
    set (CMAKE_INCLUDE_CURRENT_DIR ON)
endif (KDE4_FOUND)

# add_subdirectory (config)

add_definitions ( -DBLIB_EXPORT=Q_DECL_IMPORT )

set (virtuality_SOURCES animator/basic.cpp animator/aprogress.cpp animator/hover.cpp
animator/hoverindex.cpp animator/hovercomplex.cpp animator/tab.cpp
FX.cpp dpi.cpp shapes.cpp shadows.cpp
virtuality.cpp buttons.cpp docks.cpp frames.cpp hacks.cpp init.cpp
input.cpp menus.cpp pixelmetric.cpp polish.cpp progress.cpp qsubcmetrics.cpp
scrollareas.cpp indicators.cpp sizefromcontents.cpp slider.cpp stdpix.cpp stylehint.cpp
tabbing.cpp toolbars.cpp views.cpp window.cpp revision.cpp)

set (virtuality_HDRS animator/basic.h animator/aprogress.h animator/hover.h
animator/hoverindex.h animator/tab.h config.h draw.h hacks.h splitterproxy.h virtuality.h types.h)


file(GLOB virtuality_HDRS *.h)
list(APPEND virtuality_HDRS animator/basic.h animator/aprogress.h animator/hover.h animator/hoverindex.h animator/hovercomplex.h animator/tab.h)

if ( X11_FOUND )
    add_definitions (-DBE_WS_X11)
    set (virtuality_SOURCES ${virtuality_SOURCES} xproperty.cpp)
endif ( X11_FOUND )

set (virtuality_MOC_HDRS animator/basic.h animator/aprogress.h animator/hover.h
animator/hoverindex.h animator/tab.h virtuality.h hacks.h)

if(QT_QTDBUS_FOUND)
    message (STATUS "QtDbus available - Style will support XBar")
    set (virtuality_MOC_HDRS ${virtuality_MOC_HDRS} macmenu.h macmenu-dbus.h)
    set (virtuality_HDRS ${virtuality_HDRS} macmenu.h macmenu-dbus.h)
    set (virtuality_HDRS ${virtuality_SOURCES} macmenu.cpp)
else(QT_QTDBUS_FOUND)
    message (STATUS "WARNING: *** QtDBus  not found ***\n   ---> No KWin deco and no XBar for you")
    set (ENABLE_KWIN OFF)
    add_definitions ( -DQT_NO_DBUS )
endif(QT_QTDBUS_FOUND)

if (WITH_QT5)
    qt5_wrap_cpp(virtuality_MOC_SRCS ${virtuality_MOC_HDRS})
else (WITH_QT5)
    qt4_wrap_cpp(virtuality_MOC_SRCS ${virtuality_MOC_HDRS})
endif (WITH_QT5)

add_library (virtuality SHARED ${virtuality_SOURCES} ${virtuality_MOC_SRCS} ${virtuality_HDRS})


if ( X11_FOUND )
    target_link_libraries(virtuality ${X11_X11_LIB})
endif( X11_FOUND )
if (WITH_QT5)
    target_link_libraries(virtuality Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
    if ( X11_FOUND )
        target_link_libraries(virtuality Qt5X11Extras)
endif( X11_FOUND )
else ()
    target_link_libraries(virtuality ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY})
endif (WITH_QT5)

if (WITH_QT5)
    install (TARGETS virtuality DESTINATION ${_qt5Widgets_install_prefix}/lib/qt/plugins/styles)
else (WITH_QT5)
    if (WIN32)
        install (TARGETS virtuality DESTINATION ${QT_PLUGINS_DIR}/styles)
    else (WIN32)
        install (TARGETS virtuality LIBRARY DESTINATION ${QT_PLUGINS_DIR}/styles)
    endif (WIN32)
endif (WITH_QT5)

add_subdirectory (config)

if (KDE4_FOUND)
    install (FILES virtuality.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
    if (X11_Xrender_FOUND)
        if(ENABLE_KWIN)
            add_subdirectory (kwin)
        else(ENABLE_KWIN)
            message (STATUS "WARNING: *** KWin decoration will not be built ***")
        endif(ENABLE_KWIN)
    endif (X11_Xrender_FOUND)
    if ( X11_FOUND )
    endif ( X11_FOUND )
endif (KDE4_FOUND)
