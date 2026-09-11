#ifndef DE_COMPONENTS_CAMERA_COMPONENT_HPP
#define DE_COMPONENTS_CAMERA_COMPONENT_HPP

namespace de
{
struct CameraComponent
{
    float followSpeed = 5.0f;
    float zoomLevel = 1.0f;
    float predictionFactor = 0.1f;
    float minZoom = 0.5f;
    float maxZoom = 2.0f;
    float viewportOffsetX = -200;
    float viewportOffsetY = -100;
};

} // namespace de

#endif // DE_COMPONENTS_CAMERA_COMPONENT_HPP
