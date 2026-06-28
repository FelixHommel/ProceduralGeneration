#include "Camera.hpp"
#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>

namespace pen
{

Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
    : m_position{ position }, m_worldUp{ up }, m_yaw{ yaw }, m_pitch{ pitch }
{
    updateCameraVectors();
}

void Camera::processKeyboard(CameraMovement direction, float dt)
{
    const float velocity{ m_movementSpeed * dt };

    if(direction == CameraMovement::FORWARD)
        m_position += m_front * velocity;
    else if(direction == CameraMovement::BACKWARD)
        m_position -= m_front * velocity;

    if(direction == CameraMovement::RIGHT)
        m_position += m_right * velocity;
    else if(direction == CameraMovement::LEFT)
        m_position -= m_right * velocity;

    if(direction == CameraMovement::UP)
        m_position += m_up * velocity;
    else if(direction == CameraMovement::DOWN)
        m_position -= m_up * velocity;

    const float turnSpeed{ m_mouseSensitivity * dt };

    if(direction == CameraMovement::YAW_RIGHT)
        m_yaw += ::YAW_OFFSET * turnSpeed;
    else if(direction == CameraMovement::YAW_LEFT)
        m_yaw -= ::YAW_OFFSET * turnSpeed;

    if(direction == CameraMovement::PITCH_UP)
        m_pitch += ::PITCH_OFFSET * turnSpeed;
    else if(direction == CameraMovement::PITCH_DOWN)
        m_pitch -= ::PITCH_OFFSET * turnSpeed;

    if(m_pitch > ::PITCH_CONSTRAIN)
        m_pitch = ::PITCH_CONSTRAIN;

    if(m_pitch < -::PITCH_CONSTRAIN)
        m_pitch = -::PITCH_CONSTRAIN;

    updateCameraVectors();
}

void Camera::processMouseMovement(float offsetX, float offsetY, bool constrainPitch)
{
    offsetX *= m_mouseSensitivity;
    offsetY *= m_mouseSensitivity;

    m_yaw += offsetX;
    m_pitch -= offsetY;

    if(constrainPitch)
    {
        if(m_pitch > ::PITCH_CONSTRAIN)
            m_pitch = ::PITCH_CONSTRAIN;

        if(m_pitch < -::PITCH_CONSTRAIN)
            m_pitch = -::PITCH_CONSTRAIN;
    }

    updateCameraVectors();
}

void Camera::porocessMouseScroll(float offsetY)
{
    m_zoom -= offsetY;

    if(m_zoom < ::ZOOM_MIN)
        m_zoom = ::ZOOM_MIN;

    if(m_zoom > ::ZOOM_MAX)
        m_zoom = ::ZOOM_MAX;
}

/// \brief Calculate the new camera vectors based on the updated positional data.
void Camera::updateCameraVectors()
{
    glm::vec3 front{ std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch)),
                     std::sin(glm::radians(m_pitch)),
                     std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch)) };

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace pen

