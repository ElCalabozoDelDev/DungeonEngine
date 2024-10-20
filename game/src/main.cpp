#define SDL_MAIN_HANDLED

#include "core/game_loop.hpp"
#include "plugins/base_plugin.hpp"
#include "plugins/game_plugin.hpp"
#include <box2d/box2d.h>

void crear_mundo() {
    b2WorldDef worldDef = b2DefaultWorldDef();

    // Creación de cuerpos, fixtures, y más.
}

int main(int argc, char* argv[]) {
	crear_mundo();
	GameLoop gameLoop;
	BasePlugin basePlugin("../assets/game.xml");
	GamePlugin gamePlugin;
	gameLoop.addPlugin(basePlugin);
	gameLoop.addPlugin(gamePlugin);
	gameLoop.run();
	return 0;
}

// #include <iostream>
// #include <vector>
// #include <functional>
// #include "quadtree.hpp"

// using namespace quadtree;
// struct Entity {
//     int id;
//     Box<float> collisionBox;

//     Entity(int id, const Box<float>& box) : id(id), collisionBox(box) {}

//     bool operator==(const Entity& other) const {
//         return id == other.id;
//     }
// };

// int main() {
//     // Definir el área del Quadtree
//     Box<float> quadtreeArea(0.0f, 0.0f, 100.0f, 100.0f);

//     // Definir la función para obtener el Box de la entidad
//     auto getBox = [](const Entity& entity) -> Box<float> {
//         return entity.collisionBox;
//     };

//     // Crear el Quadtree para manejar colisiones
//     Quadtree<Entity, decltype(getBox)> quadtree(quadtreeArea, getBox);

//     // Crear y agregar entidades al Quadtree
//     Entity e1(1, Box<float>(10.0f, 10.0f, 10.0f, 10.0f));  // Entidad 1
//     Entity e2(2, Box<float>(15.0f, 15.0f, 10.0f, 10.0f));  // Entidad 2 (se intersecta con e1)
//     Entity e3(3, Box<float>(50.0f, 50.0f, 10.0f, 10.0f));  // Entidad 3 (no se intersecta)

//     quadtree.add(e1);
//     quadtree.add(e2);
//     quadtree.add(e3);

//     // Encontrar todas las intersecciones en el Quadtree
//     auto intersections = quadtree.findAllIntersections();

//     // Imprimir las intersecciones encontradas
//     for (const auto& pair : intersections) {
//         std::cout << "Colisión entre entidades " << pair.first.id
//                   << " y " << pair.second.id << std::endl;
//     }

//     return 0;
// }
