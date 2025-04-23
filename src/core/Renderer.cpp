#include "Renderer.h"

namespace cat
{
    void Renderer::InitializeVulkan()
    {
        m_Device = new cat::Device(m_Window.GetWindow());
        m_SwapChain = new cat::SwapChain(*m_Device, m_Window.GetWindow());
        createDescriptorSetLayout();
        m_GraphicsPipeline = new cat::Pipeline(
            m_Device->GetDevice(),
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            m_SwapChain->GetRenderPass(), m_SwapChain->GetSwapChainExtent(), descriptorSetLayout
        );
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        m_Model = new cat::Model(m_Device, vertices, indices);
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSet();
        createCommandBuffer();
    }

	void Renderer::Initialize(Window* window)
	{
		m_Window = window;
		InitializeVulkan();
	}

    void Renderer::Destroy()
    {
    }



	void Renderer::Render() const
	{
	}



	void Renderer::CreateCommandBuffer()
    {
        m_CommandBuffers.resize(cat::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_Device->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // is secondary or primary cmd buffer
        allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

        if (vkAllocateCommandBuffers(m_Device->GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
}
