#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>



class Camera
{
public:
	// Ctor & Dtor
	//-----------------
    Camera(float fovy, float aspect, float near, float far);


	// Methods
	//-----------------

	// Getters & Setters
	const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
	const glm::mat4& GetView() const { return m_ViewMatrix; }
	const glm::mat4& GetInverseView() const { return m_InverseViewMatrix; }
	const glm::vec3 GetPosition() const { return glm::vec3(m_InverseViewMatrix[3]); }

    void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
    void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
    void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

private:
	// Private Methods
	//-----------------

	// Private Members
	//-----------------
	glm::mat4 m_ProjectionMatrix{ 1.f };
	glm::mat4 m_ViewMatrix{ 1.f };
	glm::mat4 m_InverseViewMatrix{ 1.f };
};
