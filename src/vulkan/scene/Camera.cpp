#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <glm/gtx/hash.hpp>

// std
#include <iostream>

// Ctor & Dtor
//-----------------
cat::Camera::Camera(Window& window,glm::vec3 origin, Specifications specs)
	: m_Window(window), m_Origin(origin), m_Specs(specs)
{
	UpdateAspectRatio();
}


void cat::Camera::Update(float deltaTime)
{
	if (m_Window.GetFrameBufferResized())
		UpdateAspectRatio(); 

	// Handle Input
	//-----------------
	HandleKeyboardInput(deltaTime);
	HandleMouseInput();


	// UPDATING VECTORS
	//-----------------
	if (m_IsPositionDirty)	UpdateVectors();
}

void cat::Camera::UpdateAspectRatio()
{
	m_Specs.aspectRatio = m_Window.GetAspectRatio();
	m_IsSpecsDirty = true;
}


// Getters & Setters
//----------------
const glm::mat4& cat::Camera::GetProjection()
{
	if (m_IsSpecsDirty)
	{
		m_IsSpecsDirty = false;

		m_ProjectionMatrix = glm::perspectiveLH(
			m_Specs.fovy,
			m_Window.GetAspectRatio(),
			m_Specs.nearPlane, m_Specs.farPlane
		);

		m_ProjectionMatrix[1][1] *= -1; // flip the Y axis
	}

	return m_ProjectionMatrix;
}

const glm::mat4& cat::Camera::GetView()
{
	if (m_IsPositionDirty)
	{
		m_IsPositionDirty = false;
		m_ViewMatrix = lookAtLH(m_Origin, m_Origin + m_Forward, m_Up);
	}

	return m_ViewMatrix;
}

// Private Methods
//-----------------
void cat::Camera::UpdateVectors() const
{
	if (!m_IsPositionDirty) return;
	m_IsPositionDirty = false;

	// rotation
	float pitch = glm::radians(m_TotalPitch);
	float yaw = glm::radians(m_TotalYaw);

	glm::vec3 fwd;
	fwd.x = cos(pitch) * sin(yaw);
	fwd.y = -sin(pitch);
	fwd.z = cos(pitch) * cos(yaw);

	// recalculate vectors
	m_Forward = glm::normalize(fwd);
	m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0, 1, 0)));
	m_Up = glm::normalize(glm::cross(m_Right, m_Forward));

	m_ViewMatrix = lookAtLH(m_Origin, m_Origin + m_Forward, m_Up);
}


#pragma region Input Handling
void cat::Camera::OutputKeybinds()
{
	std::cout << COLOR_GREEN << "CAMERA CONTROLS: " << COLOR_GREY << std::endl;
	std::cout << COLOR_YELLOW << "\tWASD" << COLOR_GREY << " - Move Forward/Backward/Left/Right" << std::endl;
	std::cout << COLOR_YELLOW << "\tQE" << COLOR_GREY << " - Move Up/Down" << std::endl;
	std::cout << COLOR_YELLOW << "\tMouse Right Click" << COLOR_GREY << " - Rotate Camera" << std::endl;
	std::cout << COLOR_YELLOW << "\tMouse Left Click" << COLOR_GREY << " - Rotate Camera (Yaw only)" << std::endl;
	std::cout << COLOR_YELLOW << "\tMouse Wheel" << COLOR_GREY << " - Zoom In/Out (Aperture Control)" << std::endl;
	std::cout << COLOR_YELLOW << "\tEqual Sign ( = )" << COLOR_GREY << " - Increase Speed" << std::endl;
	std::cout << COLOR_YELLOW << "\tMinus Sign ( - )" << COLOR_GREY << " - Decrease Speed" << std::endl;
	std::cout << std::endl;		 
	std::cout << COLOR_YELLOW << "\tLeft Bracket ( [ )" << COLOR_GREY << " - Decrease Aperture" << std::endl;
	std::cout << COLOR_YELLOW << "\tRight Bracket ( ] )" << COLOR_GREY << " - Increase Aperture" << std::endl;
	std::cout << COLOR_YELLOW << "\tSemicolon ( ; )" << COLOR_GREY << " - Decrease Shutter Speed" << std::endl;
	std::cout << COLOR_YELLOW << "\tApostrophe ( ' )" << COLOR_GREY << " - Increase Shutter Speed" << std::endl;
	std::cout << COLOR_YELLOW << "\tPeriod ( . )" << COLOR_GREY << " - Decrease ISO" << std::endl;
	std::cout << COLOR_YELLOW << "\tSlash ( / )" << COLOR_GREY << " - Increase ISO" << std::endl;

	std::cout << COLOR_RESET << std::endl;
}

