---
id: BAL-01
titulo: Tablas de balance
estado: implementado
pilares: [PILAR-01]
codigo:
  - game/include/game/components/enemy_component.hpp
  - game/include/game/components/health_component.hpp
  - game/include/game/components/item_component.hpp
  - game/include/game/components/speed_component.hpp
  - game/src/systems/combat_system.cpp
  - tests/test_balance.cpp
---

# Tablas de balance

## Dónde viven estos números, de verdad

En los inicializadores por defecto de los componentes, en C++. **No hay archivo
de configuración de balance, y el `.tmx` no puede sobreescribirlos**:
`InGameScene::tagObjectsByType` construye los componentes por defecto y no lee
ninguna propiedad del objeto. Cambiar un número es editar un `.hpp` y recompilar.

Esta tabla, por tanto, es una **copia** — y una copia se desincroniza. Lo que la
mantiene honesta es `tests/test_balance.cpp`, que fija cada valor y nombra este
documento. Si cambias un número y no actualizas los dos sitios, el test se pone
rojo.

Si esto te parece torpe, lo es: ver [[ADR-0001]].

## Jugador

| Qué | Valor | Dónde | Notas |
|---|---|---|---|
| Velocidad | **200 px/s** | `SpeedComponent::value` | 12,5 tiles/s. Igual en las 8 direcciones (la diagonal se normaliza). |
| Vida inicial | **5** | `HealthComponent::current` | |
| Vida máxima | **5** | `HealthComponent::max` | Un corazón `[#]` por punto en el HUD. |
| Invulnerabilidad tras golpe | **1,0 s** | `InvulnerabilitySeconds`, `combat_system.cpp:19` | La única constante literal de todo el gameplay. |
| Caja de colisión | **16 × 16 px** | el sprite completo | Sin margen de gracia. |

## Enemigo

| Qué | Valor | Dónde | Notas |
|---|---|---|---|
| Rango de persecución | **120 px** | `EnemyComponent::chaseRange` | 7,5 tiles. |
| Velocidad | **60 px/s** | `EnemyComponent::speed` | |
| Daño por contacto | **1** | `EnemyComponent::contactDamage` | |

Los tres enemigos del nivel usan estos mismos valores: no existen tipos.

## Ítem

| Qué | Valor | Dónde | Notas |
|---|---|---|---|
| Valor | **1** | `ItemComponent::value` | Nunca es distinto de 1 en la práctica. |

## Las proporciones, que es lo que importa

Los valores sueltos dicen poco. Las relaciones son el diseño:

| Relación | Valor | Qué significa |
|---|---|---|
| Velocidad jugador : enemigo | **3,33 : 1** | **Esto es [[PILAR-01]].** En línea recta no te alcanzan nunca; solo mueren contigo si te encierras. Tocar este ratio cambia el juego, no el balance. |
| Rango de persecución : pantalla | **120 px ≈ ancho visible** | Con cámara ×3 sobre 800 px se ven ~267 px de mundo. Un enemigo se activa aproximadamente cuando entra en cuadro: te persigue lo que ves. |
| Distancia de escape | **1,0 s × 140 px/s = 140 px** | Durante la invulnerabilidad ganas 140 px sobre el enemigo: **más que su rango de persecución**. Un golpe recibido es, de hecho, una salida gratis. Esto suaviza mucho el juego y probablemente no fue calculado. |
| Vida : daño | **5 golpes** | Sin curación en todo el juego: cinco errores por partida, y no se recuperan. |

## Cómo medir en vez de opinar

Hoy no hay forma de medir esto: el balance se ajusta a ojo, recompilando. Lo que
sí se puede calcular sin jugar es la geometría del nivel, decodificando el `.tmx`
y corriendo un BFS ([[NIVEL-DUNGEON1]]).

Con los valores de esta tabla, el recorrido óptimo de los tres ítems de
`dungeon1` sería de **53 tiles = 848 px = 254 pasos fijos** (4,24 s) — salvo que
el tercero, como se explica en ese documento, no se puede recoger.

Falta una herramienta que ejecute el juego sin ventana y vuelque métricas por
paso: sin ella, cualquier afirmación sobre si el juego es duro o fácil es una
opinión. Es la carencia más seria de este documento.
