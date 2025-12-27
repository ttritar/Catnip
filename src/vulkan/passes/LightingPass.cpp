#include "LightingPass.h"

#include "ShadowPass.h"
#include "../utils/DebugLabel.h"

cat::LightingPass::LightingPass(Device& device, VkExtent2D extent, uint32_t framesInFlight, const GeometryPass& geometryPass, HDRImage* pSkyBoxImage, SwapChain& swapchain, const ShadowPass& shadowPass)
	: m_Device(device), m_FramesInFlight(framesInFlight), m_Extent(extent), m_GeometryPass(geometryPass), m_pSkyBoxImage(pSkyBoxImage), m_SwapChain(swapchain), m_ShadowPass(shadowPass)
{
	// IMAGES
	m_pLitImages.resize(m_FramesInFlight);
	for (int index{ 0 }; index < m_FramesInFlight; ++index){
		m_pLitImages[index] = std::make_unique<Image>(
			m_Device,
			extent.width, extent.height,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VMA_MEMORY_USAGE_AUTO
		);
		DebugLabel::NameImage(m_pLitImages[index]->GetImage(), std::string("Lit buffer <3.") + std::to_string(index));
	}

	CreateBuffers();
	CreateDescriptors();
	CreatePipeline();
}

cat::LightingPass::~LightingPass()
{
	delete m_pPipeline;
	m_pPipeline = nullptr;
}

void cat::LightingPass::Record(VkCommandBuffer commandBuffer, uint32_t frameIndex, Camera camera, Scene& scene) const
{
	Image& litImage = *m_pLitImages[frameIndex];
	// BEGIN RECORDING
	{
		LightingUbo uboData = {
			.lightDirection = scene.GetDirectionalLight().direction,
			.lightColor = scene.GetDirectionalLight().color,
			.lightIntensity = scene.GetDirectionalLight().intensity,
			.lightViewProj = scene.GetDirectionalLight().projectionMatrix * scene.GetDirectionalLight().viewMatrix,

			.cameraPosition = glm::vec4(camera.GetOrigin(),1.0),
			.proj = camera.GetProjection(),
			.view = camera.GetView(),
			.viewportSize = { static_cast<float>(m_Extent.width), static_cast<float>(m_Extent.height) },

			.pointLightCount = static_cast<uint32_t>(scene.GetPointLights().size())
		};
		m_pUniformBuffer->Update(frameIndex,uboData);

		std::array<Scene::PointLight, LightingPass::MAX_POINT_LIGHTS> pointLights{};
		const auto& sceneLights = scene.GetPointLights();
		for (size_t i = 0; i < sceneLights.size() && i < LightingPass::MAX_POINT_LIGHTS; ++i) {
			pointLights[i] = sceneLights[i];
		}
		m_pPointLightingStorageBuffer->Update(frameIndex, pointLights);



		// transitioning images
		//----------------------
		m_SwapChain.GetDepthImage(frameIndex)->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			Image::BarrierInfo{
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT
			});

		litImage.TransitionImageLayout(commandBuffer,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
		colorAttachments[0].imageView = litImage.GetImageView();
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
		DebugLabel::Begin(commandBuffer, "Lighting Pass", glm::vec4(0.1f, 0.3f, 0.05f, 1));
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
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_Extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		m_pUboDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), frameIndex, 0);
		m_pSamplersDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), frameIndex, 1);
		m_pHDRISamplersDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), frameIndex, 2);
		m_pShadowDescriptorSet->Bind(commandBuffer, m_pPipeline->GetPipelineLayout(), frameIndex, 3);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	}

	// END RECORDING
	{
		vkCmdEndRenderingKHR(commandBuffer);
		DebugLabel::End(commandBuffer);

		// transitioning images
		//----------------------
		m_SwapChain.GetDepthImage(frameIndex)->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			Image::BarrierInfo{
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_SHADER_READ_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			});

		litImage.TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			Image::BarrierInfo{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT
			});
	}
}

void cat::LightingPass::CreateBuffers()
{
	m_pUniformBuffer = std::make_unique<UniformBuffer<LightingUbo>>(m_Device);
	m_pPointLightingStorageBuffer = std::make_unique<StorageBuffer<Scene::PointLight, MAX_POINT_LIGHTS>>(m_Device);
}

