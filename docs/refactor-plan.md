# Plan de refactorización de DungeonEngine

Revisión completa del código de `engine/`, `game/` y `tests/` (unas 9.700
líneas), más CMake, CI y tooling, y el plan que salió de ella. Se ejecutó en
ocho fases, cada una en su rama y su PR, apiladas en cadena.

**Estado:** completado. Las ocho fases se mergearon en `main` en orden, de #30
a #37, el 2026-09-14.

| Fase | Rama | PR | Tests |
|---|---|---|---|
| 0 · Red de seguridad | `refactor-tooling` | [#30](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/30) | 46 → 48 |
| 1 · Código muerto | `refactor-cleanup` | [#31](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/31) | 48 |
| 2 · Bugs latentes | `refactor-bugs` | [#32](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/32) | 61 |
| 3 · Estado único | `refactor-state` | [#33](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/33) | 66 |
| 4 · Gameplay descompuesto | `refactor-gameplay` | [#34](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/34) | 74 |
| 5 · API del engine | `refactor-engine-api` | [#35](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/35) | 80 |
| 6 · UI del juego | `refactor-ui` | [#36](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/36) | 80 |
| 7 · Agregados y CLI | `refactor-aggregates` | [#37](https://github.com/ElCalabozoDelDev/DungeonEngine/pull/37) | 85 |

## Criterio de aceptación

Punto de partida: compilaba, pasaban 46/46 tests y `--sim` era determinista.

Cada paso de cada fase se verificó con:

- `ctest` en verde;
- build limpio con `-DDE_WARNINGS_AS_ERRORS=ON` y 0 warnings (desde la fase 0);
- `clang-format --dry-run --Werror`;
- los tests `sim_golden_seed0` y `sim_golden_seed4`: dos corridas de `--sim`
  idénticas entre sí e iguales a `tests/golden/`.

**Ninguna fase regeneró las referencias del sim.** Un refactor que cambia el
CSV es un bug del refactor; un cambio de gameplay deliberado regenera la
referencia en el mismo commit (comando en la cabecera de
`tests/sim_golden.cmake`).

Los arreglos de bugs llevan test de regresión, y se comprobó deshaciendo cada
arreglo que su test falla sin él.

---

## Diagnóstico

La base era sana: la separación engine/juego se respeta, los recursos SDL
tienen dueño claro, los errores usan `std::expected` y los comentarios
explican el porqué. Los problemas se agrupaban en cinco familias.

### A. Bugs latentes

| # | Problema | Resuelto en |
|---|---|---|
| A1 | `TransformSystem`, sistema fijo, integraba con `dt.value` en vez de `dt.fixed` | Fase 2 |
| A2 | `SpatialSyncSystem` solo re-archivaba entidades con `VelocityComponent` (el murciélago y los segmentos nunca se actualizaban) y corría antes que Snake y Bat | Fase 2 |
| A3 | La cabeza del slime se dibujaba dos veces | Fase 2 |
| A4 | El inspector de la cámara se caía sin `CameraBoundsComponent`, editaba copias y `m_current_entity` no estaba inicializado | Fase 2 |
| A5 | El fundido a gris iba por frame y no por segundo (2,4× más rápido a 144 Hz) | Fase 2 |
| A6 | `CameraSystem` exigía `VelocityComponent` en el objetivo | Fase 2 |
| A7 | Pausa y Game Over compartían el slot de `use_state`: el foco pasaba de un menú al otro | Fase 2 (y fase 6 lo hace imposible) |
| A8 | `GameLoop` leía la pausa una vez por frame: tras morir seguían corriendo pasos fijos | Fase 2 |
| A9 | *Encontrado durante la fase 2.* El quadtree repartía valores por su caja actual al dividir un nodo; con varias entidades moviéndose en el mismo paso quedaban duplicados (118 entradas para 64 entidades). El arreglo de A2 lo habría expuesto | Fase 2 |

### B. Estado duplicado

- La pausa vivía en dos sitios (`PlayState::Paused` y `de::Paused`), y los sistemas comprobaban ambos.
- Recursos del contexto creados "por si acaso" con `if (!contains) emplace` en nueve sitios.
- `Rectangle` duplicaba `de::Box<float>`; `dot`, `Circle`, `reflect` y `lerp` estaban copiados en el juego.
- La creación de la serpiente estaba duplicada y las dos versiones no coincidían.

→ Resuelto en las fases 3 y 4.

### C. Duplicación y funciones grandes

- `InGameScene::onEnter` de 90 líneas; `SnakeSystem::run` de 150.
- Tres `onExit` idénticos; tres pasadas de render casi iguales; tres widgets con la misma navegación y el mismo código de botones.
- Carga de assets repetida en tres escenas; comprobaciones de `StartupError` copiadas en cada plugin.

→ Resuelto en las fases 4, 5 y 6.

### D. Código muerto

- Restos de TMX: base64, `trim.hpp`, fixtures `.tmx`, hook y skill.
- `RenderCollision`, componentes de colisión sin lector, `hasPendingScene`, `minZoom`/`maxZoom`, `use_effect`/`use_callback`.
- Sistemas registrados que no hacían nada en este juego.

→ Resuelto en la fase 1.

### E. Tooling

- Sin flags de warnings; `.clang-tidy` solo cubría `game/` y no corría en CI.
- Componentes sin inicializadores por defecto.

→ Resuelto en las fases 0 y 5.

---

## Fases

### Fase 0 · Red de seguridad — #30

- Tests `sim_golden_seed0` y `sim_golden_seed4` con referencias en `tests/golden/`. El CSV gana columnas de posición (`head_x,head_y,bat_x,bat_y`) al final; las cuatro originales coinciden byte a byte con las de antes.
- `-Wall -Wextra -Wpedantic` en los targets propios (`de_enable_warnings`); CI con `-Werror`.
- `.clang-tidy` cubre `engine/`, sin dos checks de puro estilo; paso de CI informativo (186 hallazgos distintos).
- `.gitattributes` evita que `core.autocrlf` altere las referencias.
- README corregidos: la sección `--sim` describía un sim anterior.

### Fase 1 · Código muerto — #31

- Eliminados los restos de TMX, `RenderCollision`, componentes y APIs sin uso, y `use_effect`/`use_callback`.
- `TransformSystem`, `CameraSystem` y `CollisionSystem` quedan en el engine como sistemas opcionales, no registrados por defecto.
- Documentación sin TMX, base64 ni `game.xml`.

### Fase 2 · Bugs latentes — #32

- A1 a A9, cada uno con test de regresión.
- `GameLoop::addFixedSystemLast` para la sincronización espacial.
- `SpatialIndex::updateLayer`; los árboles navegan por las cajas archivadas.
- `use_state` devuelve el valor por copia.

### Fase 3 · Estado único — #33

- `GameState::playState` es la verdad y `de::Paused` se deriva de él; solo `setPlayState()` escribe ambos.
- Los sistemas fijos no comprobaban la pausa. `BatSystem` conserva la comprobación de `playState`, que sí es necesaria: evita que una serpiente muerta en el mismo paso coma.
- Recursos compartidos instalados una vez en `GamePlugin`; el resto usa `ctx().get`.
- `engine/core/math.hpp` con `lerp`, `reflect` y `Circle`; `Vector2D::dot` y `operator==`; `Rectangle` → `Box<float>`.

### Fase 4 · Gameplay descompuesto — #34

- `game/prefabs` para cabeza, segmentos y murciélago. Los aleatorios del murciélago se extraen en orden explícito (antes dependía del compilador).
- Reglas puras `game::snake::steer` / `advance`; `SnakeViewSystem` coloca los sprites.
- `BatSystem` separado en `fly` y `feedTouchingSnakes`.
- `InGameScene::onEnter` como orquestador; `de::buildLevelSpatialIndex`; `game::loadGameAssets` una vez al arrancar.

### Fase 5 · API del engine — #35

- `Config::logicalWidth/Height` (antes `cameraWidth/Height`); `ConfigLoader` con helper.
- `Scene::track()`: `SceneSystem` destruye las entidades registradas tras `onExit()`.
- `TiledLoader::load` devuelve `LoadedLevel`, limpia si falla, no añade velocidad y usa `TiledLayerNames`.
- `GameLoop::run()` en un `.cpp`, dueño de `DeltaTime`, corta el setup tras un `StartupError`; `SceneSystem` y `DebugSystem` en `BasePlugin`.
- `TileLayerPass` y `SpriteLayerPass`; funciones de dibujo libres; `RenderSystem` dueño del pase de color.
- Test que dibuja los pases en un renderer por software y lee píxeles.

### Fase 6 · UI del juego — #36

- `WidgetComponent` con un solo `render(registry)`; sin hooks, estado en miembros.
- `game/ui/menu`: `MenuNav`, `menuButton`, `pressed`, `uiClock`; `drawMenuBackground`.
- Recursos de UI cargados una vez; fuera los cambios de modo de textura en cada dibujo.

### Fase 7 · Agregados y CLI — #37

- `Vector2D` y `Box` como agregados (`x`, `y`, …), unos 220 usos convertidos.
- `parseCommandLine` en `game_lib`, con `std::from_chars` y código de salida 2 para una línea de comandos inválida.

---

## Cambios de comportamiento a tener en cuenta

Ninguno altera la partida simulada, pero conviene conocerlos:

- **Fase 5:** `Capa de patrones 1` ya no es nombre de capa inferior por defecto (una capa única se sigue dibujando debajo). Una capa única llamada `Overlay` ya no se dibuja dos veces. `SceneSystem`/`DebugSystem` corren antes que `GrayscaleFadeSystem` en cada frame.
- **Fase 6:** la navegación del menú se aplica antes de confirmar; solo se nota con una flecha y Enter en el mismo frame.
- **Fase 7:** un argumento desconocido impide arrancar (antes era un aviso).

## Fuera de alcance / pendiente

- **Bus de eventos** (`entt::dispatcher`) para que audio y score escuchen a los sistemas: opcional en la fase 4, no hecho.
- **Backlog de clang-tidy:** 186 hallazgos; el paso de CI es informativo. Convertirlo en bloqueante cuando se limpie.
- **Esc en Options** también activa la pausa durante un frame, hasta que Title la reinicia. Sin efecto visible.
- **`System` en `shared_ptr`:** el loop podría ser dueño único con `unique_ptr` salvo los sistemas que el contexto referencia.
