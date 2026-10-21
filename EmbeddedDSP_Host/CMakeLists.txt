cmake_minimum_required(VERSION 3.19)
project(EmbeddedDSP_Host LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    SerialPort
)

qt_standard_project_setup()

qt_add_executable(EmbeddedDSP_Host
    WIN32 MACOSX_BUNDLE
    main.cpp
    UI/mainwindow.cpp
    UI/mainwindow.h
    Models/DeviceModel.h
    Models/DeviceManifestParser.h
    Models/DeviceManifestParser.cpp
    Models/ConnectionInfo.h
    Device/IDevice.h
    Device/IDeviceFactory.h
    Device/IConnectionScanner.h
    Device/DeviceFactory.h
    Device/DeviceFactory.cpp
    Device/ConnectionScanner.h
    Device/ConnectionScanner.cpp
    Device/DeviceSession.h
    Device/DeviceSession.cpp
    Device/SerialDevice.h
    Device/SerialDevice.cpp
    Device/Demo/DemoConnectionConstants.h
    Device/Demo/DemoDevice.h
    Device/Demo/DemoDevice.cpp
    Device/Demo/DemoAudioSimulator.h
    Device/Demo/DemoAudioSimulator.cpp
    Device/Demo/DemoManifestProvider.h
    Device/Demo/DemoManifestProvider.cpp
    Core/Logging.h Core/Logging.cpp
    Core/WorkerThread.h Core/WorkerThread.cpp
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
    UI/UiObjectNames.h
    UI/ConnectionToolbar.h UI/ConnectionToolbar.cpp
    UI/Effects/AbstractRackItem.h
    UI/Effects/AbstractRack.h
    UI/Effects/ParameterItem.h UI/Effects/ParameterItem.cpp
    UI/Effects/ParameterRack.h UI/Effects/ParameterRack.cpp
    UI/Effects/EffectItem.h UI/Effects/EffectItem.cpp
    UI/Effects/EffectRack.h UI/Effects/EffectRack.cpp
)

qt_add_resources(EmbeddedDSP_Host "styles"
    PREFIX "/"
    FILES
        UI/style.qss
)

target_include_directories(EmbeddedDSP_Host PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Device
    ${CMAKE_CURRENT_SOURCE_DIR}/Device/Demo
    ${CMAKE_CURRENT_SOURCE_DIR}/Processing
    ${CMAKE_CURRENT_SOURCE_DIR}/Models
    ${CMAKE_CURRENT_SOURCE_DIR}/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/Spectrum
    ${CMAKE_CURRENT_SOURCE_DIR}/UI/Effects
    ${CMAKE_CURRENT_SOURCE_DIR}/../EmbeddedDSP_Shared
)

target_link_libraries(EmbeddedDSP_Host
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
)

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
