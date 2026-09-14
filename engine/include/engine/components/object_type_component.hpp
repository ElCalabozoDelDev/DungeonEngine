#ifndef DE_COMPONENTS_OBJECT_TYPE_COMPONENT_HPP
#define DE_COMPONENTS_OBJECT_TYPE_COMPONENT_HPP

#include <string>

namespace de
{
/// The `type` attribute of a Tiled object, carried verbatim from the .tmj.
///
/// The loader does not interpret it: a game decides what "Player" or "Enemy"
/// mean and tags the entity with its own components after the level loads.
/// This is what keeps the engine free of gameplay component types.
struct ObjectTypeComponent
{
    std::string type;
};

} // namespace de

#endif // DE_COMPONENTS_OBJECT_TYPE_COMPONENT_HPP
