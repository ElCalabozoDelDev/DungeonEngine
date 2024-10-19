#ifndef POSITION_COMPONENT_HPP
#define POSITION_COMPONENT_HPP

#include "Quadtree.hpp"

struct TransformComponent {
    quadtree::Box<float> box;
};

#endif // POSITION_HPP