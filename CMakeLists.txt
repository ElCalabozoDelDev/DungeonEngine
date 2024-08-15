cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(DungeonEngine VERSION 1.0 LANGUAGES CXX)

# Configuración de C++ estándar
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Busca SDL2 en el sistema
find_package(SDL2 REQUIRED)

# Agrega los archivos fuente de tu proyecto
set(SOURCES
    src/main.cpp
    src/otro_archivo.cpp
    # Agrega más archivos aquí
)

# Crea el ejecutable
add_executable(MiProyectoSDL ${SOURCES})

# Enlaza SDL2 a tu proyecto
target_link_libraries(MiProyectoSDL SDL2::SDL2)

# Opcional: Configurar directorios de inclusión
target_include_directories(MiProyectoSDL PRIVATE ${SDL2_INCLUDE_DIRS})

# Si utilizas otros directorios de cabeceras
include_directories(${CMAKE_SOURCE_DIR}/game/include)
