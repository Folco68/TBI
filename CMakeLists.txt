cmake_minimum_required(VERSION 3.5)

project(TBI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Warnings
if (MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

set(PROJECT_SOURCES
    # Docs
    Docs/About.txt
    Docs/BUGS.txt
    Docs/Changelog.txt
    Docs/Draft.txt
    Docs/Help.txt
    Docs/TBformats.txt
    Docs/TODO.txt

    # Index
    Index/ThreadIndex.cpp
    Index/ThreadIndex.hpp
    Index/TechnicalBulletin.cpp
    Index/TechnicalBulletin.hpp

    # UI - Misc
    UI/ContextMenuAction.cpp
    UI/ContextMenuAction.hpp
    UI/DownloadMenu.cpp
    UI/DownloadMenu.hpp
    UI/LineEditDeselect.cpp
    UI/LineEditDeselect.hpp

    # UI - Dialogs
    UI/DlgHelp.cpp
    UI/DlgHelp.hpp
    UI/DlgHelp.ui
    UI/DlgSettings.cpp
    UI/DlgSettings.hpp
    UI/DlgSettings.ui
    UI/DlgTB.cpp
    UI/DlgTB.hpp
    UI/DlgTB.ui
    UI/MainWindow.cpp
    UI/MainWindow.hpp
    UI/MainWindow.ui

    # General
    BeforeRelease.hpp
    main.cpp
    HelpRes.qrc
    Settings.cpp
    Settings.hpp
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TBI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Docs/Draft.txt
    )
endif()

target_link_libraries(TBI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(TBI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TBI)
endif()
