#pragma once
#include "Renderer.h"
#include "Window.h"

class Application final
{
public:
	void Run()
		{
			while (!glfwWindowShouldClose(m_Window.GetWindow()))
			{
				glfwPollEvents();
				m_Renderer.Update();
				m_Renderer.Render();
			}
		}

private:
	cat::Window m_Window{ 800,500,"cati" };
	cat::Renderer m_Renderer{ m_Window };
};