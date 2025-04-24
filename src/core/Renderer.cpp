#include "Renderer.h"

#include "Window.h"

namespace cat
{

	Renderer::Renderer(Window& window, Device& device )
		: m_Window(window), m_Device(device)
	{
        if (!m_pSwapChain) m_pSwapChain = new SwapChain(m_Device,m_Window.GetWindow());
		else m_pSwapChain->RecreateSwapChain();

        CreateCommandBuffer();
	}

    Renderer::~Renderer()
    {
        vkDeviceWaitIdle(m_Device.GetDevice());


        //  CLEANUP
        //-----------
        delete m_pSwapChain;

        vkDestroySampler(m_Device->GetDevice(), textureSampler, nullptr);
        vkDestroyImageView(m_Device->GetDevice(), textureImageView, nullptr);

        vkDestroyImage(m_Device->GetDevice(), textureImage, nullptr);
        vkFreeMemory(m_Device->GetDevice(), textureImageMemory, nullptr);

        for (size_t i = 0; i < cat::MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(m_Device->GetDevice(), uniformBuffers[i], nullptr);
            vkFreeMemory(m_Device->GetDevice(), uniformBuffersMemory[i], nullptr);
        }


        delete m_pGraphicsPipeline;

        delete m_Device;

        glfwTerminate();
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
