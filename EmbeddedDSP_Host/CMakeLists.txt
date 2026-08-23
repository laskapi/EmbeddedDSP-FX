cmake_minimum_required(VERSION 3.19)
project(EmbeddedDSP_Host LANGUAGES CXX)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required Qt6 packages (using core Widgets & SerialPort)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    SerialPort
)

qt_standard_project_setup()

qt_add_executable(EmbeddedDSP_Host
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    SerialManager.h
    SerialManager.cpp
    FftProcessor.h FftProcessor.cpp
    AudioFrameSimulator.h AudioFrameSimulator.cpp
    SpectrumWidget.h SpectrumWidget.cpp
    ConnectionToolbar.h ConnectionToolbar.cpp
)

# Include directories: local source and firmware protocols
target_include_directories(EmbeddedDSP_Host PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../EmbeddedDSP_Firmware/App
)

# Link Qt6 libraries
target_link_libraries(EmbeddedDSP_Host
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
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