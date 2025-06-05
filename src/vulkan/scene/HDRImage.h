#pragma once

#include "Image.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "glm/ext/matrix_transform.hpp"

#include "../Descriptors.h"
#include "../Descriptors.h"

// std
#include <array>
#include <memory>



namespace cat
{
	class HDRImage final
	{
	public:
		// CTOR & DTOR
		//---------------------
		HDRImage(Device& device, const std::string& filename);
		~HDRImage();

		HDRImage(const HDRImage&) = delete;
		HDRImage& operator=(const HDRImage&) = delete;
		HDRImage(HDRImage&&) = delete;
		HDRImage& operator=(HDRImage&&) = delete;

		// Methods
		//---------------------

		// Getters & Setters
		const VkImage& GetEquirectImage() const { return m_EquirectImage; }
		const VkImageView& GetEquirectImageView() const { return m_EquirectImageView; }
		const VkSampler& GetEquirectSampler() const { return m_EquirectSampler; }

		const VkImage& GetCubeMapImage() const { return m_CubeMapImage; }
		const VkImageView& GetCubeMapImageView() const { return m_CubeMapImageView; }
		const VkSampler& GetCubeMapSampler() const { return m_CubeMapSampler; }
		const std::array<std::vector<VkImageView>, 6>& GetCubeMapFaceViews() const { return m_CubeMapFaceViews; }

		const VkImage& GetIrradianceMapImage() const { return m_IrradianceMapImage; }
		const VkImageView& GetIrradianceMapImageView() const { return m_IrradianceMapImageView; }
		const VkSampler& GetIrradianceMapSampler() const { return m_IrradianceMapSampler; }

	private:
		// Private methods
		//---------------------
		void CreateEquirectImage(uint32_t width, uint32_t height, uint32_t miplevels, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
		void CreateEquirectTextureImageView();
		void CreateEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode);
		void GenerateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLevels) const;

		static VkImageAspectFlags GetImageAspect(VkFormat format);

		void CreateCubeMap();
		void RenderToCubeMap(const VkExtent2D& extent, uint32_t mipLevels, const std::string& vertPath, const std::string& fragPath, VkImage&
		                     inputImage, const VkImageView& inputImageView, VkSampler
		                     inputSampler, VkImage& outputCubeMapImage, std::array<std::vector<VkImageView>, 6>& outputCubeMapImageViews);
		void CreateIrradianceMap();


		// Private members
		//---------------------
		Device& m_Device;
		
		static constexpr int m_FACE_COUNT = 6;
		// IMAGES
		VkImage m_EquirectImage;
			VmaAllocation m_EquirectAllocation;
			VkImageView m_EquirectImageView;
			VkSampler m_EquirectSampler = VK_NULL_HANDLE;
			uint32_t m_EquirectMipLevels{};
			VkFormat m_EquirectFormat = VK_FORMAT_UNDEFINED;
			VkExtent2D m_EquirectExtent{ 0, 0 };
			VkImageLayout m_EquirectImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		
		VkImage m_CubeMapImage;
			VmaAllocation m_CubeMapAllocation;
			VkImageView m_CubeMapImageView;
			VkSampler m_CubeMapSampler = VK_NULL_HANDLE;
			VkExtent2D m_CubeMapExtent{ 512, 512 };
			std::array<std::vector<VkImageView>, m_FACE_COUNT> m_CubeMapFaceViews;

		VkImage m_IrradianceMapImage;
			VmaAllocation m_IrradianceMapAllocation;
			VkImageView m_IrradianceMapImageView;
			VkSampler m_IrradianceMapSampler = VK_NULL_HANDLE;
			VkExtent2D m_IrradianceMapExtent{ 32, 32 };
			std::array<VkImageView, m_FACE_COUNT> m_IrradianceMapFaceViews;

		// CUBE
		const glm::vec3 m_EYE = glm::vec3(0.0f);
		const glm::mat4 m_CAPTURE_VIEWS[m_FACE_COUNT] =
		{
			glm::lookAt(m_EYE, m_EYE + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // +X
			glm::lookAt(m_EYE, m_EYE + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // -X
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // -Y
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), // +Y
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // +Z
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) // -Z
		};
		glm::mat4 m_CAPTURE_PROJECTION;

		const std::string m_CubeVertPath = "shaders/cube.vert.spv";
		const std::string m_SkyFragPath = "shaders/sky.frag.spv";
		const std::string m_IBLFragPath = "shaders/ibl.frag.spv";

	};
}
