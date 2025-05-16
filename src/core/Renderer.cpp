#include "Renderer.h"
#include "Window.h"
#include "../vulkan/Device.h"

namespace cat
{

	Renderer::Renderer(Window& window)
		: m_Window(window), m_Device(m_Window.GetWindow()),
		m_Camera(m_Window, { 0.f,0.f,-1.f }, glm::radians(90.0f), 0.1f, 100.0f)
	{
		InitializeVulkan();
	}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(m_Device.GetDevice());


		//  CLEANUP
		//-----------
		delete m_pSwapChain;
		delete m_pGraphicsPipeline;
		delete m_pScene;
		delete m_pUniformBuffer;
		delete m_pCommandBuffer;

		glfwTerminate();
	}

	void Renderer::Update(float deltaTime)
	{
		m_Camera.Update(deltaTime);
		m_pScene->Update(deltaTime);
		m_pUniformBuffer->Update(m_CurrentFrame, m_Camera.GetView(), m_Camera.GetProjection());
	}

	void Renderer::Render() const
	{
		DrawFrame();
	}

	void Renderer::InitializeVulkan()
	{
		m_pSwapChain = new SwapChain(m_Device, m_Window.GetWindow());

		m_pUniformBuffer = new UniformBuffer(m_Device, m_pSwapChain);

		//rawr
		DescriptorSetLayout* descriptorSetLayout = new DescriptorSetLayout(m_Device);
		descriptorSetLayout = descriptorSetLayout
			->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			->AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)	// diffuse sampler
			->Create();

		m_pGraphicsPipeline = new cat::Pipeline(
			m_Device,
			m_pSwapChain,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			descriptorSetLayout->GetDescriptorSetLayout()
		);

		delete descriptorSetLayout;

		// MODELS!
		m_pScene = new Scene(m_Device, *m_pSwapChain, m_pGraphicsPipeline, m_pUniformBuffer);

		m_pScene->AddModel("resources/sponza.obj");

		m_pCommandBuffer = new CommandBuffer(m_Device);
	}

	void Renderer::DrawFrame() const
	{
		vkWaitForFences(m_Device.GetDevice(), 1, m_pSwapChain->GetInFlightFences(m_CurrentFrame), VK_TRUE, UINT64_MAX); // wait until previous frame has finished

		// AQUIRING AN IMAGE FROM THE SWAPCHAIN
		VkResult result = vkAcquireNextImageKHR(m_Device.GetDevice(), m_pSwapChain->GetSwapChain(), UINT64_MAX, m_pSwapChain->GetImageAvailableSemaphores(m_CurrentFrame), VK_NULL_HANDLE, &m_pSwapChain->GetImageIndex());

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_pSwapChain->RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(m_Device.GetDevice(), 1, m_pSwapChain->GetInFlightFences(m_CurrentFrame)); //reset fence to unsignaled


		// RECORDING THE COMMAND BUFFER
		vkResetCommandBuffer(*m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame), 0);

		RecordCommandBuffer(*m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame), m_pSwapChain->GetImageIndex());

		// SUNMITTING THE COMMAND BUFFER
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_pSwapChain->GetImageAvailableSemaphores(m_CurrentFrame) };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame);

		VkSemaphore signalSemaphore[] = { m_pSwapChain->GetRenderFinishedSemaphores(m_CurrentFrame) };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphore;

		if (vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, *m_pSwapChain->GetInFlightFences(m_CurrentFrame)) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// PRESENTATION
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphore;

		VkSwapchainKHR swapChains[] = { m_pSwapChain->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_pSwapChain->GetImageIndex();

		presentInfo.pResults = nullptr; //optional

		result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pSwapChain->GetFrameBufferResized())
		{
			m_pSwapChain->SetFrameBufferResized(false);
			m_pSwapChain->RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % cat::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const
		// writes the cmd we want executed into a cmd buffer
	{
		// Begin recording:
		VkCommandBufferBeginInfo beginInfo{};   // will reset it, if buffer was alr recorded once
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; //optional - how we will use the cmd buffer
		beginInfo.pInheritanceInfo = nullptr; //optional - relevant for 2nd buffer = specifies which state to inherit from the calling primary command buffers

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin command buffer!");
		}

		m_pSwapChain->GetSwapChainImage(static_cast<int>(m_pSwapChain->GetImageIndex()))->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		m_pSwapChain->GetDepthImage()->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);

		// Starting a render pass:
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderingAttachmentInfoKHR colorAttachmentInfo{};
		colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachmentInfo.imageView = m_pSwapChain->GetSwapChainImageView(m_pSwapChain->GetImageIndex());
		colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentInfo.clearValue = clearValues[0];

		VkRenderingInfoKHR renderInfo{};

		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea.offset = { 0, 0 };
		renderInfo.renderArea.extent = m_pSwapChain->GetSwapChainExtent();
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachmentInfo;

		vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

		// Basic drawing commands:
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetGraphicsPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_pSwapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(m_pSwapChain->GetSwapChainExtent().height);

		viewport.maxDepth = 1.f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport); // dynamic -  needs to be set in cmd buffer before issuing draw cmd

		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = m_pSwapChain->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor); // dynamic -  needs to be set in cmd buffer before issuing draw cmd

		m_pScene->Draw(commandBuffer, m_pGraphicsPipeline->GetPipelineLayout(), m_CurrentFrame);


		// Finishing up:
		vkCmdEndRenderingKHR(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}

		m_pSwapChain->GetSwapChainImage(static_cast<int>(m_pSwapChain->GetImageIndex()))->TransitionImageLayout(
			commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);

		m_pSwapChain->GetDepthImage()->TransitionImageLayout(
			commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

}
