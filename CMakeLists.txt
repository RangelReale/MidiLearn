cmake_minimum_required(VERSION 3.24)

project(midilearn CXX)

# TSE3 0.3.1 still uses dynamic exception specifications; wxWidgets inherits this setting.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not cosmetic: the Linux arm of the platform #ifdef in ctl_miditrack.h guards on the bare
# `unix` macro, which GCC only defines under -std=gnu++NN.
set(CMAKE_CXX_EXTENSIONS ON)

include(FetchContent)

#
# wxWidgets
#

set(wxBUILD_SHARED     OFF CACHE BOOL   "" FORCE)   # static, as the pre-FetchContent build was
set(wxBUILD_MONOLITHIC OFF CACHE BOOL   "" FORCE)   # required for wx::core / wx::base to exist
set(wxBUILD_INSTALL    OFF CACHE BOOL   "" FORCE)   # defaults ON: keeps wx install rules out of this project
set(wxBUILD_SAMPLES    OFF CACHE STRING "" FORCE)
set(wxBUILD_TESTS      OFF CACHE STRING "" FORCE)
set(wxBUILD_DEMOS      OFF CACHE BOOL   "" FORCE)
set(wxBUILD_BENCHMARKS OFF CACHE BOOL   "" FORCE)

FetchContent_Declare(wxWidgets
  URL      https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.11/wxWidgets-3.2.11.tar.bz2
  URL_HASH SHA256=6a129015bce2e914e4bf61ec4411854ad962801d47e92f2eb8340adb6a90af08
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

#
# TSE3
#
# This must be the patched fork: stock TSE3 0.3.1 has no MidiFilter::setTransposeIgnoreChannel
# (ctl_miditrack.cpp:1210), its TransportCallback takes a MidiCommand rather than a MidiEvent
# (ctl_miditrack.cpp:955-983), and its Win32 timer callback does not build as x64.
# SOURCE_SUBDIR builds only the library, skipping the fork's examples/ and tse3play/
# subdirectories, which have no disable option of their own.
#

FetchContent_Declare(tse3
  GIT_REPOSITORY https://github.com/RangelReale/tse3.git
  GIT_TAG        34115812d190a10c334d1687abfa8b4d997936f6
  SOURCE_SUBDIR  src/tse3
)

FetchContent_MakeAvailable(wxWidgets tse3)

# TSE3's own CMake puts nothing on its target: its include directory comes from a directory-scoped
# include_directories() at its top level, and it never links the platform MIDI library. Supply both
# here, as cmake/modules/FindTSE3.cmake used to.
target_include_directories(tse3 PUBLIC ${tse3_SOURCE_DIR}/src)
if(WIN32)
  target_link_libraries(tse3 PUBLIC winmm)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(tse3 PUBLIC asound)
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
