cmake_minimum_required (VERSION 3.8)
project("DimensionShooter")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable (imGUIexample "main.cpp" "UseImGui.cpp" "UseImGui.h" "./engine/ShaderCode.h" "./engine/ShaderCode.cpp" "./engine/Program.cpp" "./engine/Program.h")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE glad::glad)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE imgui::imgui)
