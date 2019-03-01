cmake_minimum_required(VERSION 3.13)
project(LimeSuiteRx)

set(CMAKE_CXX_STANDARD 14)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_EXAMPLES "Enable library example programs" ON "ENABLE_LIBRARY" OFF)
add_feature_info(LimeSuiteExamples ENABLE_EXAMPLES "LimeSuite library API examples")




add_executable(singleRX main.cpp SdrDevice.cpp SdrDevice.h gnuplot-iostream.h plotter.cpp plotter.h)
set_target_properties(singleRX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_link_libraries(singleRX LimeSuite)
if (UNIX)
    target_link_libraries(singleRX -lfftw3f)
    target_link_libraries(singleRX -lm)
    target_link_libraries(singleRX -lboost_iostreams)
    target_link_libraries(singleRX -lboost_system)
    target_link_libraries(singleRX -lboost_filesystem)
endif()
