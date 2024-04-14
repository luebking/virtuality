# the exec config tool

set (virtuality_config_SOURCES bconfig.cpp config.cpp kdeini.cpp ../revision.cpp)
set (virtuality_config_MOC_HDRS bconfig.h config.h dialog.h)

if (WITH_QT5)
    qt5_add_resources (virtuality_config_SOURCES)
    qt5_wrap_ui (virtuality_config_FRM_HDRS config.ui)
    qt5_wrap_cpp(virtuality_config_MOC_SRCS ${virtuality_config_MOC_HDRS})
    qt5_wrap_ui (virtuality_demo_FRM_HDRS uiDemo.ui)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
elseif (WITH_QT6)
    qt6_add_resources (virtuality_config_SOURCES)
    qt6_wrap_ui (virtuality_config_FRM_HDRS config.ui)
    qt6_wrap_cpp(virtuality_config_MOC_SRCS ${virtuality_config_MOC_HDRS})
    qt6_wrap_ui (virtuality_demo_FRM_HDRS uiDemo.ui)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
else ()
    qt4_add_resources (virtuality_config_SOURCES)
    qt4_wrap_ui (virtuality_config_FRM_HDRS config.ui)
    qt4_wrap_cpp(virtuality_config_MOC_SRCS ${virtuality_config_MOC_HDRS})
    qt4_wrap_ui (virtuality_demo_FRM_HDRS uiDemo.ui)
endif (WITH_QT5)

# add_library(virtuality_config STATIC ${virtuality_config_SOURCES} ${virtuality_config_MOC_SRCS}  ${virtuality_config_FRM_HDRS})

#--------------------------------------------------------

project (virtuality_cfg)

# add_definitions ( -DB_EXPORT=Q_DECL_IMPORT )

add_executable (virtuality_cfg main.cpp ${virtuality_config_SOURCES} ${virtuality_config_MOC_SRCS}  ${virtuality_config_FRM_HDRS} ${virtuality_demo_FRM_HDRS})
if (WITH_QT5)
    target_link_libraries (virtuality_cfg Qt5::Core Qt5::Gui Qt5::Widgets)
elseif (WITH_QT6)
    target_link_libraries (virtuality_cfg Qt6::Core Qt6::Gui Qt6::Widgets)
else (WITH_QT5)
    target_link_libraries (virtuality_cfg ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
endif (WITH_QT5)

SET_TARGET_PROPERTIES(virtuality_cfg PROPERTIES OUTPUT_NAME "virtuality")
if(WIN32)
    install (TARGETS virtuality_cfg DESTINATION ${QT_BINARY_DIR})
else(WIN32)
    install (TARGETS virtuality_cfg DESTINATION bin)
endif(WIN32)

