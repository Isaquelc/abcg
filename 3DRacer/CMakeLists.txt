project(3DRacer)
add_executable(${PROJECT_NAME} main.cpp player.cpp camera.cpp ground.cpp enemies.cpp openglwindow.cpp)
enable_abcg(${PROJECT_NAME})