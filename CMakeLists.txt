# CMakeList.txt : CMake project for gbemu, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (gbemu)

# Add source to this project's executable.
find_package(SDL2 CONFIG REQUIRED)
set (PROJECT_SRC "main.cpp" "gb.cpp" "core/core.cpp" "core/mmu.cpp" "core/timer.cpp" "core/ppu.cpp")
add_executable (gbemu ${PROJECT_SRC})
target_link_libraries(gbemu PRIVATE SDL2::SDL2 SDL2::SDL2main)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gbemu PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