void cat::LightingPass::CreateDescriptors()
{
	m_pDescriptorPool = std::make_unique<DescriptorPool>(m_Device);
	m_pDescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_FramesInFlight)
		->AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_FramesInFlight)
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_FramesInFlight * 5)
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_FramesInFlight * 2)
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_FramesInFlight)
		->Create(m_FramesInFlight * 4);

	// UBO 
	{
		m_pUboDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(m_Device);
		m_pUboDescriptorSetLayout
			->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->Create();


		m_pUboDescriptorSet = std::make_unique<DescriptorSet>(m_Device, *m_pUboDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
		m_pUboDescriptorSet
			->AddBufferWrite(0, m_pUniformBuffer->GetDescriptorBufferInfos())
			->AddBufferWrite(1, m_pPointLightingStorageBuffer->GetDescriptorBufferInfos())
			->UpdateAll();
	}

	// SAMPLERS
	{
		m_pSamplersDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(m_Device);
		m_pSamplersDescriptorSetLayout
			->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->Create();

		m_pSamplersDescriptorSet = std::make_unique<DescriptorSet>(m_Device, *m_pSamplersDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);

		for (int i = 0; i < m_pSamplersDescriptorSet->GetDescriptorSetCount(); ++i)
		{

			m_pSamplersDescriptorSet
				->AddImageWrite(0, m_GeometryPass.GetAlbedoBuffer(i).GetImageInfo(), i) // albedo
				->AddImageWrite(1, m_GeometryPass.GetNormalBuffer(i).GetImageInfo(), i) // normal
				->AddImageWrite(2, m_GeometryPass.GetSpecularBuffer(i).GetImageInfo(), i) // specular
				->AddImageWrite(3, m_GeometryPass.GetWorldBuffer(i).GetImageInfo(), i) // world
				->AddImageWrite(4, m_SwapChain.GetDepthImage(i)->GetImageInfo(), i) // depth
				->UpdateByIdx(i);
		}
	}

	// HDRI 
	{
		m_pHDRISamplersDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(m_Device);
		m_pHDRISamplersDescriptorSetLayout
			->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->Create();


		VkDescriptorImageInfo cubemapInfo{};
		cubemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		cubemapInfo.imageView = m_pSkyBoxImage->GetCubeMapImageView();
		cubemapInfo.sampler = m_pSkyBoxImage->GetCubeMapSampler(); 

		VkDescriptorImageInfo irradianceInfo{};
		irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		irradianceInfo.imageView = m_pSkyBoxImage->GetIrradianceMapImageView();
		irradianceInfo.sampler = m_pSkyBoxImage->GetIrradianceMapSampler(); 

		m_pHDRISamplersDescriptorSet = std::make_unique<DescriptorSet>(m_Device, *m_pHDRISamplersDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
		for (int i = 0; i < m_pHDRISamplersDescriptorSet->GetDescriptorSetCount(); ++i)
		{
			m_pHDRISamplersDescriptorSet
				->AddImageWrite(0, cubemapInfo, i) // skybox 
				->AddImageWrite(1, irradianceInfo, i) // irradiance 
				->UpdateByIdx(i);
		}
	}

	// SHADOWS
	{
		m_pShadowDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(m_Device);
		m_pShadowDescriptorSetLayout
			->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			->Create();
		m_pShadowDescriptorSet = std::make_unique<DescriptorSet>(m_Device, *m_pShadowDescriptorSetLayout, *m_pDescriptorPool, m_FramesInFlight);
		for (int i = 0; i < m_pShadowDescriptorSet->GetDescriptorSetCount(); ++i)
		{
			m_pShadowDescriptorSet
				->AddImageWrite(0, m_ShadowPass.GetDepthImages()[i]->GetImageInfo(), i) // shadow map
				->UpdateByIdx(i);
		}
	}
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

	pipelineInfo.CreatePipelineLayout(m_Device, { 
		m_pUboDescriptorSetLayout->GetDescriptorSetLayout(),
		m_pSamplersDescriptorSetLayout->GetDescriptorSetLayout(),
		m_pHDRISamplersDescriptorSetLayout->GetDescriptorSetLayout(),
		m_pShadowDescriptorSetLayout->GetDescriptorSetLayout()
	});

	pipelineInfo.rasterizer.depthBiasEnable = VK_TRUE;
	pipelineInfo.rasterizer.depthBiasConstantFactor = 1.25f;
	pipelineInfo.rasterizer.depthBiasSlopeFactor = 1.75f;

	m_pPipeline = new Pipeline(
		m_Device,
		m_VertPath,
		m_FragPath,
		pipelineInfo
	);
}


void cat::LightingPass::Resize(VkExtent2D size, const GeometryPass& geometryPass)
{
	m_Extent = size;

	VkFormat litFormat = m_pLitImages[0]->GetFormat();

	m_pLitImages.clear();
	m_pLitImages.resize(m_FramesInFlight);

	for (uint32_t i = 0; i < m_FramesInFlight; ++i)
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

	for (size_t i{ 0 }; i < m_FramesInFlight; i++)
	{
		VkDescriptorImageInfo imageInfo = geometryPass.GetAlbedoBuffer(i).GetImageInfo();

		m_pSamplersDescriptorSet->ClearDescriptorWrites();
		m_pSamplersDescriptorSet
			->AddImageWrite(0, geometryPass.GetAlbedoBuffer(i).GetImageInfo(), i) // albedo
			->AddImageWrite(1, geometryPass.GetNormalBuffer(i).GetImageInfo(), i) // normal
			->AddImageWrite(2, geometryPass.GetSpecularBuffer(i).GetImageInfo(), i) // specular
			->AddImageWrite(3, geometryPass.GetWorldBuffer(i).GetImageInfo(), i) // world
			->AddImageWrite(4, m_SwapChain.GetDepthImage(i)->GetImageInfo(), i) // depth()
			->UpdateByIdx(i);



		VkDescriptorImageInfo cubemapInfo{};
		cubemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		cubemapInfo.imageView = m_pSkyBoxImage->GetCubeMapImageView();
		cubemapInfo.sampler = m_pSkyBoxImage->GetCubeMapSampler();

		VkDescriptorImageInfo irradianceInfo{};
		irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		irradianceInfo.imageView = m_pSkyBoxImage->GetIrradianceMapImageView();
		irradianceInfo.sampler = m_pSkyBoxImage->GetIrradianceMapSampler();

		m_pHDRISamplersDescriptorSet->ClearDescriptorWrites();
		m_pHDRISamplersDescriptorSet
			->AddImageWrite(0, cubemapInfo, i) // skybox
			->AddImageWrite(1, irradianceInfo, i) // irradiance
			->UpdateByIdx(i);


		m_pShadowDescriptorSet->ClearDescriptorWrites();
		m_pShadowDescriptorSet
			->AddImageWrite(0, m_ShadowPass.GetDepthImages()[i]->GetImageInfo(), i) // shadow map
			->UpdateByIdx(i);
	}
}
