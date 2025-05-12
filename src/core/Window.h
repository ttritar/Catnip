#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string>

namespace cat
{
	
class Window final
{
public:
	// CTOR & DTOR
	//--------------------
	Window(int width, int height, const char* title);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	// Methods
	//--------------------
	void InitializeWindow(int width, int height, const char* title);


	// Getters & Setters
	GLFWwindow* GetWindow()const { return m_pWindow; }

	int GetWidth() const { return m_Width; }	
	int GetHeight() const { return m_Height; }
	float GetAspectRatio() const{ return static_cast<float>(m_Width) / static_cast<float>(m_Height); }


private:
	GLFWwindow* m_pWindow;

	// Window properties
	const int m_Width;
	const int m_Height;

};

}