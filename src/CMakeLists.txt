cmake_minimum_required (VERSION 2.8.11)

add_executable (eyl-lang main.cxx)
set_property (TARGET eyl-lang PROPERTY CXX_STANDARD 14)

add_subdirectory (elf)
add_subdirectory (gui)
