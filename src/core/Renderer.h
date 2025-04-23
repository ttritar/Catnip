#pragma once
#include "Singleton.h"
#include "../vulkan/SwapChain.h"

namespace cat
{
	class Pipeline;
	class Model;
	class Window;

	class Renderer : public Singleton<Renderer>
	{
	public:
		// Methods
		//--------------------
		void Initialize(Window* window);
		void Destroy();
		void Render()const;

		// Getters & Setters


	private:
		// Private Methods
		//--------------------
		void InitializeVulkan();

		// Creators
		void CreateCommandBuffer();

		// Helpers


		// Private Members
		//--------------------
		Window* m_Window;
		Pipeline* m_GraphicsPipeline;
		Device* m_Device;
		SwapChain* m_SwapChain;
		Model* m_Model;




	};
}
