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

`--sim` corre el juego sin ventana con una política de navegación determinista y
vuelca un CSV de una fila por paso fijo. Un cambio de balance se evalúa
comparando el resumen contra la referencia, no a ojo.

Referencia medida sobre `dungeon1` con los valores de esta tabla:

```
sim: outcome=unreachable
sim: steps=164 sim_seconds=2.7333
sim: items=2/3 unreachable=1 pickups_at=0.4000,2.7167
sim: damage_taken=2 hits=2 final_health=3 first_hit_at=0.7833
sim: distance_px=529.554 mean_speed=193.739 blocked_ratio=0.0122
sim: chase_ratio=0.9024 min_enemy_dist=3.722
```

Cómo leerla:

| Métrica | Qué dice |
|---|---|
| `items=2/3` + `unreachable=1` | El tercer ítem **no se puede recoger desde ninguna posición**: ver [[NIVEL-DUNGEON1]]. El nivel no se puede completar. |
| `damage_taken=2` de 5 corazones | Un recorrido perfecto pero sin esquivar cuesta 2 de 5. Queda margen: el nivel no es letal ni de lejos. |
| `chase_ratio=0.90` | Se pasa el 90 % del recorrido con algún enemigo persiguiendo. Con `chaseRange` a 120 px, los enemigos están casi siempre activos en esta ruta. |
| `mean_speed=193.7` de 200 | Apenas roza geometría (`blocked_ratio` 1,2 %): el nivel no estorba al movimiento. |

**Estas cifras son de una sonda que no esquiva.** Un jugador humano competente
recibiría menos daño; uno malo, más. Sirven como suelo comparable entre cambios,
no como predicción de una partida real.

**Advertencia sobre lo que mide `--sim`:** la política navega perfecto y no
esquiva nada — atraviesa a los enemigos y se come los golpes. No es un jugador,
es una **sonda repetible**. Sus números responden "cuánto castiga este nivel a
alguien mecánicamente perfecto y tácticamente ciego", que es una cota, no una
experiencia. El *feel* no se mide así; para eso hay que jugarlo.
