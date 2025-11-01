cmake_minimum_required(VERSION 3.31)
project(untitled2)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled2 main.cpp
        include/audio_buffer.h
        include/frame.h
        src/frame_exctractor.cpp
        include/window_functiion.h
        src/window_function.cpp
        include/audio_reader.h
        include/wav_audio_reader.h
        src/wav_audio_reader.cpp
        include/mp3_audio_reader.h
        src/mp3_audio_reader.cpp
        src/minimp3_single.cpp)

target_include_directories(untitled2 PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/minimp3
)
