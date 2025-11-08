#include "HDRImage.h"
#include "../buffers/Buffer.h"
#include "../utils/DebugLabel.h"

#include <stb_image.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <glm/gtx/hash.hpp>

// std
#include <array>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "../Pipeline.h"


cat::HDRImage::HDRImage(Device& device, const std::string& filename)
	: m_Device(device), m_CAPTURE_PROJECTION(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f))
{
	m_CAPTURE_PROJECTION[1][1] *= -1.0f; 


	// LOADING
	//----------
	if (!std::filesystem::exists(filename)) {
		throw std::runtime_error("File does not exist: " + filename);
	}
	if (!stbi_is_hdr(filename.c_str())) {
		throw std::runtime_error("File is not HDR format: " + filename);
	}

	int texWidth, texHeight, texChannels;
	float* pixels = stbi_loadf(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load HDR image: " + filename + " -> (" + stbi_failure_reason() + ")");
	}
	m_EquirectMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
	m_EquirectMipLevels = 1;
	m_EquirectExtent = VkExtent2D{ static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };


	VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(float);
	m_EquirectFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	Buffer stagingBuffer(m_Device,
		{ imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY });

	stagingBuffer.WriteToBuffer(pixels);
	stbi_image_free(pixels);

	// CREATING
	//----------
	CreateEquirectImage(texWidth, texHeight, m_EquirectMipLevels, m_EquirectFormat, 
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT , VMA_MEMORY_USAGE_GPU_ONLY);
	CreateEquirectTextureImageView();

	m_Device.TransitionImageLayout(m_EquirectImage, m_EquirectFormat,m_EquirectImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,m_EquirectMipLevels);
	device.CopyBufferToImage(stagingBuffer.GetBuffer(), m_EquirectImage, texWidth, texHeight);
	
	CreateEquirectTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	DebugLabel::NameImage(m_EquirectImage,"HDRI: " + filename);

	m_Device.TransitionImageLayout(m_EquirectImage, m_EquirectFormat, m_EquirectImageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_EquirectMipLevels);

	CreateCubeMap();
	CreateIrradianceMap();
}

cat::HDRImage::~HDRImage()
{
	// Destroy samplers
	if (m_EquirectSampler != VK_NULL_HANDLE) {
		vkDestroySampler(m_Device.GetDevice(), m_EquirectSampler, nullptr);
	}
	if (m_CubeMapSampler != VK_NULL_HANDLE) {
		vkDestroySampler(m_Device.GetDevice(), m_CubeMapSampler, nullptr);
	}
	if (m_IrradianceMapSampler != VK_NULL_HANDLE) {
		vkDestroySampler(m_Device.GetDevice(), m_IrradianceMapSampler, nullptr);
	}

	// Destroy image views
	if (m_EquirectImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(m_Device.GetDevice(), m_EquirectImageView, nullptr);
	}
	if (m_CubeMapImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(m_Device.GetDevice(), m_CubeMapImageView, nullptr);
	}
	if (m_IrradianceMapImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(m_Device.GetDevice(), m_IrradianceMapImageView, nullptr);
	}

	// Destroy cube map face views
	for (auto& faceViews : m_CubeMapFaceViews) {
		for (auto& view : faceViews) {
			vkDestroyImageView(m_Device.GetDevice(), view, nullptr);
		}
	}

	// Destroy irradiance map face views
	for (auto& view : m_IrradianceMapFaceViews) {
		if (view != VK_NULL_HANDLE) {
			vkDestroyImageView(m_Device.GetDevice(), view, nullptr);
		}
	}

	// Destroy images and allocations
	vmaDestroyImage(m_Device.GetAllocator(), m_EquirectImage, m_EquirectAllocation);
	vmaDestroyImage(m_Device.GetAllocator(), m_CubeMapImage, m_CubeMapAllocation);
	vmaDestroyImage(m_Device.GetAllocator(), m_IrradianceMapImage, m_IrradianceMapAllocation);
}



// BASE IMAGE CREATION
//---------------------
void cat::HDRImage::CreateEquirectImage(uint32_t width, uint32_t height, uint32_t miplevels, VkFormat format,
                                VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = miplevels;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.arrayLayers = 1;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = memoryUsage;

	if (vmaCreateImage(m_Device.GetAllocator(), &imageInfo, &allocInfo, &m_EquirectImage, &m_EquirectAllocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image with VMA!");
	}
}

void cat::HDRImage::CreateEquirectTextureImageView()
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_EquirectImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = m_EquirectFormat;
	viewInfo.subresourceRange.aspectMask = GetImageAspect(m_EquirectFormat);
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = m_EquirectMipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_EquirectImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image view!");
	}
}

