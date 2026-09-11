#ifndef DE_COMPONENTS_ANIMATION_COMPONENT_HPP
#define DE_COMPONENTS_ANIMATION_COMPONENT_HPP

namespace de
{
struct AnimationComponent
{
    int currentFrame;         // Current animation frame
    int totalFrames;          // Frames in the row
    float animationTime;      // Seconds between frames
    float timeSinceLastFrame; // Time since the last frame change
};

} // namespace de

#endif // DE_COMPONENTS_ANIMATION_COMPONENT_HPP
