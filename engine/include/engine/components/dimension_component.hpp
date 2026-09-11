#ifndef DE_COMPONENTS_DIMENSION_COMPONENT_HPP
#define DE_COMPONENTS_DIMENSION_COMPONENT_HPP

namespace de
{
/// Size in world units.
///
/// Floats, not ints: the camera's size comes from Config as a float and was
/// being truncated on the way in, and the spatial index needs floats anyway.
struct DimensionComponent
{
    float width = 0.0f;
    float height = 0.0f;
};

} // namespace de

#endif // DE_COMPONENTS_DIMENSION_COMPONENT_HPP
