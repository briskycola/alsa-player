cmake_minimum_required(VERSION 3.23)

# Project Name
project(audio-player)

# Use C++ 14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Target
add_executable(audio-player main.cpp)
target_include_directories(audio-player PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# libsndfile
find_package(SndFile QUIET)
if (TARGET SndFile::sndfile)
    target_link_libraries(audio-player PRIVATE SndFile::sndfile)
elseif (SndFile_FOUND)
    target_include_directories(audio-player PRIVATE ${SNDFILE_INCLUDE_DIRS})
    target_link_libraries(audio-player PRIVATE ${SNDFILE_LIBRARIES})
else()
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)
        target_link_libraries(audio-player PRIVATE PkgConfig::SNDFILE)
    else()
        find_library(SNDFILE_LIB sndfile REQUIRED)
        target_link_libraries(audio-player PRIVATE ${SNDFILE_LIB})
    endif()
endif()

# Determine if user is running Linux or macOS
if (APPLE)
    target_sources(audio-player PRIVATE CoreAudioPlayer.cpp)

    # Link Apple frameworks
    find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox REQUIRED)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)

    target_link_libraries(audio-player PRIVATE ${AUDIOTOOLBOX_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})

elseif(LINUX)
    target_sources(audio-player PRIVATE ALSAPlayer.cpp)

    find_package(ALSA REQUIRED)
    if (TARGET ALSA::ALSA)
        target_link_libraries(audio-player PRIVATE ALSA::ALSA)
    else()
        target_include_directories(audio-player PRIVATE ${ALSA_INCLUDE_DIRS})
        target_link_libraries(audio-player PRIVATE ${ALSA_LIBRARIES})
    endif()

else()
    message(FATAL_ERROR "Unsupported platform: only Linux and macOS are supported")
endif()
