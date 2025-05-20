#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb_image.h>
#include "Buffer.h"

namespace cat
{

	Image::Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkFilter filter)
		: m_Device(device), m_Image(VK_NULL_HANDLE), m_Allocation(VK_NULL_HANDLE),
		m_ImageView(VK_NULL_HANDLE), m_Format{ format }
	{

		CreateImage(width, height, 1, format, usage, memoryUsage);
		CreateTextureImageView();
		CreateTextureSampler(filter, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		m_Extent = VkExtent2D{ width, height };
	}

	Image::Image(Device& device, const std::string& filename, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkFilter filter)
		: m_Device(device), m_Image(VK_NULL_HANDLE), m_Allocation(VK_NULL_HANDLE), m_ImageView(VK_NULL_HANDLE), m_Format{ format }
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			std::cerr << "Failed to load texture image!" << std::endl;
			pixels = stbi_load("resources/TextureNotFound.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		}
		m_Extent = VkExtent2D{ static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		// Create a staging buffer
		Buffer stagingBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		//TODO: should prob actually use the VMA auto mapper, o well :p
		stagingBuffer.WriteToBuffer(pixels, imageSize);

		stbi_image_free(pixels);

		CreateImage(texWidth, texHeight, 0, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, memoryUsage);
		CreateTextureImageView();

		device.TransitionImageLayout(m_Image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0);
		device.CopyBufferToImage(stagingBuffer.GetBuffer(), m_Image, texWidth, texHeight);
		//generateMipmaps(format, texWidth, texHeight);
		// device.TransitionImageLayout(m_image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);

		CreateTextureSampler(filter, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	}

	Image::Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImage existingImage)
		: m_Device(device), m_Image(existingImage), m_Allocation(VK_NULL_HANDLE),
		m_ImageView(VK_NULL_HANDLE), m_Format(format)
	{
		CreateTextureImageView();
		m_Extent = VkExtent2D{ width, height };
		m_IsSwapchainImage = true; // Mark this image as a swapchain image
	}

	Image::~Image()
	{
		if (m_ImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(m_Device.GetDevice(), m_ImageView, nullptr);
		}
		if (!m_IsSwapchainImage) {

			if (m_Sampler != VK_NULL_HANDLE) {
				vkDestroySampler(m_Device.GetDevice(), m_Sampler, nullptr);
			}
			if (m_Image != VK_NULL_HANDLE) {
				vmaDestroyImage(m_Device.Allocator(), m_Image, m_Allocation);
			}
		}
	}

	void Image::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = m_ImageLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_Image;

		if (HasDepth())
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (HasStencil()) barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		m_ImageLayout = newLayout;
	}

	void Image::CreateImage(uint32_t width, uint32_t height, uint32_t miplevels, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = miplevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memoryUsage;

		if (vmaCreateImage(m_Device.Allocator(), &imageInfo, &allocInfo, &m_Image, &m_Allocation, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image with VMA!");
		}
	}

	void Image::CreateTextureSampler(const VkFilter filter, const VkSamplerAddressMode addressMode) {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = m_Device.properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;

		if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void Image::CreateTextureImageView() {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_Format;
		viewInfo.subresourceRange.aspectMask = getImageAspect(m_Format);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
	}

	VkImageAspectFlags Image::getImageAspect(VkFormat format) {
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

}