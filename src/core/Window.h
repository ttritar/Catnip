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

	void SetFrameBufferResized(bool value) { m_FrameBufferResized = value; }
	bool GetFrameBufferResized() const { return m_FrameBufferResized; }

private:
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	GLFWwindow* m_pWindow;

	// Window properties
	int m_Width;
	int m_Height;

	bool m_FrameBufferResized = false;

};

}