#ifndef CAMERA_COMPONENT_HPP
#define CAMERA_COMPONENT_HPP

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

#endif