#pragma once

#include "Application.h"

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