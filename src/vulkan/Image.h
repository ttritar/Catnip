#pragma once

#include "Device.h"

namespace cat
{
	class Image final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Image(Device& device, const std::string& path);
		Image(Device& device, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage,
		      VkImageAspectFlags aspectFlags);

		Image(Device& device, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage,
			VkImageAspectFlags aspectFlags, VkImage image);
;		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		// Methods
		//--------------------
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

		// Getters & Setters
		VkImage GetImage()const { return m_Image; }
		VkDeviceMemory GetImageMemory()const { return  m_ImageMemory; }
		VkImageView GetImageView()const { return  m_ImageView; }
		VkSampler GetSampler()const { return  m_Sampler; }
		bool HasDepth() const {
			switch (m_Format)
			{
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_X8_D24_UNORM_PACK32:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return true;
			default:
				return false;
			}
		}

	private:
		// Private Methods
		//--------------------
		void CreateTextureImage(const std::string& path);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
		void CreateTextureImageView();
		void CreateTextureSampler();


		// Private Members
		//--------------------
		Device& m_Device;

		std::string m_Path;

		VkImage m_Image;
		VkDeviceMemory m_ImageMemory;
		VkImageView m_ImageView;
		VkSampler m_Sampler;

		VkFormat m_Format;
		VkImageLayout m_ImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
	};
}
