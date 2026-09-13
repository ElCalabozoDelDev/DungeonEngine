#ifndef DE_COMPONENTS_CAMERA_COMPONENT_HPP
#define DE_COMPONENTS_CAMERA_COMPONENT_HPP

namespace de
{
/// How the camera follows its target.
///
/// It no longer carries a viewport offset. Those existed to nudge the drawn
/// image because the drawing formula and the culling rectangle disagreed about
/// where the camera was; both now come from Camera2D, so there is nothing left
/// to compensate for.
struct CameraComponent
{
    float followSpeed = 5.0f;
    float zoomLevel = 1.0f;
    float predictionFactor = 0.1f;
};

} // namespace de

#endif // DE_COMPONENTS_CAMERA_COMPONENT_HPP
