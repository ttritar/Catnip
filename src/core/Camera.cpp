#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>

// Ctor & Dtor
//-----------------
cat::Camera::Camera(Window& window,glm::vec3 origin, float fovy, float near, float far)
	: m_Window(window), m_Origin(origin), m_FieldOfView(fovy), m_NearPlane(near), m_FarPlane(far)
{
}


// Methods
//-----------------
void cat::Camera::Update(float deltaTime)
{
	GLFWwindow* window = m_Window.GetWindow();
	float moveSpeed = 30.0f;
	const float rotationSpeed = 30.0f;


	// CAMERA MOVEMENT
	//-----------------

	// Keyboard Input
	if (glfwGetKey(window,GLFW_KEY_LEFT_SHIFT))	moveSpeed *= 2;

	if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))		// Move (local) Forward (Arrow Up) and (‘W’)
	{
		m_Origin += m_Forward * deltaTime * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT))		// Move (local) Left (Arrow Left) and (‘A’)
	{
		m_Origin += m_Forward * (deltaTime * moveSpeed * -1);
	}
	if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))		// Move (local) Backward (Arrow Down) and (‘S’)
	{
		m_Origin += m_Forward * (deltaTime * moveSpeed * -1);
	}
	if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT))	// Move (local) Right (Arrow Right) and (‘D’)
	{
		m_Origin += m_Forward * deltaTime * moveSpeed;
	}

	// Mouse Input
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	float mouseX = static_cast<float>(x - m_LastMouseX);
	float mouseY = static_cast<float>(y - m_LastMouseY);
	m_LastMouseX = x;
	m_LastMouseY = y;

	bool lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	bool rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	glm::mat4 finalRotation{1.0f};

	if (lmb && rmb)
	{
		m_Origin += m_Up * static_cast<float>(x);
	}
	else if (lmb)
	{
		m_TotalYaw += mouseX / 360 * glm::pi<float>();	// Rotate Yaw (LMB + Mouse Move X)
		m_Origin -= m_Forward * mouseY * 0.2f;			// Move (local) Forward/Backward (LMB + Mouse Move Y)

		finalRotation = glm::rotate(finalRotation, m_TotalYaw, { 0,1,0 });

		m_Forward = glm::vec3(finalRotation * glm::vec4({ 0.0f, 0.0f, 1.0f ,0.0f }));
		m_Forward = glm::normalize(m_Forward);

		m_Up = glm::vec3(finalRotation * glm::vec4({ 0.0f, 1.0f, 0.0f ,0.0f }));
		m_Up = glm::normalize(m_Up);

		m_Right= glm::vec3(finalRotation * glm::vec4({ 1.0f, 0.0f, 0.0f ,0.0f }));
		m_Right = glm::normalize(m_Right);

		std::cout << m_Forward.x << " " << m_Forward.y << " " << m_Forward.z << std::endl;
		std::cout << m_Up.x << " " << m_Up.y << " " << m_Up.z << std::endl;
		std::cout << m_Right.x << " " << m_Right.y << " " << m_Right.z << std::endl;

	}
	else if (rmb)
	{
		m_TotalYaw += mouseX / 360 * glm::pi<float>();		// Rotate Yaw (LMB + Mouse Move X)
		m_TotalPitch -= mouseY / 360 * glm::pi<float>();	// Rotate Pitch (RMB + Mouse Move Y)

		finalRotation = glm::rotate(finalRotation, m_TotalPitch, { 1,0,0 });
		finalRotation = glm::rotate(finalRotation, m_TotalYaw, { 0,1,0 });

		m_Forward = glm::vec3(finalRotation * glm::vec4({ 0.0f, 0.0f, 1.0f ,0.0f }));
		m_Forward = glm::normalize(m_Forward);

		m_Up = glm::vec3(finalRotation * glm::vec4({ 0.0f, 1.0f, 0.0f ,0.0f }));
		m_Up = glm::normalize(m_Up);

		m_Right = glm::vec3(finalRotation * glm::vec4({ 1.0f, 0.0f, 0.0f ,0.0f }));
		m_Right = glm::normalize(m_Right);

		std::cout << m_Forward.x << " " << m_Forward.y << " " << m_Forward.z << std::endl;
		std::cout << m_Up.x << " " << m_Up.y << " " << m_Up.z << std::endl;
		std::cout << m_Right.x << " " << m_Right.y << " " << m_Right.z << std::endl;
	}

}



// Getters & Setters
//----------------
const glm::mat4& cat::Camera::GetProjection()
{
	m_ProjectionMatrix = glm::perspectiveFovLH(m_FieldOfView,
		static_cast<float>( m_Window.GetWidth()), static_cast<float>(m_Window.GetHeight()), 
		m_NearPlane, m_FarPlane);
	//std::cout << "Projection Matrix: " << m_ProjectionMatrix[0][0] << " " << m_ProjectionMatrix[1][1] << " " << m_ProjectionMatrix[2][2] << " " << m_ProjectionMatrix[3][3] << std::endl;
	//std::cout << "Projection Matrix: " << m_ProjectionMatrix[0][1] << " " << m_ProjectionMatrix[1][0] << " " << m_ProjectionMatrix[2][3] << " " << m_ProjectionMatrix[3][2] << std::endl;
	//std::cout << "Projection Matrix: " << m_ProjectionMatrix[0][2] << " " << m_ProjectionMatrix[1][3] << " " << m_ProjectionMatrix[2][0] << " " << m_ProjectionMatrix[3][1] << std::endl;
	//std::cout << "Projection Matrix: " << m_ProjectionMatrix[0][3] << " " << m_ProjectionMatrix[1][2] << " " << m_ProjectionMatrix[2][1] << " " << m_ProjectionMatrix[3][0] << std::endl;

	return m_ProjectionMatrix;
}

const glm::mat4& cat::Camera::GetView()
{
	m_ViewMatrix = glm::inverse(GetInverseView());
	//std::cout << "View Matrix: " << m_ViewMatrix[0][0] << " " << m_ViewMatrix[1][1] << " " << m_ViewMatrix[2][2] << " " << m_ViewMatrix[3][3] << std::endl;
	//std::cout << "View Matrix: " << m_ViewMatrix[0][1] << " " << m_ViewMatrix[1][0] << " " << m_ViewMatrix[2][3] << " " << m_ViewMatrix[3][2] << std::endl;
	//std::cout << "View Matrix: " << m_ViewMatrix[0][2] << " " << m_ViewMatrix[1][3] << " " << m_ViewMatrix[2][0] << " " << m_ViewMatrix[3][1] << std::endl;
	//std::cout << "View Matrix: " << m_ViewMatrix[0][3] << " " << m_ViewMatrix[1][2] << " " << m_ViewMatrix[2][1] << " " << m_ViewMatrix[3][0] << std::endl;
	return m_ViewMatrix;
}

const glm::mat4& cat::Camera::GetInverseView()
{
	m_InverseViewMatrix = glm::lookAtLH(m_Origin, m_Forward, m_Up);
	return m_InverseViewMatrix;
}

// Private Methods
//-----------------
