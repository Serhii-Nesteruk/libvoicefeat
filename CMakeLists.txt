cmake_minimum_required(VERSION 3.16)
project(libvoicefeat VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(THIRD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

set(libvoicefeat_SOURCES
        # Audio
        ${SRC_DIR}/audio/wav_audio_reader.cpp
        ${SRC_DIR}/audio/mp3_audio_reader.cpp
        ${SRC_DIR}/minimp3_single.cpp

        # DSP
        ${SRC_DIR}/dsp/frame_exctractor.cpp
        ${SRC_DIR}/dsp/window_function.cpp
        ${SRC_DIR}/dsp/fft_transformer.cpp
        ${SRC_DIR}/dsp/dft_transformer.cpp

        # Features
        ${SRC_DIR}/features/mfcc.cpp
        ${SRC_DIR}/features/delta.cpp

        # Facade
        ${SRC_DIR}/libvoicefeat.cpp
)

add_library(libvoicefeat ${libvoicefeat_SOURCES})
add_library(libvoicefeat::libvoicefeat ALIAS libvoicefeat)

target_include_directories(libvoicefeat
        PUBLIC
        $<BUILD_INTERFACE:${INC_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${THIRD_DIR}/minimp3)

target_compile_features(libvoicefeat PUBLIC cxx_std_17)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libvoicefeat PRIVATE -Wall -Wextra -pedantic)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS libvoicefeat
        EXPORT libvoicefeatTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${INC_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${THIRD_DIR}/minimp3/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/minimp3)

install(EXPORT libvoicefeatTargets
        FILE libvoicefeatTargets.cmake
        NAMESPACE libvoicefeat::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libvoicefeat)

configure_package_config_file(
        cmake/libvoicefeatConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/libvoicefeatConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libvoicefeat)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/libvoicefeatConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/libvoicefeatConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/libvoicefeatConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libvoicefeat)

option(LIBVOICEFEAT_BUILD_EXAMPLES "Build libvoicefeat example executables" ON)
if (LIBVOICEFEAT_BUILD_EXAMPLES)
    add_executable(libvoicefeat_demo main.cpp)
    target_link_libraries(libvoicefeat_demo PRIVATE libvoicefeat::libvoicefeat)
    set_target_properties(libvoicefeat_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

option(LIBVOICEFEAT_BUILD_TESTS "Build LIBVOICEFEAT tests" ON)
if (LIBVOICEFEAT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()