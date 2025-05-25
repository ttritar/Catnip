#include "DepthPrepass.h"

cat::DepthPrepass::DepthPrepass(Device& device, uint32_t framesInFlight)
	: m_Device(device), m_FramesInFlight(framesInFlight)
{
	CreateUniformBuffers();
	CreateDescriptors();
	CreatePipeline();
}

cat::DepthPrepass::~DepthPrepass()
{
	delete m_DescriptorSet;
	delete m_DescriptorSetLayout;
	delete m_DescriptorPool;

	delete m_Pipeline;
}

void cat::DepthPrepass::Record(VkCommandBuffer commandBuffer, uint32_t imageIndex,
	Image& depthImage, Camera camera, Scene& scene) const
{
	// BEGIN RECORDING
	{
		m_UniformBuffer->Update(imageIndex, camera.GetView(), camera.GetProjection());

		depthImage.TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);

		// Render Attachments
		VkRenderingAttachmentInfoKHR depthAttachmentInfo{};
		depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachmentInfo.imageView = depthImage.GetImageView();
		depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachmentInfo.clearValue.depthStencil = { 1.0f, 0 };

		// Render Info
		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea = { {0, 0}, depthImage.GetExtent() };
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 0;
		renderInfo.pColorAttachments = nullptr;
		renderInfo.pDepthAttachment = &depthAttachmentInfo;
		vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);
	}

	// Drawing
	{
		m_Pipeline->Bind(commandBuffer);

		// viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(depthImage.GetExtent().width);
		viewport.height = static_cast<float>(depthImage.GetExtent().height);
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = depthImage.GetExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		m_DescriptorSet->Bind(commandBuffer, m_Pipeline->GetPipelineLayout(), imageIndex);

		// draw the scene
		scene.Draw(commandBuffer, m_Pipeline->GetPipelineLayout(), imageIndex, true);
	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);
		depthImage.TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
	}
}


void cat::DepthPrepass::CreateUniformBuffers()
{
	m_UniformBuffer = std::make_unique<UniformBuffer>(m_Device);
}

void cat::DepthPrepass::CreateDescriptors()
{
	m_DescriptorPool = new DescriptorPool(m_Device);
	m_DescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2)
		->Create(m_FramesInFlight);

	m_DescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_DescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		->Create();

	m_DescriptorSet = new DescriptorSet(
		m_Device,
		*m_UniformBuffer,
		{},
		*m_DescriptorSetLayout,
		*m_DescriptorPool
	);
}

void cat::DepthPrepass::CreatePipeline()
{
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();
	pipelineInfo.colorAttachments = { };
	pipelineInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	pipelineInfo.CreatePipelineLayout(m_Device, { m_DescriptorSetLayout->GetDescriptorSetLayout() });

	m_Pipeline = new Pipeline(
		m_Device,
		m_VertPath,
		m_FragPath,
		pipelineInfo
	);
}