void cat::HDRImage::CreateEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	samplerInfo.magFilter = filter;
	samplerInfo.minFilter = filter;
	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = m_Device.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;

	if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_EquirectSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler!");
	}
}

void cat::HDRImage::GenerateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLevels) const
{
	const VkFormatProperties properties = m_Device.GetFormatProperties(format);
	VkImageAspectFlags aspect = GetImageAspect(format);

	if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("Format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();


	// TRANSITION image to TRANSFER_DST_OPTIMAL
	{
		VkImageMemoryBarrier initialBarrier{};
		initialBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		initialBarrier.image = image;
		initialBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		initialBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		initialBarrier.subresourceRange = {
			aspect,
			0, mipLevels,  
			0, arrayLevels
		};
		initialBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		initialBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		initialBarrier.srcAccessMask = 0;
		initialBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &initialBarrier);
	}

	for (uint32_t faceIdx = 0; faceIdx < arrayLevels; ++faceIdx) 
	{
		auto mipWidth = static_cast<int32_t>(width);
		auto mipHeight = static_cast<int32_t>(height);

		for (uint32_t i = 1; i < mipLevels; i++) 
		{
			// TRANSITION previous mip level (i-1) to TRANSFER_SRC_OPTIMAL
			{
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.image = image;
				barrier.subresourceRange = {
					VK_IMAGE_ASPECT_COLOR_BIT,
					i - 1, 1,
					faceIdx, 1
				};
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

			// BLIT
			{
				VkImageBlit blit{};
				blit.srcSubresource = {
					VK_IMAGE_ASPECT_COLOR_BIT,
					i - 1,  
					faceIdx, 
					1     
				};

				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };

				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = {
					mipWidth > 1 ? mipWidth / 2 : 1,
					mipHeight > 1 ? mipHeight / 2 : 1,
					1
				};

				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = faceIdx;
				blit.dstSubresource.layerCount = 1;

				blit.srcSubresource.aspectMask = aspect;
				blit.dstSubresource.aspectMask = aspect;

				vkCmdBlitImage(commandBuffer,
					image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR);
			}


			// TRANSITION previous mip level (i-1) to SHADER_READ_ONLY_OPTIMAL
			{
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.image = image;
				barrier.subresourceRange = {
					VK_IMAGE_ASPECT_COLOR_BIT,
					i - 1, 1,
					faceIdx, 1
				};
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		// TRANSITION last mip level to SHADER_READ_ONLY_OPTIMAL
		{
			VkImageMemoryBarrier finalBarrier{};
			finalBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			finalBarrier.image = image;
			finalBarrier.subresourceRange = {
				VK_IMAGE_ASPECT_COLOR_BIT,
				mipLevels - 1, 1,  
				faceIdx, 1
			};
			finalBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			finalBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			finalBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			finalBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &finalBarrier);
		}
	}


	m_Device.EndSingleTimeCommands(commandBuffer);
}

VkImageAspectFlags cat::HDRImage::GetImageAspect(VkFormat format)
{
	switch (format) {
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

// EXTRA HDRI SHITS
//-------------------
void cat::HDRImage::CreateCubeMap()
{
	uint32_t cubeMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_CubeMapExtent.width, m_CubeMapExtent.height)))) + 1;
	cubeMipLevels = 1;

	// 1. Image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = m_CubeMapExtent.width;
	imageInfo.extent.height = m_CubeMapExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = cubeMipLevels;
	imageInfo.arrayLayers = m_FACE_COUNT;
	imageInfo.format = m_EquirectFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(m_Device.GetAllocator(), &imageInfo, &allocInfo,
		&m_CubeMapImage, &m_CubeMapAllocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create cubemap image!");
	}

	// 2. Image view 
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_CubeMapImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.format = m_EquirectFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = m_FACE_COUNT;

	if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_CubeMapImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create cubemap image view!");
	}

	// 3. Face views
	for (uint32_t face = 0; face < m_FACE_COUNT; ++face)
	{
		VkImageViewCreateInfo faceViewInfo{};
		faceViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		faceViewInfo.image = m_CubeMapImage;
		faceViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		faceViewInfo.format = m_EquirectFormat;
		faceViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		faceViewInfo.subresourceRange.baseMipLevel = 0;
		faceViewInfo.subresourceRange.levelCount = 1;
		faceViewInfo.subresourceRange.baseArrayLayer = face;
		faceViewInfo.subresourceRange.layerCount = 1;

		VkImageView view;
		if (vkCreateImageView(m_Device.GetDevice(), &faceViewInfo, nullptr, &view) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create cubemap face view!");
		}

		m_CubeMapFaceViews[face].push_back(view); 
	}

	// 4. Create sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = m_Device.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(cubeMipLevels);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_CubeMapSampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create cubemap texture sampler!");
	}

	RenderToCubeMap(m_CubeMapExtent, cubeMipLevels, m_CubeVertPath, m_SkyFragPath, 
		m_EquirectImage,m_EquirectImageView,
		m_EquirectSampler, m_CubeMapImage, m_CubeMapFaceViews);
	//GenerateMipmaps(m_CubeMapImage, m_EquirectFormat, m_CubeMapExtent.width, m_CubeMapExtent.height, cubeMipLevels , m_FACE_COUNT);
	DebugLabel::NameImage(m_CubeMapImage, "HDRI CubeMap: " + std::to_string(m_CubeMapExtent.width) + "x" + std::to_string(m_CubeMapExtent.height));
}


