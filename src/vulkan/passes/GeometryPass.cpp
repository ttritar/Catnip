#include "GeometryPass.h"

#include "../utils/DebugLabel.h"

cat::GeometryPass::GeometryPass(Device& device, VkExtent2D extent, uint32_t framesInFlight)
	: m_Device(device), m_Extent(extent), m_FramesInFlight(framesInFlight)
{
	// IMAGES
	m_pAlbedoBuffer = std::make_unique<Image>(
		m_Device,
		extent.width, extent.height,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VMA_MEMORY_USAGE_AUTO
	);
	DebugLabel::NameImage(m_pAlbedoBuffer->GetImage(), "Albedo buffer <3");

	m_pNormalBuffer = std::make_unique<Image>(
		m_Device,
		extent.width, extent.height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VMA_MEMORY_USAGE_AUTO
	);
	DebugLabel::NameImage(m_pNormalBuffer->GetImage(), "Normal buffer <3");

	m_pSpecularBuffer = std::make_unique<Image>(
		m_Device,
		extent.width, extent.height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VMA_MEMORY_USAGE_AUTO
	);
	DebugLabel::NameImage(m_pSpecularBuffer->GetImage(), "Specular buffer <3");

	m_pWorldBuffer = std::make_unique<Image>(
		m_Device,
		extent.width, extent.height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VMA_MEMORY_USAGE_AUTO
	);
	DebugLabel::NameImage(m_pWorldBuffer->GetImage(), "World buffer <3");

	// CREATE
	CreateUniformBuffers();
	CreateDescriptors();
	CreatePipeline();	
}

cat::GeometryPass::~GeometryPass()
{
	delete m_pDescriptorPool;
	m_pDescriptorPool = nullptr;
	delete m_pUboDescriptorSetLayout;
	m_pUboDescriptorSetLayout = nullptr;
	delete m_pSamplersDescriptorSetLayout;
	m_pSamplersDescriptorSetLayout = nullptr;
	delete m_pDescriptorSet;
	m_pDescriptorSet = nullptr;

	delete m_pPipeline;
	m_pPipeline = nullptr;
}

void cat::GeometryPass::Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, 
	Image& depthImage,
	Camera camera, Scene& scene) const
{
	// BEGIN RECORDING
	{
		m_pUniformBuffer->Update(imageIndex, camera.GetView(), camera.GetProjection());

		// transitioning images
		//----------------------
		depthImage.TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);

		m_pAlbedoBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		m_pNormalBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		m_pSpecularBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		m_pWorldBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);



		// Render Attachments
		//---------------------
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// Color Attachment
		std::vector<VkRenderingAttachmentInfoKHR> colorAttachments;
		colorAttachments.resize(4);

		colorAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachments[0].imageView = m_pAlbedoBuffer->GetImageView();
		colorAttachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments[0].clearValue = clearValues[0];

		colorAttachments[1].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachments[1].imageView = m_pNormalBuffer->GetImageView();
		colorAttachments[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments[1].clearValue = clearValues[0];

		colorAttachments[2].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachments[2].imageView = m_pSpecularBuffer->GetImageView();
		colorAttachments[2].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments[2].clearValue = clearValues[0];

		colorAttachments[3].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachments[3].imageView = m_pWorldBuffer->GetImageView();
		colorAttachments[3].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments[3].clearValue = clearValues[0];

		// Depth Attachment
		VkRenderingAttachmentInfoKHR depthAttachmentInfo{};
		depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachmentInfo.imageView = depthImage.GetImageView();
		depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachmentInfo.clearValue = clearValues[1];

		// Render Info
		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea = { {0, 0}, m_Extent};
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = colorAttachments.size();
		renderInfo.pColorAttachments = colorAttachments.data();
		renderInfo.pDepthAttachment = &depthAttachmentInfo;
		vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);
	}

	// Drawing
	{
		m_pPipeline->Bind(commandBuffer);

		// viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Extent.width);
		viewport.height = static_cast<float>(m_Extent.height);
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_Extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		m_pDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex, 0);

		// draw the scene
		scene.Draw(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex);
	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);

		// transitioning images
		//----------------------
		depthImage.TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);

		m_pAlbedoBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		m_pNormalBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		m_pSpecularBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		m_pWorldBuffer->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
	}
}

void cat::GeometryPass::CreateUniformBuffers()
{
	m_pUniformBuffer = std::make_unique<UniformBuffer>(m_Device);
}

void cat::GeometryPass::CreateDescriptors()
{
	m_pDescriptorPool = new DescriptorPool(m_Device);
	m_pDescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2)
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3)
		->Create(m_FramesInFlight);


	
	m_pUboDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pUboDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		->Create();

	m_pSamplersDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pSamplersDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->Create();


	m_pDescriptorSet = new DescriptorSet(m_Device, *m_pUboDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
	m_pDescriptorSet
		->AddBufferWrite(0, m_pUniformBuffer->GetDescriptorBufferInfos()) // uniform buffer
		->Update();
}

void cat::GeometryPass::CreatePipeline()
{
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();

	// attachments
	pipelineInfo.colorAttachments = {
		m_pAlbedoBuffer->GetFormat(),
		m_pNormalBuffer->GetFormat(),
		m_pSpecularBuffer->GetFormat(),
		m_pWorldBuffer->GetFormat()
	};
	pipelineInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

	pipelineInfo.colorBlendAttachments.resize(pipelineInfo.colorAttachments.size(),
		VkPipelineColorBlendAttachmentState{ .blendEnable = VK_FALSE, .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT }
	);
	pipelineInfo.colorBlending.pAttachments = pipelineInfo.colorBlendAttachments.data();
	pipelineInfo.colorBlending.attachmentCount = static_cast<uint32_t>(pipelineInfo.colorBlendAttachments.size());

	pipelineInfo.CreatePipelineLayout(m_Device, {m_pUboDescriptorSetLayout->GetDescriptorSetLayout(),m_pSamplersDescriptorSetLayout->GetDescriptorSetLayout() });

	m_pPipeline = new Pipeline(
		m_Device,
		m_VertPath,
		m_FragPath,
		pipelineInfo
	);
}

