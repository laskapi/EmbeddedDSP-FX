include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(dsp_unit_tests
        test_protocol.cpp
        test_dsp.cpp
    
        ../EmbeddedDSP_Firmware/App/Comm/ControlParser.h
        ../EmbeddedDSP_Shared/Protocol/SpscQueue.h
        ../EmbeddedDSP_Firmware/App/DSP/OverdriveEffect.h
        ../EmbeddedDSP_Firmware/App/DSP/DelayEffect.h
     )

set_target_properties(dsp_unit_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME "dsp_unit_tests"
        SUFFIX ".exe"
)

target_compile_definitions(dsp_unit_tests PRIVATE FIRMWARE_UNIT_TESTS)

target_include_directories(dsp_unit_tests PRIVATE
        ../EmbeddedDSP_Firmware/App
        ../EmbeddedDSP_Shared
)

target_link_libraries(dsp_unit_tests
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(dsp_unit_tests)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Test SerialPort)

add_executable(host_unit_tests
        host/test_manifestparser.cpp
        host/test_appcontroller.cpp
        host/test_fftprocessor.cpp
        ../EmbeddedDSP_Host/Models/DeviceManifestParser.cpp
        ../EmbeddedDSP_Host/Core/AppController.cpp
        ../EmbeddedDSP_Host/Core/WorkerThread.cpp
        ../EmbeddedDSP_Host/Processing/AudioFrameWorker.cpp
        ../EmbeddedDSP_Host/Processing/FftProcessor.cpp
        ../EmbeddedDSP_Host/Device/DeviceSession.cpp
        ../EmbeddedDSP_Host/Device/IDevice.h
        ../EmbeddedDSP_Host/Device/IConnectionScanner.h
)

set_target_properties(host_unit_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME "host_unit_tests"
        SUFFIX ".exe"
)

target_include_directories(host_unit_tests PRIVATE
        ../EmbeddedDSP_Host
        ../EmbeddedDSP_Shared
)

target_link_libraries(host_unit_tests PRIVATE
        GTest::gtest_main
        GTest::gmock
        Qt6::Core
        Qt6::Test
        Qt6::SerialPort
)

gtest_discover_tests(host_unit_tests)
