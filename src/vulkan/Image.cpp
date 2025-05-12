#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace cat
{

    Image::Image(Device& device, SwapChain& swapChain, char const* path)
        :m_Device(device), m_SwapChain(swapChain), m_Path(path)
    {
        CreateTextureImage(path);
        CreateTextureImageView();
        CreateTextureSampler();
    }

    Image::~Image()
    {
        vkDestroySampler(m_Device.GetDevice(), m_TextureSampler, nullptr);
        vkDestroyImageView(m_Device.GetDevice(), m_TextureImageView, nullptr);

        vkDestroyImage(m_Device.GetDevice(), m_TextureImage, nullptr);
        vkFreeMemory(m_Device.GetDevice(), m_TextureImageMemory, nullptr);
    }

    void Image::CreateTextureImage(char const* path)
    {
        // LOADING IMAGE
        int texWidth;
        int texHeight;
    	int texChannels;

        stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;  // 4 bytes per pixel

        if (!pixels)
        {
            pixels = stbi_load("resources/textureNotFound.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            imageSize = texWidth * texHeight * 4;  // 4 bytes per pixel
        }


        // CREATE STAGING BUFFER
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_Device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);


        // MEMORY MAPPING
        void* data;
        vkMapMemory(m_Device.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_Device.GetDevice(), stagingBufferMemory);

        stbi_image_free(pixels);


        // CREATE IMAGE
        m_SwapChain.CreateImage(texWidth, texHeight,
            VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);


        // TRANSITION IMAGE LAYOUT
        TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, m_TextureImage,
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


        // CLEANUP
        vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.GetDevice(), stagingBufferMemory, nullptr);
    }

    void Image::TransitionImageLayout(VkImage image, VkFormat format,
        VkImageLayout oldLayout, VkImageLayout newLayout) const
    {
        VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // to transfer queue family ownership
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,       // which pipeline stage the operations occur that should happen before the barrier   +   pipeline stage in which operations will wait on the barrier  
            0,                  // either 0 or VK_DEPENDENCY_BY_REGION_BIT. The latter turns the barrier into a per-region condition.
            0, nullptr,         // memory barriers
            0, nullptr,         // buffer memory barriers
            1, &barrier         // image memory barriers
        );

        m_Device.EndSingleTimeCommands(commandBuffer);
    }

    void Image::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const
    {
        VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();


        VkBufferImageCopy region{};
        region.bufferOffset = 0;    // specifies the byte offset in the buffer at which the pixel values start
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;   // < + ^  fields specify how the pixels are laid out in memory

        // which part of the image we want to copy the pixels
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        // which part of the image we want to copy the pixels
        region.imageOffset = { 0,0,0 };
        region.imageExtent = { width,height,1 };


        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   // which layout the image is currently using (assuming that the image has already been transitioned to the layout that is optimal for copying pixels to)
            1,
            &region
        );

        m_Device.EndSingleTimeCommands(commandBuffer);
    }

    void  Image::CreateTextureImageView()
    {
        m_TextureImageView = m_SwapChain.CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void  Image::CreateTextureSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;   // <^ how to interpolate texels that are magnified or minified

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // x
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // y
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // z

        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_Device.GetPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // color of the border if the texture is sampled outside of the image
        samplerInfo.unnormalizedCoordinates = VK_FALSE; // if true, the coordinates are not normalized to the range [0,1]

        samplerInfo.compareEnable = VK_FALSE; // if true, the sampler will compare the sampled texel with a reference value
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // if compareEnable is true, this specifies the comparison operation to use

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // how to sample mipmap levels
        samplerInfo.mipLodBias = 0.0f; // optional - used to bias the mipmap level used when sampling
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;


        if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

}