void cat::Camera::HandleKeyboardInput(float deltaTime)
{
	auto window = m_Window.GetWindow();

	// HANDLING
	//-----------------
	HandleMoveInput(deltaTime, window);
	HandleSpeedInput(deltaTime, window);
	HandleToneMappingControlInput(deltaTime, window);
}

void cat::Camera::HandleMouseInput()
{
	auto window = m_Window.GetWindow();

	// MOUSE INPUT
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	float deltaX = static_cast<float>(mouseX - m_LastMouseX);
	float deltaY = static_cast<float>(mouseY - m_LastMouseY);

	m_LastMouseX = static_cast<float>(mouseX);
	m_LastMouseY = static_cast<float>(mouseY);

	bool rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	bool lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;


	// HANDLING
	//-----------------
	HandleRotationInput(lmb, rmb, glm::vec2(deltaX, deltaY));
}



void cat::Camera::HandleMoveInput(float deltaTime, GLFWwindow* window)
{
	// WASD
	//-----------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		m_Origin += m_Forward * m_Speed * deltaTime;
		m_IsPositionDirty = true;
		char* StatsString = nullptr;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		m_Origin -= m_Forward * m_Speed * deltaTime;
		m_IsPositionDirty = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		m_Origin += m_Right * m_Speed * deltaTime;
		m_IsPositionDirty = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		m_Origin -= m_Right * m_Speed * deltaTime;
		m_IsPositionDirty = true;
	}


	// QE
	//-----------------
	if (glfwGetKey(m_Window.GetWindow(), GLFW_KEY_E) == GLFW_PRESS)
	{
		m_Origin += m_Up * m_Speed * deltaTime;
		m_IsPositionDirty = true;
	}
	if (glfwGetKey(m_Window.GetWindow(), GLFW_KEY_Q) == GLFW_PRESS)
	{
		m_Origin -= m_Up * m_Speed * deltaTime;
		m_IsPositionDirty = true;
	}
}

void cat::Camera::HandleSpeedInput(float deltaTime, GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
		++m_Speed;
	else if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
		--m_Speed;

	m_Speed = max(m_Speed, 0.1f);
}

void cat::Camera::HandleToneMappingControlInput(float deltaTime, GLFWwindow* window)
{
	// APERTURE
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
	{
		m_Specs.aperture -= 1.f * deltaTime;
		m_IsSpecsDirty = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
	{
		m_Specs.aperture += 1.f * deltaTime;
		m_IsSpecsDirty = true;
	}

	// SHUTTER SPEED
	if (glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS)
	{
		m_Specs.shutterSpeed -= 0.1f * deltaTime;
		m_IsSpecsDirty = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_APOSTROPHE) == GLFW_PRESS)
	{
		m_Specs.shutterSpeed += 0.1f * deltaTime;
		m_IsSpecsDirty = true;
	}

	// ISO
	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
	{
		m_Specs.iso -= 1000.0f * deltaTime;
		m_IsSpecsDirty = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS)
	{
		m_Specs.iso += 1000.0f * deltaTime;
		m_IsSpecsDirty = true;
	}
}

void cat::Camera::HandleRotationInput(bool lmb, bool rmb, glm::vec2 d)
{
	// ROTATION
	//-----------------
	if (rmb || lmb)	m_IsPositionDirty = true;

	if (rmb && lmb)
	{
	}
	else if (rmb)
	{
	}
	else if (lmb)
	{
		m_TotalPitch += d.y * m_MouseSensitivity;
		m_TotalYaw += d.x * m_MouseSensitivity;
		m_TotalPitch = std::clamp(m_TotalPitch, -89.9f, 89.9f);
	}
}

#pragma endregion
