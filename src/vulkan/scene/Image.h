#pragma once

#include <string>

#include "../Device.h"

namespace cat
{
	class Image final
	{
	public:
		// CTOR & DTOR
		//--------------------
		explicit Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkFilter filter = VK_FILTER_LINEAR);
		Image(Device& device, const std::string& filename, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkFilter filter = VK_FILTER_LINEAR);

		//Used for swapchain only
		Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImage existingImage);
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		// Methods
		//--------------------
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

		// Getters & Setters
		VkImage GetImage()const { return m_Image; }
		VkImageView GetImageView()const { return  m_ImageView; }
		VkFormat GetFormat() const { return m_Format; }
		VkSampler GetSampler()const { return  m_Sampler; }
		VkExtent2D GetExtent() const { return m_Extent; }
		bool HasDepth() const
		{
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
		bool HasStencil() const
		{
			switch (m_Format)
			{
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
		void CreateImage(uint32_t width, uint32_t height, uint32_t miplevels, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
		void CreateTextureImageView();
		void CreateTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode);
		void GenerateMipmaps(VkFormat format, uint32_t width, uint32_t height) const;

		static VkImageAspectFlags GetImageAspect(VkFormat format);

		// Private Members
		//--------------------
		Device& m_Device;

		std::string m_Path;

		VkImage m_Image;
		VmaAllocation m_Allocation;
		VkImageView m_ImageView;
		VkSampler m_Sampler;
		uint32_t m_MipLevels{};

		VkFormat m_Format;
		VkImageLayout m_ImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkEvent m_ImageEvent{ VK_NULL_HANDLE };

		VkExtent2D m_Extent{ 0, 0 };

		bool m_IsSwapchainImage{ false };
	};
}
