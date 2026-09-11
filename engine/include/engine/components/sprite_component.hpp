#ifndef DE_COMPONENTS_SPRITE_COMPONENT_HPP
#define DE_COMPONENTS_SPRITE_COMPONENT_HPP

namespace de
{
struct SpriteComponent
{
    int spriteRow;     // Sprite row in the sheet
    int currentCol;    // Sprite column in the sheet
    int currentSprite; // Current animation sprite
};

} // namespace de

#endif // DE_COMPONENTS_SPRITE_COMPONENT_HPP
