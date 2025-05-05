#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Camera
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float fov;

public:
    explicit Camera(float fovDegrees) :
        position(glm::vec3(0.0f, 0.0f, 60.0f)),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        fov(fovDegrees)
    {
    }

    glm::mat4 getViewMatrix() const
    {
        std::cout << "Camera pos: " << position.x << ", " << position.y << ", " << position.z << "\n";
        std::cout << "Front: " << front.x << ", " << front.y << ", " << front.z << "\n";
        std::cout << "Up: " << up.x << ", " << up.y << ", " << up.z << "\n";

        return glm::lookAt(position, position + front, up);
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

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    float getFov() const { return fov; }

    void setPosition(const glm::vec3& pos) { position = pos; }
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

    void rotate(float yaw)
    {
        front.x = cos(glm::radians(yaw));
        front.y = 0;
        front.z = sin(glm::radians(yaw));
    }
};
