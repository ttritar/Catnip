#pragma once

#include "SwapChain.h"
#include "Device.h"


namespace cat
{
	class Image final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Image(Device& device, SwapChain& swapChain, char const* path);
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		// Methods
		//--------------------

		// Getters & Setters
		VkImage GetTextureImage()const { return m_TextureImage; }
		VkDeviceMemory GetTextureImageMemory()const { return  m_TextureImageMemory; }
		VkImageView GetTextureImageView()const { return  m_TextureImageView; }
		VkSampler GetTextureSampler()const { return  m_TextureSampler; }


	private:
		// Private Methods
		//--------------------
		void CreateTextureImage(char const* path);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
		void CreateTextureImageView();
		void CreateTextureSampler();


		// Private Members
		//--------------------
		Device& m_Device;
		SwapChain& m_SwapChain;

		const std::string m_Path;

		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
		VkSampler m_TextureSampler;
	};
}
