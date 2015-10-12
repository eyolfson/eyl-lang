cmake_minimum_required (VERSION 2.8.11)

find_package(PkgConfig)

pkg_check_modules(WAYLAND_CLIENT wayland-client)
pkg_check_modules(CAIRO cairo)

include_directories(
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
)


add_executable (eyl-lang-gui main.cxx xdg_shell.cxx)
set_property (TARGET eyl-lang-gui PROPERTY CXX_STANDARD 14)
target_link_libraries(eyl-lang-gui
    ${WAYLAND_CLIENT_LIBRARIES}
    ${CAIRO_LIBRARIES}
)
