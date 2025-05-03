#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float yaw;
    float pitch;
    float fov;

public:
    Camera(float fovDegrees = 45.0f) :
        position(glm::vec3(0.0f, 0.0f, 3.0f)),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        yaw(-90.0f),
        pitch(0.0f),
        fov(fovDegrees) {}

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    // Getters
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    float getFov() const { return fov; }

    // Camera movement
    void setPosition(const glm::vec3& pos) { position = pos; }
    
    void moveForward(float distance) {
        position += front * distance;
    }

    void moveRight(float distance) {
        glm::vec3 right = glm::normalize(glm::cross(front, up));
        position += right * distance;
    }

    void moveUp(float distance) {
        position += up * distance;
    }

    void rotate(float deltaYaw, float deltaPitch) {
        yaw += deltaYaw;
        pitch += deltaPitch;

        // Constrain pitch to avoid camera flipping
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        // Update front vector
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);
    }

    void setFov(float newFov) {
        fov = glm::clamp(newFov, 1.0f, 120.0f);
    }
};