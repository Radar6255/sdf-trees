set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required (VERSION 3.8)
project("DimensionShooter")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB game_SRC "./**/*.h" "./**/*.cpp" "*.h" "*.cpp")
add_executable (imGUIexample ${game_SRC})

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE glad::glad)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE imgui::imgui)

find_package(glm CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE glm::glm)

# Or use the header-only version
# find_package(glm CONFIG REQUIRED)
# target_link_libraries(main PRIVATE glm::glm-header-only)

find_package(unofficial-noise CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE unofficial::noise::noise-static)

find_package(unofficial-noiseutils CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE unofficial::noiseutils::noiseutils-static)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(imGUIexample PRIVATE assimp::assimp)
