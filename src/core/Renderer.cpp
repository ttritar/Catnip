#include "Renderer.h"

#include <iostream>

#include "Window.h"
#include "../vulkan/Device.h"

namespace cat
{

	Renderer::Renderer(Window& window)
		: m_Window(window), m_Device(m_Window.GetWindow()),
		m_Camera(m_Window, { 0.f,1.f,-1.f })
	{
		m_Camera.SetSpecs({ .fovy = glm::radians(90.f), .nearPlane = 0.1f, .farPlane = 1500.f, .aperture = 1.4f, .shutterSpeed = 1.0f / 60.0f, .iso = 1600.f });
		InitializeVulkan();
		OutputKeybinds();
	}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(m_Device.GetDevice());


		//  CLEANUP
		//-----------
		delete m_pSwapChain;
		delete m_pHDRImage;
		for (auto& scene : m_pScenes)
		{
			delete scene;
		}
		delete m_pUniformBuffer;
		delete m_pCommandBuffer;

		glfwTerminate();
	}

	void Renderer::OutputKeybinds()
	{
		m_Camera.OutputKeybinds();


		// SCENE SWITCHING
		std::cout << COLOR_GREEN	<< "SCENE SWITCHING: " << COLOR_RESET << std::endl;
		std::cout << COLOR_YELLOW	<< "\t Press 0 to switch to Scene 0 (Flight Helmet)" << COLOR_RESET << std::endl;
		std::cout << COLOR_YELLOW	<< "\t Press 1 to switch to Scene 1 (Sponza)" << COLOR_RESET << std::endl;
	}

	void Renderer::Update(float deltaTime)
	{
		if (glfwGetKey(m_Window.GetWindow(), GLFW_KEY_0)) m_pCurrentScene = m_pScenes[0];
		if (glfwGetKey(m_Window.GetWindow(), GLFW_KEY_1)) m_pCurrentScene = m_pScenes[1];


		m_Camera.Update(deltaTime);
		m_pCurrentScene->Update(deltaTime);
		MatrixUbo uboData = { m_Camera.GetView(), m_Camera.GetProjection() };
		m_pUniformBuffer->Update(m_CurrentFrame, uboData);
	}

	void Renderer::Render() const
	{
		DrawFrame();
	}

	void Renderer::InitializeVulkan()
	{
		m_pSwapChain = new SwapChain(m_Device, m_Window.GetWindow());

		m_pUniformBuffer = new UniformBuffer<MatrixUbo>(m_Device);


		// SCENES
		//-----------------
		m_pScenes.resize(2);

		m_pScenes[0] = new Scene(m_Device, m_pUniformBuffer);
		m_pScenes[0]->AddModel("resources/Models/Sponza/Sponza.gltf")
			->SetRotation(glm::radians(90.f), { 0,1,0 });
		m_pScenes[0]->SetDirectionalLight(Scene::DirectionalLight{ .direction = { 0.f, -1.f, 0.f }, .color = { 1.f, 1.f, 1.f }, .intensity = 1000.f });
		m_pScenes[0]->AddPointLight(Scene::PointLight{ .position = { 0.f, 1.f, 5.f ,0.f}, .color = { 1.f, 0.f, 0.f ,0.f}, .intensity = 150.f , .radius = 100.f});
		m_pScenes[0]->AddPointLight(Scene::PointLight{ .position = { 0.f, 1.f, 0.f ,0.f}, .color = { 0.f, 1.f, 0.f ,0.f}, .intensity = 150.f , .radius = 100.f });
		m_pScenes[0]->AddPointLight(Scene::PointLight{ .position = { 0.f, 1.f, 2.5f ,0.f}, .color = { 0.f, 0.f, 1.f ,0.f}, .intensity = 150.f , .radius = 100.f });

		m_pScenes[1] = new Scene(m_Device, m_pUniformBuffer);
		m_pScenes[1]->AddModel("resources/Models/ABeautifulGame/ABeautifulGame.gltf")
			->SetScale({ 1.f,1.f,1.f });
		m_pScenes[1]->SetDirectionalLight(Scene::DirectionalLight{ .direction = { 0.2f, -0.9f, 0.2f }, .color = { 1.f, 1.f, 1.f }, .intensity = 100.f });

		m_pCurrentScene = m_pScenes[1]; // set default scene

		m_pHDRImage = new HDRImage(m_Device, "resources/HDRIs/CircusArena.hdr");

		m_pCommandBuffer = new CommandBuffer(m_Device);

		// PASSES
		//-----------------
		m_pDepthPrepass = std::make_unique<DepthPrepass>(m_Device, cat::MAX_FRAMES_IN_FLIGHT);
		m_pShadowPass = std::make_unique<ShadowPass>(m_Device, cat::MAX_FRAMES_IN_FLIGHT);
		m_pGeometryPass = std::make_unique<GeometryPass>(m_Device, m_pSwapChain->GetSwapChainExtent(), cat::MAX_FRAMES_IN_FLIGHT);
		m_pLightingPass = std::make_unique<LightingPass>(
			m_Device, m_pSwapChain->GetSwapChainExtent(), cat::MAX_FRAMES_IN_FLIGHT, 
			*m_pGeometryPass, 
			m_pHDRImage, *m_pSwapChain, * m_pShadowPass);
		m_pBlitPass = std::make_unique<BlitPass>(m_Device, *m_pSwapChain, cat::MAX_FRAMES_IN_FLIGHT, *m_pLightingPass);
	}

	void Renderer::DrawFrame() const
	{
		vkWaitForFences(m_Device.GetDevice(), 1, m_pSwapChain->GetInFlightFences(m_CurrentFrame), VK_TRUE, UINT64_MAX); // wait until previous frame has finished

		// AQUIRING AN IMAGE FROM THE SWAPCHAIN
		VkResult result = vkAcquireNextImageKHR(m_Device.GetDevice(), m_pSwapChain->GetSwapChain(), UINT64_MAX, m_pSwapChain->GetImageAvailableSemaphores(m_CurrentFrame), VK_NULL_HANDLE, &m_pSwapChain->GetImageIndex());

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_pSwapChain->RecreateSwapChain();
			ResizePasses();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(m_Device.GetDevice(), 1, m_pSwapChain->GetInFlightFences(m_CurrentFrame)); //reset fence to unsignaled


		// RECORDING
		//-----------------
		{
			vkResetCommandBuffer(*m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame), 0);

			// Begin recording:
			VkCommandBufferBeginInfo beginInfo{};   // will reset it, if buffer was alr recorded once
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; //optional - how we will use the cmd buffer
			beginInfo.pInheritanceInfo = nullptr; //optional - relevant for 2nd buffer = specifies which state to inherit from the calling primary command buffers

			if (vkBeginCommandBuffer(*m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame), &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin command buffer!");
			}

			//RecordCommandBuffer(*m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame), m_pSwapChain->GetImageIndex());
			RecordPasses();

			m_pSwapChain->GetSwapChainImage(static_cast<int>(m_pSwapChain->GetImageIndex()))->TransitionImageLayout(
				*m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,{}
			);

			// End recording:
			if (vkEndCommandBuffer(*m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame)) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!");
			}
		}

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
			ResizePasses();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % cat::MAX_FRAMES_IN_FLIGHT;
	}
	
	
	void Renderer::RecordPasses() const
	{
		auto& commandBuffer = *m_pCommandBuffer->GetCommandBuffer(m_CurrentFrame);

		m_pDepthPrepass->Record(
			commandBuffer,
			m_CurrentFrame,
			*m_pSwapChain->GetDepthImage(m_CurrentFrame),
			m_Camera,
			*m_pCurrentScene
		);

		m_pShadowPass->Record(
			commandBuffer,
			m_CurrentFrame,
			*m_pCurrentScene
		);

		m_pGeometryPass->Record(
			commandBuffer,
			m_CurrentFrame,
			*m_pSwapChain->GetDepthImage(m_CurrentFrame),
			m_Camera,
			*m_pCurrentScene
		);
		
		m_pLightingPass->Record(
			commandBuffer,
			m_CurrentFrame,
			m_Camera,
			*m_pCurrentScene
		);
		
		m_pBlitPass->Record(
			commandBuffer,
			m_CurrentFrame,
			m_Camera
		);

	}

	void Renderer::ResizePasses() const
	{
		m_pGeometryPass->Resize(m_pSwapChain->GetSwapChainExtent());
		m_pLightingPass->Resize(m_pSwapChain->GetSwapChainExtent(), *m_pGeometryPass);
		m_pBlitPass->Resize(m_pSwapChain->GetSwapChainExtent(), *m_pLightingPass);
	}

}
