#pragma once

#include "Renderer.h"
#include "Window.h"

// std
#include <chrono>

class Application final
{
public:
	void Run()
	{
		cat::Window window{ 800,500,"cati" };
		cat::Renderer renderer{ window };

		using clock = std::chrono::high_resolution_clock;
		auto lastTime = clock::now();

		while (!glfwWindowShouldClose(window.GetWindow()))
		{
			auto currentTime = clock::now();
			std::chrono::duration<float> delta = currentTime - lastTime;
			lastTime = currentTime;

			float deltaTime = delta.count(); 

			glfwPollEvents();
			renderer.Update(deltaTime);
			renderer.Render();
		}
	}

};