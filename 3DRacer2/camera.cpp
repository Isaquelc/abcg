#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

void Camera::computeProjectionMatrix(int width, int height) {
    m_projMatrix = glm::mat4(1.0f);
    const auto aspect{static_cast<float>(width) / static_cast<float>(height)};
    m_projMatrix = glm::perspective(glm::radians(50.0f), aspect, 0.01f, 100.0f);
}

void Camera::computeViewMatrix() {
    m_viewMatrix = glm::lookAt(m_eye, m_at, m_up);
}