cmake_minimum_required(VERSION 3.19)
project(EmbeddedDSP_Host LANGUAGES CXX)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    SerialPort
)

# --- Simulator & Demo Mode Configuration ---
option(ENABLE_SIMULATOR "Enable audio simulator and demo mode" ON)

if(ENABLE_SIMULATOR)
    add_definitions(-DHOST_SIMULATOR_ENABLED)
    message(STATUS "Build: Simulator mode ENABLED")
else()
    message(STATUS "Build: Simulator mode DISABLED")
endif()

qt_standard_project_setup()

qt_add_executable(EmbeddedDSP_Host
    WIN32 MACOSX_BUNDLE
    main.cpp
    UI/mainwindow.cpp
    UI/mainwindow.h
    Models/EffectSpec.h
    Models/ConnectionInfo.h
    Backend/IDeviceBackend.h
    Core/Logging.h Core/Logging.cpp
    Backend/SerialBackend.h
    Backend/SerialBackend.cpp
    Processing/AudioFrameWorker.h
    Processing/AudioFrameWorker.cpp
    Processing/FftProcessor.h Processing/FftProcessor.cpp
    Core/AppController.h Core/AppController.cpp
    UI/Spectrum/SpectrumCommon.h
    UI/Spectrum/SpectrumPlotHost.h UI/Spectrum/SpectrumPlotHost.cpp
    UI/Spectrum/SpectrumPlotView.h UI/Spectrum/SpectrumPlotView.cpp
    UI/Spectrum/SpectrumAxisX.h UI/Spectrum/SpectrumAxisX.cpp
    UI/Spectrum/SpectrumAxisY.h UI/Spectrum/SpectrumAxisY.cpp
    UI/Spectrum/SpectrumPanel.h UI/Spectrum/SpectrumPanel.cpp
    UI/Spectrum/SpectrumPlotWelcomeOverlay.h UI/Spectrum/SpectrumPlotWelcomeOverlay.cpp
    UI/ConnectionToolbar.h UI/ConnectionToolbar.cpp
    UI/Effects/AbstractRackItem.h
    UI/Effects/AbstractRack.h
    UI/Effects/ParameterItem.h UI/Effects/ParameterItem.cpp
    UI/Effects/ParameterRack.h UI/Effects/ParameterRack.cpp
    UI/Effects/EffectItem.h UI/Effects/EffectItem.cpp
    UI/Effects/EffectRack.h UI/Effects/EffectRack.cpp
)

# Conditionally add simulator files to build
if(ENABLE_SIMULATOR)
    target_sources(EmbeddedDSP_Host PRIVATE
        Backend/Demo/DemoAudioSimulator.h Backend/Demo/DemoAudioSimulator.cpp
        Backend/Demo/DemoManifestProvider.h Backend/Demo/DemoManifestProvider.cpp
        Backend/Demo/DemoBackend.h Backend/Demo/DemoBackend.cpp
    )
endif()

# Embed resources directly using Qt6 build system to avoid qrc caching issues
qt_add_resources(EmbeddedDSP_Host "styles"
    PREFIX "/"
    FILES
        UI/style.qss
)

# Include directories: local source, UI components, and firmware protocols
target_include_directories(EmbeddedDSP_Host PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Backend
    ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Demo
    ${CMAKE_CURRENT_SOURCE_DIR}/Processing
    ${CMAKE_CURRENT_SOURCE_DIR}/Models
    ${CMAKE_CURRENT_SOURCE_DIR}/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/Spectrum
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/Effects
    ${CMAKE_CURRENT_SOURCE_DIR}/../EmbeddedDSP_Shared
)

# Link Qt6 libraries
target_link_libraries(EmbeddedDSP_Host
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
)

# Optimize build speed using Precompiled Headers
target_precompile_headers(EmbeddedDSP_Host PRIVATE
    <QApplication>
    <QWidget>
    <QMainWindow>
    <QObject>
    <QString>
    <QVector>
    <QMap>
    <QPainter>
    <QDebug>
    <memory>
    <vector>
    <algorithm>
)

# Disable debug output in Release builds to improve performance and security
target_compile_definitions(EmbeddedDSP_Host PRIVATE
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

install(TARGETS EmbeddedDSP_Host
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET EmbeddedDSP_Host
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
