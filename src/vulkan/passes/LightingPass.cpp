#include "LightingPass.h"
#include "../utils/DebugLabel.h"

cat::LightingPass::LightingPass(Device& device, VkExtent2D extent, uint32_t framesInFlight,const GeometryPass& geometryPass)
	: m_Device(device), m_FramesInFlight(framesInFlight), m_Extent(extent), m_GeometryPass(geometryPass)
{
	// IMAGES
	m_pLitImages.resize(m_FramesInFlight);
	for (int index{ 0 }; index < m_FramesInFlight; ++index){
		m_pLitImages[index] = std::make_unique<Image>(
			m_Device,
			extent.width, extent.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VMA_MEMORY_USAGE_AUTO
		);
		DebugLabel::NameImage(m_pLitImages[index]->GetImage(), std::string("Lit buffer <3.") + std::to_string(index));
	}

	CreateUniformBuffers();
	CreateDescriptors();
	CreatePipeline();
}

cat::LightingPass::~LightingPass()
{
	delete m_pDescriptorPool;
	m_pDescriptorPool = nullptr;
	delete m_pUboDescriptorSetLayout;
	m_pUboDescriptorSetLayout = nullptr;
	delete m_pSamplersDescriptorSetLayout;
	m_pSamplersDescriptorSetLayout = nullptr;
	delete m_pUboDescriptorSet;
	m_pUboDescriptorSet = nullptr;

	delete m_pPipeline;
	m_pPipeline = nullptr;
}

void cat::LightingPass::Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, Camera camera, Scene& scene) const
{

	Image& m_pLitImage = *m_pLitImages[imageIndex];
	// BEGIN RECORDING
	{
		if (scene.GetLights().empty()) return;

		LightingUbo uboData = {
			.lightDirection = scene.GetLights()[0].direction,
			.lightColor = scene.GetLights()[0].color,
			.lightIntensity = scene.GetLights()[0].intensity,
			.cameraPosition = camera.GetOrigin()
		};
		m_pUniformBuffer->Update(imageIndex,uboData);

		// transitioning images
		//----------------------
		m_pLitImage.TransitionImageLayout(commandBuffer,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_NONE,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			});


		// Render Attachments
		//---------------------
		VkClearValue clearValue {};
		clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

		// Color Attachment
		std::vector<VkRenderingAttachmentInfoKHR> colorAttachments;
		colorAttachments.resize(1);

		colorAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachments[0].imageView = m_pLitImage.GetImageView();
		colorAttachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments[0].clearValue = clearValue;

		// Render Info
		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea = { {0, 0}, m_Extent };
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = colorAttachments.size();
		renderInfo.pColorAttachments = colorAttachments.data();
		vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

		DebugLabel::BeginCmdLabel(commandBuffer, "Lighting Pass", glm::vec4(0.1f, 0.3f, 0.05f, 1));
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
		m_pSamplersDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), imageIndex, 1);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);
		DebugLabel::EndCmdLabel(commandBuffer);

		// transitioning images
		//----------------------
		m_pLitImage.TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			Image::BarrierInfo{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT
			});
	}
}

void cat::LightingPass::CreateUniformBuffers()
{
	m_pUniformBuffer = std::make_unique<UniformBuffer<LightingUbo>>(m_Device);
}

void cat::LightingPass::CreateDescriptors()
{
	m_pDescriptorPool = new DescriptorPool(m_Device);
	m_pDescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_FramesInFlight * 2 )
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_FramesInFlight * 10)
		->Create(m_FramesInFlight * 2);



	m_pUboDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pUboDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->Create();


	m_pUboDescriptorSet = new DescriptorSet(m_Device, *m_pUboDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
	m_pUboDescriptorSet
		->AddBufferWrite(0, m_pUniformBuffer->GetDescriptorBufferInfos()) // uniform buffer
		->UpdateAll();

	m_pSamplersDescriptorSetLayout = new DescriptorSetLayout(m_Device);
	m_pSamplersDescriptorSetLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->Create();


	m_pSamplersDescriptorSet = new DescriptorSet(m_Device, *m_pSamplersDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);

	for (int i = 0; i < m_pSamplersDescriptorSet->GetDescriptorSetCount(); ++i)
	{
		m_pSamplersDescriptorSet
			->AddImageWrite(0, m_GeometryPass.GetAlbedoBuffer(i).GetImageInfo(), i) // albedo
			->AddImageWrite(1, m_GeometryPass.GetNormalBuffer(i).GetImageInfo(), i) // normal
			->AddImageWrite(2, m_GeometryPass.GetSpecularBuffer(i).GetImageInfo(), i) // specular
			->AddImageWrite(3, m_GeometryPass.GetWorldBuffer(i).GetImageInfo(), i) // world
			->UpdateByIdx(i);
	}
	//m_pSamplersDescriptorSet->UpdateAll();
}

void cat::LightingPass::CreatePipeline()
{
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();

	// attachments
	pipelineInfo.colorAttachments = {
		m_pLitImages[0]->GetFormat()
	};
	pipelineInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

	pipelineInfo.colorBlendAttachments.resize(pipelineInfo.colorAttachments.size(),
		VkPipelineColorBlendAttachmentState{ .blendEnable = VK_FALSE, .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT }
	);
	pipelineInfo.colorBlending.pAttachments = pipelineInfo.colorBlendAttachments.data();
	pipelineInfo.colorBlending.attachmentCount = static_cast<uint32_t>(pipelineInfo.colorBlendAttachments.size());

	pipelineInfo.vertexAttributeDescriptions = {};
	pipelineInfo.vertexBindingDescriptions = {};

	pipelineInfo.CreatePipelineLayout(m_Device, { m_pUboDescriptorSetLayout->GetDescriptorSetLayout(), m_pSamplersDescriptorSetLayout->GetDescriptorSetLayout()});

	m_pPipeline = new Pipeline(
		m_Device,
		m_VertPath,
		m_FragPath,
		pipelineInfo
	);
}


void cat::LightingPass::Resize(VkExtent2D size)
{
	m_Extent = size;

	VkFormat litFormat = m_pLitImages[0]->GetFormat();

	m_pLitImages.clear();
	m_pLitImages.resize(m_FramesInFlight);

	for (uint32_t i = 0 ; i < m_FramesInFlight; ++i)
	{
		m_pLitImages[i] = std::make_unique<Image>(
			m_Device,
			m_Extent.width, m_Extent.height,
			litFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VMA_MEMORY_USAGE_AUTO
		);
		DebugLabel::NameImage(m_pLitImages[i]->GetImage(), std::string("Lit buffer <3.") + std::to_string(i));
	}	
}