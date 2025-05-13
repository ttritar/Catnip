#include "UniformBuffer.h"

// std
#include <chrono>


namespace cat
{

    UniformBuffer::UniformBuffer(Device& device, SwapChain* swapChain)
		: m_Device(device), m_SwapChain(swapChain)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(cat::MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersMemory.resize(cat::MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersMapped.resize(cat::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < cat::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);

            vkMapMemory(m_Device.GetDevice(), m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
        }
    }

    UniformBuffer::~UniformBuffer()
    {
        for (size_t i = 0; i < cat::MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(m_Device.GetDevice(), m_UniformBuffers[i], nullptr);
            vkFreeMemory(m_Device.GetDevice(), m_UniformBuffersMemory[i], nullptr);
        }
    }

    void UniformBuffer::Update(unsigned int currentImage, 
        const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


        UniformBufferObject ubo{};

        // MODEL ROTATION
        ubo.model = model;

        // VIEW TRANSFORMATION
        ubo.view = view;

        // PERSPECTIVE PROJECTION   
        ubo.proj = proj;


        // Copy data to uniform buffer
        memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

}
