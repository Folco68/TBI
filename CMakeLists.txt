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

set(PROJECT_SOURCES
    # Root
    BeforeRelease.hpp
    HelpRes.qrc
    Logger.hpp
    Logger.cpp
    main.cpp
    Settings.cpp
    Settings.hpp

    # Docs
    Docs/About.txt
    Docs/BUGS.txt
    Docs/Changelog.txt
    Docs/Help.txt
    Docs/TBformats.txt
    Docs/TODO.txt

    # Icons
    Icon/Icon.qrc
    Icon/Icon.ico
    Icon/Icon.png
    Icon/Icon.rc

    # Index
    Index/Index.cpp
    Index/Index.hpp
    Index/TechnicalBulletin.cpp
    Index/TechnicalBulletin.hpp

    # UI
    UI/ContextMenuAction.cpp
    UI/ContextMenuAction.hpp
    UI/DlgHelp.cpp
    UI/DlgHelp.hpp
    UI/DlgHelp.ui
    UI/DlgSettings.cpp
    UI/DlgSettings.hpp
    UI/DlgHelp.ui
    UI/DlgTB.cpp
    UI/DlgTB.hpp
    UI/DlgTB.ui
    UI/DownloadMenu.cpp
    UI/DownloadMenu.hpp
    UI/LineEditDeselect.cpp
    UI/LineEditDeselect.hpp
    UI/MainWindow.cpp
    UI/MainWindow.hpp
    UI/MainWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TBI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        .gitignore
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TBI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TBI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TBI
            ${PROJECT_SOURCES}
        )
    endif()
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
