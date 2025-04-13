#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string>


class Window final
{
public:
	Window() = default;

	void Initialize(int width, int height, const char* title);
	void Destroy();

	// Getters & Setters
	//--------------------
	GLFWwindow* GetWindow() const { return m_pWindow; }

	int GetWidth() const { return m_Width; }	
	int GetHeight() const { return m_Height; }


private:
	GLFWwindow* m_pWindow;

	// Window properties
	int m_Width;
	int m_Height;

};