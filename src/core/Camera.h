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
		struct Specifications
		{
			float fovy = glm::radians(90.0f);

			float aspectRatio;

			float nearPlane = 0.1f;
			float farPlane = 1500.0f;
		};


		// Ctor & Dtor
		//-----------------
		Camera(Window& window, glm::vec3 origin = { 0.f,0.f,0.f }, Specifications specs = Specifications{});


		// Methods
		//-----------------
		void Update(float deltaTime);
		void UpdateAspectRatio();

		// Getters & Setters
		const glm::mat4& GetProjection();
		const glm::mat4& GetView();
		const glm::mat4& GetInverseView();

		const glm::vec3 GetForward() const { return m_Forward; }
		const glm::vec3 GetUp() const { return m_Up; }
		const glm::vec3 GetRight() const { return m_Right; }

		const Specifications GetSpecs() const { return m_Specs; }	
		void SetSpecs(const Specifications& specs)
		{
			m_Specs = specs;
			m_IsSpecsDirty = true;
		}

		void SetSpeed(float speed) { m_Speed = speed; }
		void SetMouseSensitivity(float sensitivity) { m_MouseSensitivity = sensitivity; }

	private:
		// Private Methods
		//-----------------
		void HandleKeyboardInput(float deltaTime);
		void HandleMouseInput();

		void UpdateVectors();

		// Private Members
		//-----------------
		Window& m_Window;

		// specs
		Specifications m_Specs{};
		bool m_IsSpecsDirty{true};


		// position
		glm::vec3 m_Origin;
		bool m_IsPositionDirty{true};


		// planes
		glm::mat4 m_ProjectionMatrix{ 1.f };
		glm::mat4 m_ViewMatrix{ 1.f };
		glm::mat4 m_InverseViewMatrix{ 1.f };

		glm::vec3 m_Forward{ glm::vec3{0.0f,0.0f,1.0f} };
		glm::vec3 m_Up{ glm::vec3(0.0f, 1.0f, 0.0f) };
		glm::vec3 m_Right{ glm::vec3{1.0f,0.0f,0.0f}};

		float m_TotalPitch{};
		float m_TotalYaw{};


		// input
		float m_Speed{ 5.f };

		float m_MouseSensitivity{0.1f};
		double m_LastMouseX{};
		double m_LastMouseY{};
	};
}
