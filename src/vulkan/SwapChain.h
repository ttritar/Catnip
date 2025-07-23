#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <memory>
#include <GLFW/glfw3native.h>

#include "Device.h"
#include "../core/Window.h"


#include <vector>
#include "scene/Image.h"

namespace cat
{
	static const int MAX_FRAMES_IN_FLIGHT = 2;

	class SwapChain
	{
	public:
		// CTOR & DTOR
		//--------------------
		SwapChain(Device& device, GLFWwindow* window);
		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;



		// Methods
		//--------------------
		void RecreateSwapChain();

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;

		// Getters & Setters
		VkSwapchainKHR GetSwapChain() const { return m_SwapChain; }

		uint32_t GetImageCount()const { return m_ImageCount; }
		uint32_t& GetImageIndex() { return m_ImageIndex; }
		VkFormat* GetSwapChainImageFormat() { return &m_SwapChainImageFormat; }
		VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
		std::vector<Image*> GetSwapChainImages() const {
			// tis could be better
			std::vector<Image*> images;
			images.reserve(m_pSwapChainImages.size());
			for (const auto& image : m_pSwapChainImages) {
				images.push_back(image.get());
			}
			return images;
		}
		Image* GetSwapChainImage(size_t i) const { return m_pSwapChainImages[i].get(); }
		Image* GetDepthImage(int frameIndex) const { return m_pDepthImages[frameIndex]; }
		VkImageView GetSwapChainImageView(size_t i) const { return m_pSwapChainImages[i]->GetImageView(); }

		void SetFrameBufferResized(bool value) { m_FramebufferResized = value; }
		bool GetFrameBufferResized() const { return m_FramebufferResized; }
		VkFence* GetInFlightFences(uint16_t idx) { return &m_InFlightFences[idx]; }
		VkSemaphore GetImageAvailableSemaphores(uint16_t idx) const { return m_ImageAvailableSemaphores[idx]; }
		VkSemaphore GetRenderFinishedSemaphores(uint16_t idx) const { return m_RenderFinishedSemaphores[idx]; }
		VkFormat FindDepthFormat() const;

	private:
		// Private Methods
		//--------------------

		// Creators
		void CreateSwapChain();
		void CleanupSwapChain();
		void CreateDepthResources();
		void CreateSyncObjects();

		// Helpers
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkSurfaceFormatKHR GetSwapSurfaceFormat() const { return { m_SwapChainImageFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }; }
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
		                             VkFormatFeatureFlags features) const;


		// Private Members
		//--------------------
		bool m_FramebufferResized;
		VkSwapchainKHR m_SwapChain;
		uint32_t m_ImageCount ;
		uint32_t m_ImageIndex;

		std::vector<std::unique_ptr<Image>> m_pSwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector < VkSemaphore> m_ImageAvailableSemaphores;
		std::vector < VkSemaphore> m_RenderFinishedSemaphores;
		std::vector < VkFence> m_InFlightFences;

		std::vector<Image*> m_pDepthImages;
		VkFormat m_swapChainDepthFormat;


		Device& m_Device;
		GLFWwindow* m_Window;
	};
}
