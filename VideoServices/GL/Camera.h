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
        // Calculate right and up vectors
        glm::vec3 r = glm::cross(front, up);
        glm::vec3 u = glm::cross(r, front);

        glm::mat4 view(1.0f);
        view[0][0] = r.x;
        view[1][0] = r.y;
        view[2][0] = r.z;

        view[0][1] = u.x;
        view[1][1] = u.y;
        view[2][1] = u.z;

        view[0][2] = -front.x;
        view[1][2] = -front.y;
        view[2][2] = -front.z;

        view[3][0] = -glm::dot(r, position);
        view[3][1] = -glm::dot(u, position);
        view[3][2] = glm::dot(front, position);

        return view;
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) const
    {
        const float nearPlane = 100.0f;
        const float farPlane = 1000000.0f;

        return glm::perspective(
            glm::radians(fov),
            aspectRatio,
            nearPlane,
            farPlane
        );
    }

    glm::mat4 getOrthographicMatrix(float left, float right, float bottom, float top) const
    {
        const float nearPlane = 0.0f;
        const float farPlane = 10000.0f;
        return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    void setFront(glm::vec3 value) { front = value; }
    glm::vec3 getUp() const { return up; }
    void setUp(glm::vec3 value) { up = value; }
    float getFov() const { return fov; }

    void setPosition(const glm::vec3& pos) { position = pos; }

    void rotate(float yaw)
    {
        front.x = glm::cos(yaw);
        front.y = 0;
        front.z = glm::sin(yaw);
    }
};

inline float MR_ANGLE_TO_RADIANS(MR_Angle angle)
{
    float radians = static_cast<float>(angle) * (glm::pi<float>() / static_cast<float>(MR_PI));
    return radians;
}
