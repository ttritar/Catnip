#include "ForwardPass.h"

#include "../utils/DebugLabel.h"
#include <iostream>

cat::ForwardPass::ForwardPass(Device& device, SwapChain& swapchain, uint32_t framesInFlight, LightingPass& lightingPass)
	: m_Device(device), m_FramesInFlight(framesInFlight), m_SwapChain(swapchain), m_Extent(swapchain.GetSwapChainExtent()), m_LightingPass(lightingPass)
{
	CreateBuffers();
	CreateDescriptors();
	CreatePipeline();
}

cat::ForwardPass::~ForwardPass()
{
	delete m_pPipeline;
	m_pPipeline = nullptr;

	delete m_pUboDescriptorSet;
	m_pUboDescriptorSet = nullptr;
	delete m_pDescriptorSet;
	m_pDescriptorSet = nullptr;
	delete m_pDescriptorPool;
	m_pDescriptorPool = nullptr;
	delete m_pUboDescriptorSetLayout;
	m_pUboDescriptorSetLayout = nullptr;
	delete m_pDescriptorSetLayout;
	m_pDescriptorSetLayout = nullptr;
}

void cat::ForwardPass::Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Scene& scene,
								Camera camera) const
{
	Image& litImage = *m_LightingPass.GetLitImages()[imageIndex];

	// BEGIN RECORDING
	{
		MatrixUbo matrixUbo = {
			.view = camera.GetView(),
			.proj = camera.GetProjection()
		};
		m_pUniformBuffer->Update(imageIndex, matrixUbo);



		// transitioning images
		//----------------------

		litImage.TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			{
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			});



		// Render Attachments
		//---------------------

		// Color Attachment
		VkRenderingAttachmentInfoKHR colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachment.imageView = litImage.GetImageView();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingAttachmentInfoKHR depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachment.imageView = m_SwapChain.GetDepthImage(imageIndex)->GetImageView();
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;


		// Render Info
		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea = { {0, 0}, m_Extent };
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;
		renderInfo.pDepthAttachment = &depthAttachment;

		assert(litImage.GetImageView() != VK_NULL_HANDLE);
		assert(m_SwapChain.GetDepthImage(imageIndex)->GetImageView() != VK_NULL_HANDLE);

		DebugLabel::BeginCmdLabel(commandBuffer, "Forward Pass", glm::vec4(0.3f, 0.05f, 0.2f, 1));
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

		m_pUboDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex, 0); 
		m_pDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex, 1);
		std::cout << "Transparent mesh count: " << scene.GetTransparentMeshes(camera.GetOrigin()).size() << std::endl;

		for (auto& mesh : scene.GetTransparentMeshes(camera.GetOrigin())) {
			vkCmdPushConstants(commandBuffer, m_pPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mesh->GetTransform());
			mesh->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex,false);
			mesh->Draw(commandBuffer);
		}

	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);
		DebugLabel::EndCmdLabel(commandBuffer);

		// transitioning images
		//----------------------
	}

}

void cat::ForwardPass::Resize(VkExtent2D size)
{
	m_Extent = size;
}

void cat::ForwardPass::CreateBuffers()
{
	m_pUniformBuffer = std::make_unique<UniformBuffer<MatrixUbo>>(m_Device);
}

void cat::ForwardPass::CreateDescriptors()
{
	m_pUboDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pUboDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		->Create();

	m_pDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->Create();

	m_pDescriptorPool = new DescriptorPool(m_Device);
	m_pDescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_FramesInFlight * 4 )
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_FramesInFlight * 4 )
		->Create(m_FramesInFlight * 2);

	m_pUboDescriptorSet = new DescriptorSet(m_Device, *m_pUboDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
	m_pUboDescriptorSet->AddBufferWrite(0,m_pUniformBuffer->GetDescriptorBufferInfos())
		->UpdateAll();

	m_pDescriptorSet = new DescriptorSet(m_Device, *m_pDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
	for (int i = 0; i < m_pDescriptorSet->GetDescriptorSetCount(); ++i)
	{
		m_pDescriptorSet
			->AddImageWrite(0, m_LightingPass.GetLitImages()[i]->GetImageInfo(), i)
			->UpdateByIdx(i);
	}
}
 
void cat::ForwardPass::CreatePipeline()
{
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();

	// attachments
	pipelineInfo.colorAttachments = { m_LightingPass.GetLitImages()[0]->GetFormat()};
	pipelineInfo.depthAttachmentFormat = m_SwapChain.GetDepthImage(0)->GetFormat();
	
	// Enable alpha blending
	VkPipelineColorBlendAttachmentState blend{};
	blend.blendEnable = VK_TRUE;
	blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend.colorBlendOp = VK_BLEND_OP_ADD;
	blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blend.alphaBlendOp = VK_BLEND_OP_ADD;
	blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	pipelineInfo.colorBlendAttachments.resize(pipelineInfo.colorAttachments.size(), blend);
	pipelineInfo.colorBlending.attachmentCount = static_cast<uint32_t>(pipelineInfo.colorBlendAttachments.size());
	pipelineInfo.colorBlending.pAttachments = pipelineInfo.colorBlendAttachments.data();

	pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;
	pipelineInfo.depthStencil.depthWriteEnable = VK_FALSE;

	pipelineInfo.CreatePipelineLayout(m_Device, {
		m_pUboDescriptorSetLayout->GetDescriptorSetLayout(),
		m_pDescriptorSetLayout->GetDescriptorSetLayout()
		});

	m_pPipeline = new Pipeline(
		m_Device,
		m_VertPath,
		m_FragPath,
		pipelineInfo
	);
}