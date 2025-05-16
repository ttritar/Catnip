#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <fstream>
#include <string>
#include <vector>

#include "Mesh.h"

namespace cat
{
	class Pipeline final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Pipeline(Device& device, SwapChain* swapchain, const std::string& vertPath, const std::string& fragPath,
		         const VkDescriptorSetLayout& descriptorSetLayout);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;


		// Methods
		//--------------------

		// Getters & Setters
		VkPipeline GetGraphicsPipeline() const { return m_GraphicsPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }


	private:
		// Private Methods
		//--------------------
		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
		static std::vector<char> ReadFile(const std::string& filename);



		// Private Members
		//--------------------
		VkPipeline m_GraphicsPipeline;
		VkPipelineLayout m_PipelineLayout;

		const std::string m_VertPath;
		const std::string m_FragPath;

		Device& m_Device;
		SwapChain* m_pSwapChain;
		VkExtent2D m_SwapChainExtent;
		VkDescriptorSetLayout m_DescriptorSetLayout;
	};

}
