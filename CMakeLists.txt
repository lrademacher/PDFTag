# Set the name and the supported language of the project
PROJECT(PdfTag CXX)
# Set the minimum version of cmake required to build this project
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Use the package PkgConfig to detect GTK+ headers/library files
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

SET(BIN_NAME PdfTag)

# Add an executable compiled from sources in src directory
file(GLOB_RECURSE SOURCES "src/*.cc")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/res/PdfTag_GtkGui.glade ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(${BIN_NAME} ${SOURCES})

target_compile_options(${BIN_NAME} PRIVATE -Wall -Wextra -pedantic -std=gnu++17)

# set(CMAKE_VERBOSE_MAKEFILE ON)

# Link the target to the GTK+ libraries
TARGET_LINK_LIBRARIES(${BIN_NAME} ${GTK3_LIBRARIES})