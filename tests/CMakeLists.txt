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
    
        ../EmbeddedDSP_Firmware/App/Protocol/ControlParser.h
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

target_include_directories(dsp_unit_tests PRIVATE
        ../EmbeddedDSP_Firmware/App
        ../EmbeddedDSP_Shared
)

target_link_libraries(dsp_unit_tests
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(dsp_unit_tests)
