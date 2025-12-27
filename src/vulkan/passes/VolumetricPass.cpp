#include "VolumetricPass.h"

#include "LightingPass.h"
#include "../utils/DebugLabel.h"

cat::VolumetricPass::VolumetricPass(Device& device, SwapChain& swapChain, uint32_t framesInFlight, LightingPass& lightingPass, ShadowPass& shadowPass)
	: m_Device(device), m_FramesInFlight(framesInFlight), m_SwapChain(swapChain), m_Extent(swapChain.GetSwapChainExtent()),
	m_ShadowPass(shadowPass), m_LightingPass(lightingPass)
{
	// IMAGES
	m_pVolumetricImages.resize(m_FramesInFlight);
	for (int index{ 0 }; index < m_FramesInFlight; ++index) {
		m_pVolumetricImages[index] = std::make_unique<Image>(
			m_Device,
			m_SwapChain.GetSwapChainExtent().width, m_SwapChain.GetSwapChainExtent().height,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VMA_MEMORY_USAGE_AUTO
		);
		DebugLabel::NameImage(m_pVolumetricImages[index]->GetImage(), std::string("Volumetric buffer <3.") + std::to_string(index));
	}

	CreateBuffers();
	CreateDescriptors();
	CreatePipeline();
}

cat::VolumetricPass::~VolumetricPass()
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

void cat::VolumetricPass::Record(VkCommandBuffer commandBuffer, uint32_t frameIndex, Camera camera, Scene& scene) const
{
	DebugLabel::Begin(commandBuffer, "Volumetric Pass", glm::vec4(0.4f, 0.0f, 0.8f, 1.0f));

	Image& volImage = *m_pVolumetricImages[frameIndex];

	// BEGIN RECORDING
	{
		VolumetricsUbo uboData = {
			.invViewProj = glm::inverse(camera.GetProjection() * camera.GetView()),
			.lightViewProj = scene.GetDirectionalLight().projectionMatrix * scene.GetDirectionalLight().viewMatrix,
			.camPos = camera.GetOrigin(),

			.lightDir = scene.GetDirectionalLight().direction,
			.lightColor = scene.GetDirectionalLight().color,
			.lightIntensity = scene.GetDirectionalLight().intensity,
			.volumetricDensity = 0.1f,

			.stepSize = 0.5f,
			.numSteps = 100,

			.rayStrength = 100.f,
			.rayDecay = 0.99f,
			.rayDensity = 5.f,
			.rayWeight = 0.8f

		};
		m_pUniformBuffer->Update(frameIndex, uboData);

		// transitioning images
		//----------------------
		volImage.TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			{
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_NONE,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			});

		m_ShadowPass.GetDepthImages()[frameIndex]->TransitionImageLayout(
			commandBuffer,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			{
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT
			}
		);

		m_LightingPass.GetLitImages()[frameIndex]->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			{
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT
			});

		m_SwapChain.GetDepthImage(frameIndex)->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT
			});


		// Render Attachments
		//---------------------
		VkClearValue clearValue{};
		clearValue.color = { 0.0f, 1.0f, 0.0f, 1.0f };

		// Color Attachment
		std::vector<VkRenderingAttachmentInfoKHR> colorAttachments(1);

		colorAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachments[0].imageView = volImage.GetImageView();
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

		m_pDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), frameIndex, 0);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);

		// transitioning images
		//----------------------
		volImage.TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			Image::BarrierInfo{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT
			});


	}

	DebugLabel::End(commandBuffer);
}

void cat::VolumetricPass::CreateBuffers()
{
	m_pUniformBuffer = std::make_unique<UniformBuffer<VolumetricsUbo>>(m_Device, m_FramesInFlight);
}

void cat::VolumetricPass::CreateDescriptors()
{
	m_pDescriptorPool = new DescriptorPool(m_Device);
	m_pDescriptorPool->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 * m_FramesInFlight);
	m_pDescriptorPool->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 * m_FramesInFlight);
	m_pDescriptorPool->Create(m_FramesInFlight);

	m_pDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); // frame
	m_pDescriptorSetLayout->AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); // depth
	m_pDescriptorSetLayout->AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); // shadow map
	m_pDescriptorSetLayout->AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT); // buffer
	m_pDescriptorSetLayout->Create();

	m_pDescriptorSet = new DescriptorSet(m_Device, *m_pDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
	for (int i{}; i < m_pDescriptorSet->GetDescriptorSetCount();i++)
	{
		m_pDescriptorSet
			->AddImageWrite(0, m_LightingPass.GetLitImages()[i]->GetImageInfo(), i) // scene
			->AddImageWrite(1, m_SwapChain.GetDepthImage(i)->GetImageInfo(), i) // depth
			->AddImageWrite(2, m_ShadowPass.GetDepthImages()[i]->GetImageInfo(), i) // shadow map)
			->AddBufferWrite(3, m_pUniformBuffer->GetDescriptorBufferInfos(), i) // buffer 
			->UpdateByIdx(i);
	}
}

void cat::VolumetricPass::CreatePipeline()
{
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();

	// attachments
	pipelineInfo.depthStencil.depthTestEnable = VK_FALSE;
	pipelineInfo.colorAttachments = {
		VK_FORMAT_R32G32B32A32_SFLOAT
	};
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

void cat::VolumetricPass::Resize(VkExtent2D size)
{
	m_Extent = size;
	for (size_t i{ 0 }; i < m_FramesInFlight; i++)
	{
		m_pDescriptorSet->ClearDescriptorWrites();
		m_pDescriptorSet
			->AddImageWrite(0, m_LightingPass.GetLitImages()[i]->GetImageInfo(), i) //Lit image
			->AddImageWrite(1, m_SwapChain.GetDepthImage(i)->GetImageInfo(), i) // depth
			->AddImageWrite(2, m_ShadowPass.GetDepthImages()[i]->GetImageInfo(), i) // shadow map
			->AddBufferWrite(3, m_pUniformBuffer->GetDescriptorBufferInfos(), i)
			->UpdateByIdx(i);
	}
}