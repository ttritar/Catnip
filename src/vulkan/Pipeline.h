#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <fstream>
#include <string>
#include <vector>

namespace cat
{
	class Pipeline
	{
	public:
		// CTOR & DTOR
		//--------------------
		Pipeline(const VkDevice& device, const std::string& vertPath, const std::string& fragPath, const VkRenderPass& renderPass, const VkExtent2D& swapChainExtent, const VkDescriptorSetLayout& descriptorSetLayout);
		~Pipeline();
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

		VkDevice m_Device;
		VkRenderPass m_RenderPass;
		VkExtent2D m_SwapChainExtent;
		VkDescriptorSetLayout m_DescriptorSetLayout;

	};

}
