#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../Util/WorldCoordinates.h"

class Camera
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float fov;

public:
    explicit Camera(float fovDegrees) :
        position(glm::vec3(0.0f, 0.0f, 0.0f)),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        fov(fovDegrees)
    {
    }

    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) const
    {
        const float nearPlane = 1000.0f;
        const float farPlane = 1000000.0f;

        return glm::perspective(
            glm::radians(fov),
            aspectRatio,
            nearPlane,
            farPlane
        );
    }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    float getFov() const { return fov; }

    void setPosition(const glm::vec3& pos) { position = pos; }

    void rotate(float yaw)
    {
        front.x = cos(glm::radians(yaw));
        front.y = 0;
        front.z = sin(glm::radians(yaw));
    }
};

inline float MR_ANGLE_TO_DEGREES(MR_Angle angle)
{
    float degrees = (static_cast<float>(angle) * 360.0f) / static_cast<float>(MR_2PI);
    return degrees;
}
