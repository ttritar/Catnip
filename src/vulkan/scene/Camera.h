#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../../core/Window.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_GREY    "\033[90m"

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

			float aperture = 1.4f;
			float shutterSpeed = 1.0f / 60.0f; // 60 FPS
			float iso = 1600.0f; 
		};


		// Ctor & Dtor
		//-----------------
		Camera(Window& window, glm::vec3 origin = { 0.f,0.f,0.f }, Specifications specs = Specifications{});


		// Methods
		//-----------------
		void OutputKeybinds();

		void Update(float deltaTime);
		void UpdateAspectRatio();

		// Getters & Setters
		const glm::mat4& GetProjection();
		const glm::mat4& GetView();
		const glm::mat4& GetInverseView();

		const glm::vec3 GetForward() const { return m_Forward; }
		const glm::vec3 GetUp() const { return m_Up; }
		const glm::vec3 GetRight() const { return m_Right; }

		glm::vec3 GetOrigin()
		{
			if (m_IsPositionDirty)
			{
				UpdateVectors();
				m_IsPositionDirty = false;
			}
			return m_Origin;
		}

		void SetPitchYaw(float pitch, float yaw)
		{
			m_TotalPitch = pitch;
			m_TotalYaw = yaw;
			UpdateVectors();
		}

		Specifications GetSpecs() const { return m_Specs; }	
		void SetSpecs(const Specifications& specs)
		{
			m_Specs = specs;
			m_IsSpecsDirty = true;
		}

		float GetAperture() const { return m_Specs.aperture; }
		float GetShutterSpeed() const { return m_Specs.shutterSpeed; }
		float GetIso() const { return m_Specs.iso; }

		void SetSpeed(float speed) { m_Speed = speed; }
		void SetMouseSensitivity(float sensitivity) { m_MouseSensitivity = sensitivity; }

	private:
		// Private Methods
		//-----------------
		void HandleKeyboardInput(float deltaTime);
		void HandleMouseInput();
		void HandleMoveInput(float deltaTime, GLFWwindow* window);
		void HandleSpeedInput(float deltaTime, GLFWwindow* window);
		void HandleToneMappingControlInput(float deltaTime, GLFWwindow* window);
		void HandleRotationInput(bool lmb, bool rmb, glm::vec2 d);


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
