# DungeonEngine

[![CI](https://github.com/ElCalabozoDelDev/DungeonEngine/actions/workflows/ci.yml/badge.svg)](https://github.com/ElCalabozoDelDev/DungeonEngine/actions/workflows/ci.yml)

Boilerplate de C++23 + SDL2 para juegos 2D, con un pequeño dungeon crawler como
ejemplo trabajado.

*[English version](README.md)*

La separación tiene un objetivo concreto: `engine/` no sabe nada del juego.
Borra `game/`, escribe el tuyo contra `engine`, y tienes un punto de partida en
lugar del proyecto de otro que hay que desenredar.

---

## Qué incluye

- **ECS** sobre [EnTT](https://github.com/skypjack/entt), con un sistema de
  plugins y hooks para componer el arranque, el frame y el cierre.
- **Bucle de paso fijo** — el movimiento y las colisiones avanzan en pasos
  fijos y no dependen de los FPS; el render sigue al frame.
- **Carga de Tiled (`.tmx`)**: capas de tiles, tilesets y capas de objetos.
- **Cámara** con seguimiento, predicción y tope a los límites del nivel, sobre
  una única transformación mundo→pantalla.
- **Índice espacial** (quadtree) por capa, correcto también para entidades que
  se mueven.
- **Input** como acciones con nombre y teclas rebindeables, no scancodes
  metidos en los sistemas.
- **Dear ImGui** integrado, inspector de entidades y una capa de widgets basada
  en hooks (`use_state` / `use_callback` / `use_effect`).
- **Animación de sprites** con modos en bucle, ping-pong y de una sola pasada,
  multiplicador de velocidad y pausa en cada extremo.
- **Audio** con SDL2_mixer.
- **Escenas** con cambio diferido, y pausa gestionada por el bucle.
- **46 tests** con doctest, ejecutables sin ventana, y un workflow de GitHub
  Actions que compila y testea un checkout limpio en Windows.

## Requisitos

- Git
- CMake 3.26 o superior
- Ninja
- [vcpkg](https://github.com/microsoft/vcpkg), con `VCPKG_ROOT` en el entorno
- LLVM-MinGW (UCRT, x86_64) — clang apuntando a `x86_64-w64-windows-gnu`

No interviene Visual Studio, MSVC ni el SDK de Windows. Instala el toolchain
con:

```bash
winget install --id MartinStorsjo.LLVM-MinGW.UCRT
```

`toolchain/llvm-mingw.cmake` localiza sola la instalación por defecto de
winget. Si lo tienes en otro sitio, indícalo con la variable de entorno
`LLVM_MINGW_ROOT`, o pasa
`-DLLVM_MINGW_ROOT=C:/ruta/a/llvm-mingw-...-ucrt-x86_64` al configurar.

## Compilar

```bash
git clone git@github.com:ElCalabozoDelDev/DungeonEngine.git
cd DungeonEngine
cmake --preset default -B build
ninja -C build
```

El preset `default` ya fija el directorio de build, el toolchain de vcpkg y el
triplet `x64-mingw-llvm` — ejecútalo desde la raíz del repositorio, no desde
dentro de `build/`. El primer configure compila todas las dependencias con
vcpkg y tarda unos minutos; los siguientes van en caché.

## Ejecutar

```bash
./build/bin/DungeonEngine.exe
```

Los assets se localizan relativos al ejecutable, así que funciona desde
cualquier directorio de trabajo — también haciendo doble clic en el explorador.
`build/bin` es autocontenido: el build copia junto a los binarios tanto las
dependencias de vcpkg como el runtime del propio compilador (`libc++`,
`libunwind`), así que el toolchain no necesita estar en el `PATH`. Copia ese
directorio a otro sitio y sigue funcionando; saca el `.exe` solo y no.

Llega a las tres monedas sin que te toquen los esqueletos.

| Tecla | |
|---|---|
| Flechas / WASD | moverse |
| Enter / Espacio | confirmar (menú) |
| Esc | pausar, o salir desde el menú |
| F5 | recargar el nivel |

Hay dos flags para CI y depuración: `--frames N` sale tras N frames, y
`--level` arranca directamente en el nivel en lugar del menú.

## Tests

```bash
ctest --test-dir build --output-on-failure
```

Los tests unitarios enlazan el motor y la librería del juego directamente, así
que no necesitan ventana, ni input, ni sesión de escritorio — que es lo que los
hace utilizables en CI y la razón para recurrir a ellos antes que a intentar
pilotar el juego compilado. Dos smoke tests ejecutan el binario real con los
drivers dummy de SDL para cubrir arranque, carga de nivel, render y cierre de
punta a punta.

## Documentos de diseño

`docs/design/` contiene el diseño del juego — pilares, core loop, sistemas,
biomas, niveles, tablas de balance y decisiones. Está escrito en español y habla
del *juego*, no del motor; este README sigue siendo la referencia técnica.

Cada documento declara un `estado` (`implementado`, `parcial`, `propuesto`,
`descartado`) y lista el código que lo implementa, de modo que la
desincronización entre ambos se detecta en vez de descubrirse tarde. En
`.claude/` están las skills, agentes, comandos y hooks que los mantienen al día.

---

## Arquitectura

```
        third_party            vendorizado: backends SDL2 de imgui, base64
             |                 cabeceras expuestas como includes SYSTEM
             v
          engine               librería estática, namespace de
             |                 bucle, plugins, SDL, input, audio, texturas,
             |                 carga TMX, render, cámara, quadtree,
             v                 escenas, widgets
         game_lib              este juego: componentes, sistemas, escenas, HUD
             |
             v
           game                main()
```

**`engine` no tiene `game/include` en su include path.** Un
`#include <game/...>` desde código del motor es un error de compilación, no una
convención que alguien deba recordar. Cuando el motor necesita saber algo del
juego, se invierte: el cargador TMX guarda el `type` de Tiled en
`de::ObjectTypeComponent`, y es `InGameScene::tagObjectsByType` quien convierte
`"Player"` en un `PlayerComponent`.

Regla práctica para código nuevo: **si nombra un componente de gameplay, va en
`game/`.**

### Por dónde pasa el frame

```
frame begin   input (eventos SDL -> InputState)  ->  ImGui::NewFrame
pasos fijos   movimiento -> integración -> sync espacial -> colisión -> combate
              (cero o más veces, cada uno avanza DeltaTime::fixed)
frame         animación, cámara, escenas, debug
last          passes de render por `order`, luego la GUI
frame end     present
```

Los sistemas que no deben depender de los FPS van en `addFixedSystem`. El resto
en `addSystem`, y el render en `addSystemLast`.

---

## Recetas

### Añadir un componente

Los componentes son datos planos. Los del motor van en
`engine/include/engine/components/`, los de gameplay en
`game/include/game/components/`.

```cpp
// game/include/game/components/mana_component.hpp
#ifndef GAME_COMPONENTS_MANA_COMPONENT_HPP
#define GAME_COMPONENTS_MANA_COMPONENT_HPP

struct ManaComponent
{
    int current = 10;
    int max = 10;
};

#endif // GAME_COMPONENTS_MANA_COMPONENT_HPP
```

Añádelo donde se construye la entidad — para objetos que vienen de un `.tmx`,
eso es `InGameScene::tagObjectsByType`.

### Añadir un sistema

```cpp
// game/include/game/systems/mana_regen_system.hpp
#include <engine/systems/system.hpp>

class ManaRegenSystem final : public de::System
{
public:
    void run(entt::registry& registry) override;
};
```

```cpp
// game/src/systems/mana_regen_system.cpp
void ManaRegenSystem::run(entt::registry& registry)
{
    const float dt = registry.ctx().get<de::DeltaTime>().fixed;
    for (auto&& [entity, mana] : registry.view<ManaComponent>().each())
    {
        // ...
    }
}
```

Regístralo en `GamePlugin::mount` y añade el `.cpp` a `game/CMakeLists.txt` —
las listas de fuentes son explícitas, no globs.

```cpp
gameLoop.addFixedSystem(std::make_shared<ManaRegenSystem>());
```

### Añadir una escena

Implementa `de::Scene`: `onEnter` construye las entidades, `onExit` las
destruye.

```cpp
class ShopScene : public de::Scene
{
public:
    void onEnter(entt::registry& registry) override;
    void onUpdate(entt::registry& registry) override;
    void onExit(entt::registry& registry) override;

private:
    std::vector<entt::entity> m_entities;
};
```

Dos cosas que `onExit` debe hacer, ambas aprendidas por las malas:

```cpp
void ShopScene::onExit(entt::registry& registry)
{
    // Los árboles espaciales guardan handles de entidades. Límpialos primero,
    // o la siguiente consulta devolverá entidades destruidas.
    registry.ctx().get<de::SpatialIndex>().clear();

    for (auto entity : m_entities)
    {
        // Comprobado: el gameplay puede haber destruido alguna ya.
        if (registry.valid(entity))
        {
            registry.destroy(entity);
        }
    }
    m_entities.clear();
}
```

Cambia a ella con `requestScene`, que se aplica entre frames:

```cpp
registry.ctx().get<de::SceneSystem>().requestScene(
    std::make_unique<ShopScene>());
```

Usa `setScene` solo desde un callback de setup, donde no hay frame en curso.

---

## Estructura

```
engine/          motor 2D reutilizable, librería estática (namespace de)
  include/engine/  cabeceras públicas
  src/             implementación
game/            el juego de ejemplo
  include/game/    cabeceras
  src/             implementación, más main.cpp
tests/           tests unitarios con doctest y fixtures .tmx
third_party/     código vendorizado: backends SDL2 de imgui, base64
assets/          texturas, audio, niveles Tiled (.tmx) y game.xml
toolchain/       fichero de toolchain de LLVM-MinGW y el triplet de vcpkg
```

## Configuración

`assets/game.xml` se lee al arrancar. `<Screen>` es la ventana; `<Camera
width/height>` es la resolución lógica en la que dibuja el juego, que SDL
escala a la ventana, y `zoomLevel` es cuántos píxeles lógicos ocupa una unidad
de mundo. `vsync` es opcional y por defecto está activado.

Si el fichero falta o está mal formado, el juego sale con un mensaje que nombra
el problema en lugar de reventar.

## Problemas frecuentes

**`vcpkg was unable to detect the active compiler's information`** — vcpkg sanea
el entorno de los subprocesos que lanza, así que `LLVM_MINGW_ROOT` solo le llega
porque el triplet la lista en `VCPKG_ENV_PASSTHROUGH`. Si escribes tu propio
triplet, consérvala.

**Código de salida `0xc0000135` al ejecutar un binario** — falta una DLL. El
build deja todo lo necesario en `build/bin`; esto significa que el ejecutable se
sacó solo de ese directorio.

## Dependencias

Resueltas con vcpkg (ver `vcpkg.json`): SDL2, SDL2_image, SDL2_mixer, EnTT,
Dear ImGui, tinyxml2, zlib, libpng, doctest.

Los efectos de sonido de `assets/Audio/` son placeholders sintetizados, no
grabaciones — ver `assets/Audio/README.txt`.

## Licencia

Ver [LICENSE](LICENSE).
