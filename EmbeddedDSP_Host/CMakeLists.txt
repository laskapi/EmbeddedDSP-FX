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
    mainwindow.cpp
    mainwindow.h
    EffectSpec.h
    IDeviceBackend.h
    Logging.h Logging.cpp
    SerialBackend.h
    SerialBackend.cpp
    AudioFrameWorker.h
    AudioFrameWorker.cpp
    FftProcessor.h FftProcessor.cpp
    AppController.h AppController.cpp
    UI/Spectrum/SpectrumCommon.h
    UI/Spectrum/SpectrumView.h UI/Spectrum/SpectrumView.cpp
    UI/Spectrum/SpectrumAxisX.h UI/Spectrum/SpectrumAxisX.cpp
    UI/Spectrum/SpectrumAxisY.h UI/Spectrum/SpectrumAxisY.cpp
    UI/Spectrum/SpectrumPanel.h UI/Spectrum/SpectrumPanel.cpp
    UI/ConnectionToolbar.h UI/ConnectionToolbar.cpp
    UI/AbstractRackItem.h
    UI/ParameterItem.h UI/ParameterItem.cpp
    UI/EffectItem.h UI/EffectItem.cpp
    UI/ParameterRack.h UI/ParameterRack.cpp
    UI/EffectRack.h UI/EffectRack.cpp
    UI/AbstractRack.h
)

# Conditionally add simulator files to build
if(ENABLE_SIMULATOR)
    target_sources(EmbeddedDSP_Host PRIVATE
        AudioFrameSimulator.h AudioFrameSimulator.cpp
        DemoManifestProvider.h DemoManifestProvider.cpp
        SimulatorBackend.h SimulatorBackend.cpp
    )
endif()

# Embed resources directly using Qt6 build system to avoid qrc caching issues
qt_add_resources(EmbeddedDSP_Host "styles"
    PREFIX "/"
    FILES
        style.qss
)

# Include directories: local source, UI components, and firmware protocols
target_include_directories(EmbeddedDSP_Host PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/Spectrum
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
