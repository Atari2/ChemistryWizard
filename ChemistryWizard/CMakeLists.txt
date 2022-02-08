cmake_minimum_required(VERSION 3.8)

project(ChemistryWizardUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.2.2/msvc2019_64")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        ChemistryWizardUI.cpp
        ChemistryWizardUI.h
		ChemistryWizard.cpp
		ChemistryWizard.h
		Actions.h
		Actions.cpp
        ChemistryWizard.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ChemistryWizardUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ChemistryWizardUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ChemistryWizardUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ChemistryWizardUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ChemistryWizardUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_options(ChemistryWizardUI PRIVATE /utf-8)
target_compile_definitions(ChemistryWizardUI PRIVATE _CRT_SECURE_NO_WARNINGS)

set_target_properties(ChemistryWizardUI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ChemistryWizardUI)
    find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES STREQUAL "Release")
    set(WINDEPLOY_CFG "release")
else()
    set(WINDEPLOY_CFG "debug")
endif()

message(STATUS "Building ${WINDEPLOY_CFG} windeploy")
add_custom_command(TARGET ChemistryWizardUI POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXE}"
            --${WINDEPLOY_CFG}
            --no-compiler-runtime
            --no-translations
            "$<TARGET_FILE:ChemistryWizardUI>"
    COMMENT "Running windeployqt..."
)

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES STREQUAL "Release")
    message(STATUS "Building Release, removing qDebug()")
    target_compile_definitions(ChemistryWizardUI PRIVATE QT_NO_DEBUG_OUTPUT)
    target_link_options(ChemistryWizardUI PRIVATE /SUBSYSTEM:windows /ENTRY:mainCRTStartup)
endif()