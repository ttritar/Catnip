#include "Image.h"
#include "Buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <stdexcept>


namespace cat
{
	Image::Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkFilter filter)
		: m_Device(device), m_Image(VK_NULL_HANDLE), m_Allocation(VK_NULL_HANDLE),
		m_ImageView(VK_NULL_HANDLE), m_Format{ format }, m_MipLevels(1)
	{
		CreateImage(width, height, m_MipLevels, format, usage, memoryUsage);
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
		m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(max(texWidth, texHeight)))) + 1;
		m_Extent = VkExtent2D{ static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		// Create a staging buffer
		Buffer stagingBuffer(device, Buffer::BufferInfo{ imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY });

		stagingBuffer.WriteToBuffer(pixels, imageSize);

		stbi_image_free(pixels);

		CreateImage(texWidth, texHeight, m_MipLevels, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, memoryUsage);
		CreateTextureImageView();

		device.TransitionImageLayout(m_Image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
		device.CopyBufferToImage(stagingBuffer.GetBuffer(), m_Image, texWidth, texHeight);
		GenerateMipmaps(format, texWidth, texHeight);

		CreateTextureSampler(filter, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	}

	Image::Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImage existingImage)
		: m_Device(device), m_Image(existingImage), m_Allocation(VK_NULL_HANDLE),
		m_ImageView(VK_NULL_HANDLE), m_Format(format), m_MipLevels(1)
	{
		CreateTextureImageView();
		m_Extent = VkExtent2D{ width, height };
		m_IsSwapchainImage = true; // Mark this image as a swapchain image
	}

	Image::~Image()
	{
		if (m_ImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_Device.GetDevice(), m_ImageView, nullptr);
		}
		if (!m_IsSwapchainImage)
		{

			if (m_Sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(m_Device.GetDevice(), m_Sampler, nullptr);
			}
			if (m_Image != VK_NULL_HANDLE) 
			{
				vmaDestroyImage(m_Device.GetAllocator(), m_Image, m_Allocation);
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
		barrier.subresourceRange.levelCount = m_MipLevels;
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

	void Image::CreateImage(uint32_t width, uint32_t height, uint32_t miplevels, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
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

		if (vmaCreateImage(m_Device.GetAllocator(), &imageInfo, &allocInfo, &m_Image, &m_Allocation, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image with VMA!");
		}
	}

	void Image::CreateTextureSampler(const VkFilter filter, const VkSamplerAddressMode addressMode)
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

		if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void Image::CreateTextureImageView() {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_Format;
		viewInfo.subresourceRange.aspectMask = GetImageAspect(m_Format);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to create image view!");
		}
	}

	VkImageAspectFlags Image::GetImageAspect(VkFormat format)
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

	void Image::GenerateMipmaps(VkFormat format, uint32_t width, uint32_t height) const
	{
		const VkFormatProperties properties = m_Device.GetFormatProperties(format);
		VkImageAspectFlags aspect = GetImageAspect(format);

		if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
		{
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

		auto mipWidth = static_cast<int32_t>(width);
		auto mipHeight = static_cast<int32_t>(height);

		for (uint32_t i = 1; i < m_MipLevels; i++) 
		{
			// Transition previous mip level to SRC_OPTIMAL
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = m_Image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = aspect;
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);

			// Transition next mip level to DST_OPTIMAL
			barrier.subresourceRange.baseMipLevel = i;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);

			// Blit
			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = {
				mipWidth > 1 ? mipWidth / 2 : 1,
				mipHeight > 1 ? mipHeight / 2 : 1,
				1
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			blit.srcSubresource.aspectMask = aspect;
			blit.dstSubresource.aspectMask = aspect;

			vkCmdBlitImage(commandBuffer,
				m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			// Transition previous level to SHADER_READ_ONLY_OPTIMAL
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);

			mipWidth = max(mipWidth / 2, 1);
			mipHeight = max(mipHeight / 2, 1);
		}
		//
		// // Transition last mip level to SHADER_READ_ONLY_OPTIMAL
		VkImageMemoryBarrier lastBarrier{};
		lastBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		lastBarrier.image = m_Image;
		lastBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		lastBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		lastBarrier.subresourceRange.aspectMask = aspect;
		lastBarrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
		lastBarrier.subresourceRange.levelCount = 1;
		lastBarrier.subresourceRange.baseArrayLayer = 0;
		lastBarrier.subresourceRange.layerCount = 1;

		lastBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		lastBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		lastBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		lastBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &lastBarrier);

		// m_device.TransitionImageLayout(m_image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);
		m_Device.EndSingleTimeCommands(commandBuffer);
	}
}