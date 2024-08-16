cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(DungeonEngine VERSION 1.0 LANGUAGES CXX)

# Configuración de C++ estándar
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Añade la ruta donde se encuentra SDL2
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2-2.30.6/x86_64-w64-mingw32/lib/cmake/SDL2")

# Busca SDL2 en el sistema
find_package(SDL2 REQUIRED)

# Recursivamente busca todos los archivos .cpp dentro del directorio src/
file(GLOB_RECURSE SOURCES game/src/*.cpp)

# Crea el ejecutable en la carpeta _bin
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Crea el ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Enlaza SDL2 a tu proyecto
target_link_libraries(${PROJECT_NAME} SDL2::SDL2)

# Configura los directorios de inclusión, incluyendo SDL2 y tu proyecto
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/game/include
    ${CMAKE_SOURCE_DIR}/entt
)

# Copia el archivo SDL2.dll a la carpeta _bin
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/SDL2-2.30.6/x86_64-w64-mingw32/bin/SDL2.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)