void cat::HDRImage::RenderToCubeMap(const VkExtent2D& extent,uint32_t mipLevels, const std::string& vertPath, const std::string& fragPath,
	VkImage& inputImage, const VkImageView& inputImageView, const VkSampler inputSampler,
	VkImage& outputCubeMapImage, std::array<std::vector<VkImageView>, m_FACE_COUNT>& outputCubeMapImageViews)
{
	if (m_EquirectImageLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		m_Device.TransitionImageLayout(inputImage, m_EquirectFormat, 
			m_EquirectImageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_EquirectMipLevels);
	VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = outputCubeMapImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = m_FACE_COUNT;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	// 1. Descriptor Set Layout
	std::unique_ptr<DescriptorSetLayout> pDescriptorLayout = std::make_unique<DescriptorSetLayout>(m_Device);
	pDescriptorLayout
		->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		->Create();

	// 2. Pipeline Layout (push constant = view/proj -> its kinda just more performant and ezpez)
	VkPushConstantRange pushRange{};
	pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushRange.offset = 0;
	pushRange.size = sizeof(glm::mat4) * 2;

	// 3. Descriptor Pool + Set
	std::unique_ptr<DescriptorPool> pDescriptorPool = std::make_unique<DescriptorPool>(m_Device);
	pDescriptorPool
		->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
		->Create(1);
	std::unique_ptr<DescriptorSet> pDescriptorSet = std::make_unique<DescriptorSet>(m_Device, *pDescriptorLayout, *pDescriptorPool, 1);
	pDescriptorSet
		->AddImageWrite(0, { inputSampler, inputImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL })
		->UpdateByIdx(0);

	// 4. Pipeline
	Pipeline::PipelineInfo pipelineInfo{};
	pipelineInfo.SetDefault();
	pipelineInfo.colorAttachments = { m_EquirectFormat };
	pipelineInfo.vertexBindingDescriptions = {};
	pipelineInfo.vertexAttributeDescriptions = {};
	pipelineInfo.pushConstantRanges = pushRange;
	pipelineInfo.rasterizer.cullMode = VK_CULL_MODE_NONE;
	pipelineInfo.CreatePipelineLayout(m_Device, { pDescriptorLayout->GetDescriptorSetLayout() });
	std::unique_ptr<Pipeline> pPipeline = std::make_unique<Pipeline>(
		m_Device,
		vertPath,
		fragPath,
		pipelineInfo
	);


	// 5. RENDER
	//-----------------

	for (uint32_t face = 0; face < m_FACE_COUNT; ++face)
	{
		VkImageView imageView = outputCubeMapImageViews[face][0];

		// Transition layout -> cant call func cus ye commandbuffers and shit
		{
			VkImageSubresourceRange range{
				VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, face, 1
			};

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.image = outputCubeMapImage;
			barrier.subresourceRange = range;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier
			);
		}

		VkImageMemoryBarrier samplerBarrier{};
		samplerBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		samplerBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		samplerBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		samplerBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		samplerBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		samplerBarrier.image = inputImage;
		samplerBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1 };
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &samplerBarrier
		);

		// Set up dynamic rendering
		VkRenderingAttachmentInfoKHR colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachment.imageView = imageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue.color = { 0.f, 0.f, 0.f, 1.f };

		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea.extent = extent;
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);
		DebugLabel::Begin(commandBuffer, "HDRI Capture Face: " + std::to_string(face), glm::vec4(0.1f, 0.3f, 0.75f, 1));

		pPipeline->Bind(commandBuffer);

		// viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		pDescriptorSet->Bind(commandBuffer, pPipeline->GetPipelineLayout(), 0, 0);

		glm::mat4 view = m_CAPTURE_VIEWS[face];
		vkCmdPushConstants(commandBuffer, pPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &view);
		vkCmdPushConstants(commandBuffer, pPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(glm::mat4), &m_CAPTURE_PROJECTION);

		vkCmdDraw(commandBuffer, 36, 1, 0, 0);

		vkCmdEndRenderingKHR(commandBuffer);
		DebugLabel::End(commandBuffer);
	}

	// TRAMSITION TO SHADER READ ONLY
	VkImageMemoryBarrier finalBarrier{};
	finalBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	finalBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	finalBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	finalBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	finalBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	finalBarrier.image = outputCubeMapImage;
	finalBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, m_FACE_COUNT };

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &finalBarrier
	);
	m_Device.EndSingleTimeCommands(commandBuffer);
}


