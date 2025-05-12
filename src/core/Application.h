#pragma once
#include "Renderer.h"
#include "Window.h"

class Application final
{
public:
	void Run()
		{
			cat::Window window{ 800,500,"cati" };
			cat::Renderer renderer{ window };
			while (!glfwWindowShouldClose(window.GetWindow()))
			{
				glfwPollEvents();
				renderer.Update(1);
				renderer.Render();
			}
		}

private:
};