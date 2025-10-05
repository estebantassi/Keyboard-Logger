# CMakeList.txt : CMake project for CPP Keyboard App, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (CMakeTarget)

configure_file(
    ${cmake_source_dir}./src/data/layouts.json  # source file
    ${cmake_binary_dir}./data/layouts.json  # destination file
    copyonly                                # just copy, no variable replacement
)

include(FetchContent)

FetchContent_Declare(
  wxwidgets
  GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
  GIT_TAG v3.3.1
)

set(wxBUILD_SHARED OFF)
set(wxBUILD_TESTS OFF)
set(wxBUILD_SAMPLES OFF)

FetchContent_MakeAvailable(wxwidgets)

add_executable (CMakeTarget WIN32 "src/main.cpp" "src/app.cpp" "src/Frame.cpp")

target_link_libraries(CMakeTarget PRIVATE wx::core wx::base)
