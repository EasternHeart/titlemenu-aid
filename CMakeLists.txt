
project(kwin4_effect_titlemenu)

set (KWIN_HEADER_FOUND true )

if (EXISTS ${CMAKE_INSTALL_PREFIX}/include/kwineffects.h)
    MESSAGE( STATUS "GOOD: KWin headers found" )
else()
    MESSAGE( "\n\nWARNING: KWin headers NOT found, compiling might fail.\n\n  -> Ensure you've installed the kdebase[-workspace]-dev package\n\n" )
endif()

cmake_minimum_required(VERSION 2.4)
add_definitions( ${QT_DEFINITIONS} )
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

find_package(KDE4)
add_definitions ( ${KDE4_DEFINITIONS} )

find_package(X11)
include (KDE4Defaults)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUSMENUQT REQUIRED dbusmenu-qt>=0.6.0)
find_package(QJSON REQUIRED)
include (MacroLibrary)
include_directories (${KDE4_INCLUDES}
    ${DBUSMENUQT_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${QJSON_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR})

set( kwin4_effect_titlemenu_SOURCES menubutton.cpp menubar.cpp titlemenu.cpp registrar.cpp gtkicontable.cpp menucloner.cpp)

qt4_add_dbus_adaptor(kwin4_effect_titlemenu_SOURCES com.canonical.AppMenu.Registrar.xml
    registrar.h Registrar)

qt4_add_dbus_adaptor(kwin4_effect_titlemenu_SOURCES com.canonical.AppMenu.Renderer.xml
    titlemenu.h KWin::TitleMenuEffect)

kde4_add_plugin (kwin4_effect_titlemenu ${kwin4_effect_titlemenu_SOURCES})

target_link_libraries(kwin4_effect_titlemenu ${KDE4_KDEUI_LIBS} kwineffects ${X11_LIBRARIES} ${DBUSMENUQT_LIBRARIES} ${QJSON_LIBRARIES} ${KDE4_PLASMA_LIBS})

install (TARGETS kwin4_effect_titlemenu DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES titlemenu.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kwin )

project (kcm_kwin4_effect_titlemenu)
set( kcm_kwin4_effect_titlemenu_SOURCES titlemenu_config.cpp )
kde4_add_ui_files( kcm_kwin4_effect_titlemenu_SOURCES titlemenu_config.ui )
kde4_add_plugin (kcm_kwin4_effect_titlemenu ${kcm_kwin4_effect_titlemenu_SOURCES})
target_link_libraries(kcm_kwin4_effect_titlemenu ${KDE4_KDEUI_LIBS} kwineffects)

install (TARGETS kcm_kwin4_effect_titlemenu DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES titlemenu_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kwin )
