#include "ShadowPass.h"
#include "../utils/DebugLabel.h"

cat::ShadowPass::ShadowPass(Device& device, uint32_t framesInFlight, SwapChain& swapchain)
	: m_Device(device), m_FramesInFlight(framesInFlight)
{
	// IMAGES
	m_pDepthImages.resize(m_FramesInFlight);
	for (int index{ 0 }; index < m_FramesInFlight; ++index) {
		m_pDepthImages[index] = std::make_unique<Image>(
			m_Device,
			swapchain.GetSwapChainExtent().width, swapchain.GetSwapChainExtent().height,
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VMA_MEMORY_USAGE_AUTO
		);
		DebugLabel::NameImage(m_pDepthImages[index]->GetImage(), std::string("Depth Image - Directional light POV <") + std::to_string(index));
	}

	CreateUniformBuffers();
	CreateDescriptors();
	CreatePipeline();
}

cat::ShadowPass::~ShadowPass()
{
	delete m_pDescriptorSet;
	m_pDescriptorSet = nullptr;
	delete m_pDescriptorSetLayout;
	m_pDescriptorSetLayout = nullptr;
	delete m_pDescriptorPool;
	m_pDescriptorPool = nullptr;

	delete m_pPipeline;
}

void cat::ShadowPass::Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, Camera camera, Scene& scene) const
{
	// BEGIN RECORDING
	{
		ShadowUbo uboData = { scene.GetDirectionalLight().viewMatrix, scene.GetDirectionalLight().projectionMatrix };
		m_pUniformBuffer->Update(imageIndex, uboData);

		m_pDepthImages[imageIndex]->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			Image::BarrierInfo{
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	VK_ACCESS_NONE,
	VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			});

		// Render Attachments
		VkRenderingAttachmentInfoKHR depthAttachmentInfo{};
		depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachmentInfo.imageView = m_pDepthImages[imageIndex]->GetImageView();
		depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachmentInfo.clearValue.depthStencil = { 1.0f, 0 };

		// Render Info
		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea = { {0, 0}, m_pDepthImages[imageIndex]->GetExtent() };
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 0;
		renderInfo.pColorAttachments = nullptr;
		renderInfo.pDepthAttachment = &depthAttachmentInfo;
		vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);
		DebugLabel::BeginCmdLabel(commandBuffer, "Shadow Pass", glm::vec4(0.3f, 0.5f, 1.f, 1));
	}

	// Drawing
	{
		m_pPipeline->Bind(commandBuffer);

		// viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_pDepthImages[imageIndex]->GetExtent().width);
		viewport.height = static_cast<float>(m_pDepthImages[imageIndex]->GetExtent().height);
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_pDepthImages[imageIndex]->GetExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		m_pDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex);

		// draw the scene
		scene.DrawOpaque(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex, true);
	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);
		DebugLabel::EndCmdLabel(commandBuffer);
		m_pDepthImages[imageIndex]->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			Image::BarrierInfo{
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			});
	}
}

void cat::ShadowPass::CreateUniformBuffers()
{
	m_pUniformBuffer = std::make_unique<UniformBuffer<ShadowUbo>>(m_Device);
}

void cat::ShadowPass::CreateDescriptors()
{
	m_pDescriptorPool = new DescriptorPool(m_Device);
	m_pDescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2)
		->Create(m_FramesInFlight);

	m_pDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		->Create();

	m_pDescriptorSet = new DescriptorSet(m_Device, *m_pDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
	m_pDescriptorSet
		->AddBufferWrite(0, m_pUniformBuffer->GetDescriptorBufferInfos())
		->UpdateAll();
}

void cat::ShadowPass::CreatePipeline()
{
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();
	pipelineInfo.colorAttachments = { };
	pipelineInfo.colorBlending.attachmentCount = 0;
	pipelineInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	pipelineInfo.CreatePipelineLayout(m_Device, { m_pDescriptorSetLayout->GetDescriptorSetLayout() });

	m_pPipeline = new Pipeline(
		m_Device,
		m_VertPath,
		m_FragPath,
		pipelineInfo
	);
}
