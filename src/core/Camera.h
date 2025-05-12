#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Window.h"

namespace cat
{
	class Camera
	{
	public:
		// Ctor & Dtor
		//-----------------
		Camera(Window& window, glm::vec3 origin = { 0.f,0.f,0.f } ,float fovy = glm::radians(90.0f), float nearPlane = 0.1f, float farPlane = 100.0f);


		// Methods
		//-----------------
		void Update(float deltaTime);

		// Getters & Setters
		const glm::mat4& GetProjection();
		const glm::mat4& GetView();
		const glm::mat4& GetInverseView();

		const glm::vec3 GetForward() const { return m_Forward; }
		const glm::vec3 GetUp() const { return m_Up; }
		const glm::vec3 GetRight() const { return m_Right; }

		const float GetFieldOfView() const { return m_FieldOfView; }
		const float GetNearPlane() const { return m_NearPlane; }
		const float GetFarPlane() const { return m_FarPlane; }

	private:
		// Private Methods
		//-----------------

		// Private Members
		//-----------------
		Window& m_Window;

		glm::mat4 m_ProjectionMatrix{ 1.f };
		glm::mat4 m_ViewMatrix{ 1.f };
		glm::mat4 m_InverseViewMatrix{ 1.f };

		glm::vec3 m_Forward{ glm::vec3{0.0f,0.0f,1.0f} };
		glm::vec3 m_Up{ glm::vec3(0.0f, 1.0f, 0.0f) };
		glm::vec3 m_Right{ glm::vec3{1.0f,0.0f,0.0f}};

		glm::vec3 m_Origin;

		float m_FieldOfView;
		float m_NearPlane;
		float m_FarPlane;

		double m_LastMouseX{};
		double m_LastMouseY{};

		float m_TotalPitch{};
		float m_TotalYaw{};
	};
}
