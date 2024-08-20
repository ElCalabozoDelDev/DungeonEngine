cmake_minimum_required(VERSION 3.10)

# Especifica Clang como compilador
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

# Integración con vcpkg
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/share")
# Nombre del proyecto
project(DungeonEngine VERSION 1.0 LANGUAGES CXX)

# Configuración de CMake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuración de C++ estándar
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Recursivamente busca todos los archivos .cpp dentro del directorio src/
file(GLOB_RECURSE SOURCES game/src/*.cpp)

# Crea el ejecutable en la carpeta _bin
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Crea el ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Añadir el directorio de third_party a los include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/game/third_party)

# Busca recursivamente todos los archivos .cpp en third_party y agrégalos a las fuentes del proyecto
file(GLOB_RECURSE THIRD_PARTY_SOURCES ${CMAKE_SOURCE_DIR}/game/third_party/*.cpp)

# Agrega los archivos de implementación de third_party al proyecto
target_sources(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_SOURCES})

# Definir rutas para ZLIB
find_path(ZLIB_INCLUDE_DIR zlib.h HINTS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/include")

if(NOT ZLIB_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find ZLIB include directory")
endif()
set(ZLIB_LIBRARY "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib")

# Definir rutas para PNG
find_path(PNG_PNG_INCLUDE_DIR png.h HINTS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/include/libpng16")
set(PNG_LIBRARY "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib")

if(NOT PNG_PNG_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find PNG include directory")
endif()

# Busca e incluye las dependencias desde vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2_image CONFIG REQUIRED)
find_package(sdl2_mixer CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

# Enlaza las librerías necesarias a tu proyecto
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer imgui::imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARY} ${PNG_LIBRARY})

# Configura los directorios de inclusión
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/game/include
)

# Copia el archivo SDL2.dll y otras DLL necesarias a la carpeta _bin
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
)
