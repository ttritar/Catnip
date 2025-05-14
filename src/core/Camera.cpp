#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>

// Ctor & Dtor
//-----------------
cat::Camera::Camera(Window& window,glm::vec3 origin, float fovy, float nearPlane, float farPlane)
	: m_Window(window), m_Origin(origin), m_FieldOfView(fovy), m_NearPlane(nearPlane), m_FarPlane(farPlane)
{
}


void cat::Camera::Update(float deltaTime)
{
	GLFWwindow* window = m_Window.GetWindow();
	const float moveSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 60.0f : 30.0f);
	const float mouseSensitivity = 0.005f;

	// KEYBOARD INPUT
	//-----------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_Origin += m_Forward * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_Origin -= m_Forward * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_Origin -= m_Right * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_Origin += m_Right * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) m_Origin += m_Up * moveSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) m_Origin -= m_Up * moveSpeed * deltaTime;


	// MOUSE INPUT
	//-----------------
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	float deltaX = static_cast<float>(mouseX - m_LastMouseX);
	float deltaY = static_cast<float>(mouseY - m_LastMouseY);
	m_LastMouseX = static_cast<float>(mouseX);
	m_LastMouseY = static_cast<float>(mouseY);

	bool rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	bool lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	glm::mat4  finalRotation = glm::mat4(1.0f);
	if (rmb&&lmb)
	{
		m_Origin += m_Up * deltaY* mouseSensitivity;
	}
	else if (rmb)
	{
		m_TotalYaw += deltaX * mouseSensitivity; 
		m_TotalPitch += deltaY * mouseSensitivity;

		m_TotalPitch = glm::clamp(m_TotalPitch, -glm::half_pi<float>(), glm::half_pi<float>()); // clamp pitch to avoid gimbal lock

		// Handle rotation on axises
		finalRotation = glm::rotate(finalRotation, m_TotalPitch, glm::vec3(1, 0, 0));
		finalRotation = glm::rotate(finalRotation, m_TotalYaw, glm::vec3(0, 1, 0));

		m_Forward = glm::normalize(glm::vec3(finalRotation * glm::vec4(0, 0, 1, 0)));
		m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0, 1, 0)));
		m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
	}
	else if (lmb)
	{
		m_TotalYaw += deltaX * mouseSensitivity;
		m_Origin -= m_Forward * deltaY * mouseSensitivity;


		// Handle rotation on axises
		finalRotation = glm::rotate(finalRotation, m_TotalPitch, glm::vec3(1, 0, 0));
		finalRotation = glm::rotate(finalRotation, m_TotalYaw, glm::vec3(0, 1, 0));

		m_Forward = glm::normalize(glm::vec3(finalRotation * glm::vec4(0, 0, 1, 0)));
		m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0, 1, 0)));
		m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
	}
}



// Getters & Setters
//----------------
const glm::mat4& cat::Camera::GetProjection()
{
	m_ProjectionMatrix = glm::perspectiveRH_ZO(m_FieldOfView,
		m_Window.GetAspectRatio(),
		m_NearPlane, m_FarPlane);

	m_ProjectionMatrix[1][1] *= -1; // flip the Y axis

	return m_ProjectionMatrix;
}

const glm::mat4& cat::Camera::GetView()
{
	m_ViewMatrix = glm::inverse(GetInverseView());

	return m_ViewMatrix;
}

const glm::mat4& cat::Camera::GetInverseView()
{
	m_InverseViewMatrix = glm::lookAtLH(m_Origin, m_Origin + m_Forward, m_Up);
	return m_InverseViewMatrix;
}

// Private Methods
//-----------------
