#ifndef DE_COMPONENTS_SPRITE_COMPONENT_HPP
#define DE_COMPONENTS_SPRITE_COMPONENT_HPP

namespace de
{
struct SpriteComponent
{
    int spriteRow = 0;     // Sprite row in the sheet
    int currentCol = 0;    // Sprite column in the sheet
    int currentSprite = 0; // Current animation sprite
};

} // namespace de

#endif // DE_COMPONENTS_SPRITE_COMPONENT_HPP
