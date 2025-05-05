#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float yaw;
    float pitch;
    float fov;

public:
    explicit Camera(float fovDegrees) :
        position(glm::vec3(0.0f, 0.0f, 60.0f)),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        yaw(0.0f),
        pitch(0.0f),
        fov(fovDegrees)
    {
    }

    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) const
    {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 10000000.0f);
       // return glm::perspective(glm::radians(fov), aspectRatio, 10.0f, 100000000.0f);
    }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    float getFov() const { return fov; }

    void setPosition(const glm::vec3& pos)
    {
        position = pos;
    }
    //
    // void moveForward(float distance)
    // {
    //     position += front * distance;
    // }
    //
    // void moveRight(float distance)
    // {
    //     glm::vec3 right = glm::normalize(glm::cross(front, up));
    //     position += right * distance;
    // }
    //
    // void moveUp(float distance)
    // {
    //     position += up * distance;
    // }

    void rotate(float newYaw)
    {
        yaw = newYaw;

        // Update front vector
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);
    }
};