void cat::HDRImage::CreateIrradianceMap()
{
	uint32_t irradianceMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_IrradianceMapExtent.width, m_IrradianceMapExtent.height)))) + 1;
	irradianceMipLevels = 1;

	// 1. Create image
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_IrradianceMapExtent.width;
		imageInfo.extent.height = m_IrradianceMapExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = irradianceMipLevels;
		imageInfo.arrayLayers = m_FACE_COUNT;
		imageInfo.format = m_EquirectFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		| VK_IMAGE_USAGE_SAMPLED_BIT
		| VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (vmaCreateImage(m_Device.GetAllocator(), &imageInfo, &allocInfo,
			&m_IrradianceMapImage, &m_IrradianceMapAllocation, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create irradiance image!");
		}
	}

	// 2. Create view
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_IrradianceMapImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = m_EquirectFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = irradianceMipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = m_FACE_COUNT;

		if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_IrradianceMapImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create irradiance cubemap view!");
		}
	}
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_IrradianceMapSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create irradiance cubemap sampler!");
		}
	}

	// 3. Create face views
	{
		for (uint32_t face = 0 ; face<m_FACE_COUNT;++face)
		{
			VkImageViewCreateInfo faceViewInfo{};
			faceViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			faceViewInfo.image = m_IrradianceMapImage;
			faceViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			faceViewInfo.format = m_EquirectFormat;
			faceViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			faceViewInfo.subresourceRange.baseMipLevel = 0;
			faceViewInfo.subresourceRange.levelCount = irradianceMipLevels;
			faceViewInfo.subresourceRange.baseArrayLayer = face;
			faceViewInfo.subresourceRange.layerCount = 1;
			VkImageView view;
			if (vkCreateImageView(m_Device.GetDevice(), &faceViewInfo, nullptr, &view) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create irradiance cubemap face view!");
			}
			m_IrradianceMapFaceViews[face] = view; 
		}
	}

	// 4. Render to irradiance map
	{
		std::array<std::vector<VkImageView>, 6> faceViews;
		for (int i = 0; i < 6; ++i)
			faceViews[i].push_back(m_IrradianceMapFaceViews[i]);

		RenderToCubeMap(m_IrradianceMapExtent, irradianceMipLevels,
		                m_CubeVertPath, m_IBLFragPath, m_CubeMapImage,
		                m_CubeMapImageView, m_EquirectSampler, m_IrradianceMapImage, faceViews
		);

		//GenerateMipmaps(m_IrradianceMapImage, m_EquirectFormat,
		//	m_IrradianceMapExtent.width, m_IrradianceMapExtent.height,
		//	irradianceMipLevels, m_FACE_COUNT);

		DebugLabel::NameImage(m_IrradianceMapImage, "HDRI Irradiance Map: " + std::to_string(m_IrradianceMapExtent.width) + "x" + std::to_string(m_IrradianceMapExtent.height));
	}
}