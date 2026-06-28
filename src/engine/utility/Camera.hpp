#ifndef PEN_SRC_ENGINE_UTILITY_CAMERA_HPP
#define PEN_SRC_ENGINE_UTILITY_CAMERA_HPP

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <cstdint>

namespace
{

constexpr auto DEFAULT_POSITION{ glm::vec3(0.f) };
constexpr auto DEFAULT_FRONT{ glm::vec3(0.f, 0.f, -1.f) };
constexpr auto DEFAULT_UP{ glm::vec3(0.f, 1.f, 0.f) };
constexpr auto DEFAULT_YAW{ -90.f };
constexpr auto DEFAULT_PITCH{ 0.f };
constexpr auto DEFAULT_SPEED{ 2.5f };
constexpr auto DEFAULT_SENSITIVITY{ 0.1f };
constexpr auto DEFAULT_ZOOM{ 45.f };

constexpr auto YAW_OFFSET{ 200.f };
constexpr auto PITCH_OFFSET{ 200.f };
constexpr auto PITCH_CONSTRAIN{ 89.f };

constexpr auto ZOOM_MIN{ 1.f };
constexpr auto ZOOM_MAX{ 45.f };

} // namespace

namespace pen
{

/// \brief The \ref Camera is a abstraction for the observer position.
///
/// \author Felix Hommel
/// \date 6/27/2026
class Camera
{
public:
    /// \brief \ref CameraMovement indicates the direction in which the camera is moved.
    ///
    /// \author Felix Hommel
    /// \date 6/27/2026
    enum class CameraMovement : std::uint8_t
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN,
        YAW_LEFT,
        YAW_RIGHT,
        PITCH_UP,
        PITCH_DOWN
    };

    explicit Camera(
        const glm::vec3& position = ::DEFAULT_POSITION,
        const glm::vec3& up = ::DEFAULT_UP,
        float yaw = ::DEFAULT_YAW,
        float pitch = ::DEFAULT_PITCH
    );

    [[nodiscard]] glm::vec3 position() const noexcept { return m_position; }
    [[nodiscard]] float zoom() const noexcept { return m_zoom; }
    [[nodiscard]] glm::mat4 viewMatrix() const { return glm::lookAt(m_position, m_position + m_front, m_up); }

    /// \brief Apply keyboard input on the camera's position.
    ///
    /// \param direction The \ref CameraMovement direction.
    /// \param dt The time delta between frames
    void processKeyboard(CameraMovement direction, float dt);
    /// \brief Apply mouse movement input on the camera's position.
    ///
    /// \param offsetX The offset the mouse moved on the x-axis
    /// \param offsetY The offset the mouse moved on the y-axis
    /// \param constrainPitch Whether or not the maximum pitch should be constrained or not
    void processMouseMovement(float offsetX, float offsetY, bool constrainPitch = true);
    /// \brief Apply mouse wheel movement on the camera's zoom.
    ///
    /// \param offsetY The amount of scroll that happened
    void porocessMouseScroll(float offsetY);

private:
    glm::vec3 m_position;
    glm::vec3 m_front{ ::DEFAULT_FRONT };
    glm::vec3 m_up{};
    glm::vec3 m_right{};
    glm::vec3 m_worldUp;
    float m_yaw;
    float m_pitch;
    float m_movementSpeed{ ::DEFAULT_SPEED };
    float m_mouseSensitivity{ ::DEFAULT_SENSITIVITY };
    float m_zoom{ ::DEFAULT_ZOOM };

    void updateCameraVectors();
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_UTILITY_CAMERA_HPP

