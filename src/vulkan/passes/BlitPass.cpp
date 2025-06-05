#include "BlitPass.h"

#include "LightingPass.h"
#include "../utils/DebugLabel.h"

cat::BlitPass::BlitPass(Device& device, SwapChain& swapChain, uint32_t framesInFlight, LightingPass& lightingPass)
	: m_Device(device), m_FramesInFlight(framesInFlight), m_SwapChain(swapChain) , m_Extent(swapChain.GetSwapChainExtent()), m_LightingPass(lightingPass)
{
	CreateDescriptors();
	CreatePipeline();
}

cat::BlitPass::~BlitPass()
{
	delete m_pDescriptorPool;
	m_pDescriptorPool = nullptr;
	delete m_pDescriptorSetLayout;
	m_pDescriptorSetLayout = nullptr;
	delete m_pDescriptorSet;
	m_pDescriptorSet = nullptr;

	delete m_pPipeline;
	m_pPipeline = nullptr;
}

void cat::BlitPass::Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Camera& camera) const
{
	Image& swapchainImage = *m_SwapChain.GetSwapChainImage(imageIndex);

	// BEGIN RECORDING
	{
		ToneMappingUbo uboData = {
			.exposure = camera.GetSpecs().exposure,
			.gamma = camera.GetSpecs().gamma
		};
		m_pUniformBuffer->Update(imageIndex, uboData);


		// transitioning images
		//----------------------
		swapchainImage.TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			{
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_NONE,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			});

		m_LightingPass.GetLitImages()[imageIndex]->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			{
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_ACCESS_SHADER_READ_BIT
			});

		// Render Attachments
		//---------------------
		VkClearValue clearValue{};
		clearValue.color = { 0.0f, 1.0f, 0.0f, 1.0f };

		// Color Attachment
		std::vector<VkRenderingAttachmentInfoKHR> colorAttachments(1);

		colorAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachments[0].imageView = swapchainImage.GetImageView();
		colorAttachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments[0].clearValue = clearValue;

		// Render Info
		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea = { {0, 0}, m_Extent };
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		renderInfo.pColorAttachments = colorAttachments.data();
		vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

		DebugLabel::BeginCmdLabel(commandBuffer, "Blit Pass", glm::vec4(1.0f, 0.7f, 0.7f, 1));
	}

	// Drawing
	{

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

		m_pPipeline->Bind(commandBuffer);

		m_pDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex, 0);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);
		DebugLabel::EndCmdLabel(commandBuffer);

		// transitioning images
		//----------------------
		swapchainImage.TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			Image::BarrierInfo{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_NONE
			});
	}
}

void cat::BlitPass::CreateDescriptors()
{
	m_pUniformBuffer = std::make_unique<UniformBuffer<ToneMappingUbo>>(m_Device, m_FramesInFlight);


	m_pDescriptorPool = new DescriptorPool(m_Device);
	m_pDescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_FramesInFlight )
		->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_FramesInFlight)
		->Create(m_FramesInFlight);

	m_pDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->Create();

	m_pDescriptorSet = new DescriptorSet(m_Device, *m_pDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
	for (int i{}; i < m_pDescriptorSet->GetDescriptorSetCount();i++)
	{
		m_pDescriptorSet
			->AddImageWrite(0, m_LightingPass.GetLitImages()[i]->GetImageInfo(), i) //Lit image
			->AddBufferWrite(1, m_pUniformBuffer->GetDescriptorBufferInfos(), i)
			->UpdateByIdx(i);
	}
}

void cat::BlitPass::CreatePipeline()
{
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();

	// attachments
	pipelineInfo.colorAttachments = {
		*m_SwapChain.GetSwapChainImageFormat()
	};
	pipelineInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
	pipelineInfo.colorBlendAttachments.resize(pipelineInfo.colorAttachments.size(),
		VkPipelineColorBlendAttachmentState{ .blendEnable = VK_FALSE, .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT }
	);
	pipelineInfo.colorBlending.pAttachments = pipelineInfo.colorBlendAttachments.data();
	pipelineInfo.colorBlending.attachmentCount = static_cast<uint32_t>(pipelineInfo.colorBlendAttachments.size());

	pipelineInfo.vertexBindingDescriptions = {};
	pipelineInfo.vertexAttributeDescriptions = {};

	pipelineInfo.CreatePipelineLayout(m_Device, { m_pDescriptorSetLayout->GetDescriptorSetLayout() });

	m_pPipeline = new Pipeline(m_Device, m_VertPath, m_FragPath, pipelineInfo);
}

void cat::BlitPass::Resize(VkExtent2D size, const LightingPass& lightingPass)
{
	m_Extent = size;
	for (size_t i{ 0 }; i < m_FramesInFlight; i++) 
	{
		VkDescriptorImageInfo imageInfo = lightingPass.GetLitImages()[i]->GetImageInfo();

		m_pDescriptorSet->ClearDescriptorWrites();
		m_pDescriptorSet
			->AddImageWrite(0, m_LightingPass.GetLitImages()[i]->GetImageInfo(), i) //Lit image
			->UpdateByIdx(i);
	}
}