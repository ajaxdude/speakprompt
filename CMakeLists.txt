cmake_minimum_required(VERSION 3.16)
project(SpeakPrompt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Try GTK3 first, fallback to Qt5 or no GUI
set(GUI_BACKEND "")
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GTK3 gtk+-3.0)
    if(GTK3_FOUND)
        set(GUI_BACKEND "GTK3")
        message(STATUS "Using GTK3 for GUI")
    else()
        pkg_check_modules(QT5 Qt5Widgets)
        if(QT5_FOUND)
            set(GUI_BACKEND "QT5")
            message(STATUS "Using Qt5 for GUI")
        endif()
    endif()
    
    pkg_check_modules(PULSE libpulse)
    if(NOT PULSE_FOUND)
        message(WARNING "PulseAudio not found, audio capture may not work")
    endif()
endif()

if(NOT GUI_BACKEND)
    message(STATUS "No GUI framework found, building console version")
    add_definitions(-DNO_GUI)
endif()

# Whisper.cpp
set(WHISPER_CPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp)
add_subdirectory(${WHISPER_CPP_ROOT} whisper-cpp-build)

# Include directories
include_directories(${WHISPER_CPP_ROOT})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/main_simple.cpp
    src/audio_capture.cpp
    src/transcription_engine.cpp
    src/terminal_output.cpp
)

# Headers
set(HEADERS
    src/audio_capture.h
    src/transcription_engine.h
    src/terminal_output.h
)

# Add GUI files only if GUI backend is available
if(GUI_BACKEND STREQUAL "GTK3")
    list(APPEND SOURCES src/gui.cpp)
    list(APPEND HEADERS src/gui.h)
elseif(GUI_BACKEND STREQUAL "QT5")
    list(APPEND SOURCES src/gui_qt.cpp)
    list(APPEND HEADERS src/gui_qt.h)
endif()

# Create executable
add_executable(speakprompt ${SOURCES} ${HEADERS})

# Link libraries - threads will be added later if needed for PulseAudio
set(LINK_LIBS whisper)

if(GUI_BACKEND STREQUAL "GTK3")
    list(APPEND LINK_LIBS ${GTK3_LIBRARIES})
    target_include_directories(speakprompt PRIVATE ${GTK3_INCLUDE_DIRS})
    target_compile_options(speakprompt PRIVATE ${GTK3_CFLAGS_OTHER})
elseif(GUI_BACKEND STREQUAL "QT5")
    list(APPEND LINK_LIBS Qt5::Widgets)
endif()

if(PULSE_FOUND)
    # Add pthread explicitly as PulseAudio requires it
    find_package(Threads REQUIRED)
    
    # Use both libpulse-simple and libpulse for the simple API functions and error handling
    list(APPEND LINK_LIBS pulse-simple pulse Threads::Threads)
    
    target_include_directories(speakprompt PRIVATE ${PULSE_INCLUDE_DIRS})
    target_compile_options(speakprompt PRIVATE ${PULSE_CFLAGS_OTHER})
    target_compile_definitions(speakprompt PRIVATE HAVE_PULSE)
    
    message(STATUS "Using PulseAudio simple API library: pulse-simple")
    message(STATUS "PulseAudio include dirs: ${PULSE_INCLUDE_DIRS}")
    message(STATUS "PulseAudio cflags: ${PULSE_CFLAGS_OTHER}")
else()
    message(WARNING "PulseAudio development libraries not found. Audio capture will be disabled.")
endif()

# No X11 dependency needed for simplified version

target_link_libraries(speakprompt PRIVATE ${LINK_LIBS})

# Compiler flags
target_compile_options(speakprompt PRIVATE -Wall -Wextra)

# Installation
install(TARGETS speakprompt DESTINATION bin)

# AppImage creation
include(GNUInstallDirs)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/speakprompt.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
