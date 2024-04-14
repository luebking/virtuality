project(virtuality)
cmake_minimum_required(VERSION 3.5.0)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
#     add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif(WIN32)

option(WITH_QT5 "Build Qt5 Style instead of Qt4" OFF)
option(WITH_QT6 "Build Qt6 Style instead of Qt4" OFF)

find_package(X11)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(revision.h ALL COMMAND ${CMAKE_COMMAND} ARGS -P ${CMAKE_CURRENT_SOURCE_DIR}/revision.cmake)
set_property(SOURCE revision.cpp APPEND PROPERTY OBJECT_DEPENDS revision.h)
if (WITH_QT5 OR WITH_QT6)
    set_property(SOURCE moc_virtuality.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/virtuality.json)
endif ()

# add_definitions (-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

if (WITH_QT5)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5X11Extras)
    find_package(Qt5DBus)
    include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS})
    add_definitions(${Qt5Widgets_DEFINITIONS})
    set (QT_QTDBUS_FOUND ${Qt5DBus_FOUND})
elseif (WITH_QT6)
    find_package(Qt6Core REQUIRED)
    find_package(Qt6Gui REQUIRED)
    find_package(Qt6Widgets REQUIRED)
#    find_package(Qt5X11Extras)
    find_package(Qt6DBus)
    include_directories(${Qt6Widgets_INCLUDE_DIRS} ${Qt6X11Extras_INCLUDE_DIRS})
    add_definitions(${Qt6Widgets_DEFINITIONS})
    set (QT_QTDBUS_FOUND ${Qt6DBus_FOUND})
else ()
    add_definitions( ${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} )
    set (QT_MIN_VERSION "4.8.0")
    find_package (Qt4 REQUIRED)
    include (UseQt4)
endif (WITH_QT5)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

# add_subdirectory (config)

add_definitions ( -DBLIB_EXPORT=Q_DECL_IMPORT )

set (virtuality_SOURCES animator/basic.cpp animator/aprogress.cpp animator/focus.cpp animator/hover.cpp
animator/hoverindex.cpp animator/hovercomplex.cpp animator/tab.cpp
FX.cpp dpi.cpp shapes.cpp shadows.cpp
virtuality.cpp buttons.cpp docks.cpp frames.cpp hacks.cpp init.cpp
input.cpp menus.cpp pixelmetric.cpp polish.cpp progress.cpp qsubcmetrics.cpp
scrollareas.cpp indicators.cpp sizefromcontents.cpp slider.cpp stdpix.cpp stylehint.cpp
tabbing.cpp toolbars.cpp views.cpp window.cpp revision.cpp)

set (virtuality_HDRS animator/basic.h animator/aprogress.h animator/hover.h
animator/hoverindex.h animator/tab.h config.h draw.h hacks.h splitterproxy.h virtuality.h types.h)


file(GLOB virtuality_HDRS *.h)
list(APPEND virtuality_HDRS animator/basic.h animator/aprogress.h animator/focus.h animator/hover.h animator/hoverindex.h animator/hovercomplex.h animator/tab.h)

if ( X11_FOUND )
    add_definitions (-DBE_WS_X11)
    set (virtuality_SOURCES ${virtuality_SOURCES} xproperty.cpp)
endif ( X11_FOUND )

set (virtuality_MOC_HDRS animator/basic.h animator/aprogress.h animator/focus.h animator/hover.h
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
elseif (WITH_QT6)
    qt6_wrap_cpp(virtuality_MOC_SRCS ${virtuality_MOC_HDRS})
else ()
    qt4_wrap_cpp(virtuality_MOC_SRCS ${virtuality_MOC_HDRS})
endif (WITH_QT5)

add_library (virtuality SHARED ${virtuality_SOURCES} ${virtuality_MOC_SRCS} ${virtuality_HDRS})
set_property (TARGET virtuality PROPERTY CXX_STANDARD 11)
set (CXX_STANDARD_REQUIRED ON)


if ( X11_FOUND )
    target_link_libraries(virtuality ${X11_X11_LIB})
endif( X11_FOUND )
if (WITH_QT5)
    target_link_libraries(virtuality Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
    if ( X11_FOUND )
        find_package(ECM 1.0.0 REQUIRED NO_MODULE)
        set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})
        find_package(XCB REQUIRED COMPONENTS XCB)
        target_link_libraries(virtuality Qt5X11Extras XCB::XCB)
    endif( X11_FOUND )
elseif (WITH_QT6)
    target_link_libraries(virtuality Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus)
    if ( X11_FOUND )
        find_package(ECM 1.0.0 REQUIRED NO_MODULE)
        set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})
        find_package(XCB REQUIRED COMPONENTS XCB)
#        target_link_libraries(virtuality Qt5X11Extras XCB::XCB)
        target_link_libraries(virtuality XCB::XCB)
    endif( X11_FOUND )
else ()
    target_link_libraries(virtuality ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY})
endif (WITH_QT5)

if (WITH_QT5)
    if (NOT QT_QMAKE_EXECUTABLE)
        get_target_property (QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    ENDIF(NOT QT_QMAKE_EXECUTABLE)
    EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_PLUGINS" OUTPUT_VARIABLE QTPLUGINS)
#     install (TARGETS virtuality DESTINATION ${_qt5Widgets_install_prefix}/lib/qt/plugins/styles)
    install (TARGETS virtuality DESTINATION ${QTPLUGINS}/styles)
elseif (WITH_QT6)
    if (NOT QT_QMAKE_EXECUTABLE)
        get_target_property (QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    ENDIF(NOT QT_QMAKE_EXECUTABLE)
    EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_PLUGINS" OUTPUT_VARIABLE QTPLUGINS)
    install (TARGETS virtuality DESTINATION ${QTPLUGINS}/styles)
else ()
    if (WIN32)
        install (TARGETS virtuality DESTINATION ${QT_PLUGINS_DIR}/styles)
    else (WIN32)
        install (TARGETS virtuality LIBRARY DESTINATION ${QT_PLUGINS_DIR}/styles)
    endif (WIN32)
endif (WITH_QT5)

add_subdirectory (config)

