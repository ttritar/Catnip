#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Device.h"
#include "../core/Window.h"


#include <vector>

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
		VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
		VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
		const std::vector<VkImage>& GetSwapChainImages() const { return m_SwapChainImages; }
		const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_SwapChainImageViews; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkFramebuffer GetSwapChainFramebuffer() const { return m_SwapChainFramebuffers[m_ImageIndex]; }
		void SetFrameBufferResized(bool value) { m_FramebufferResized = value; }
		bool GetFrameBufferResized() const { return m_FramebufferResized; }
		VkFence* GetInFlightFences(uint16_t idx) { return &m_InFlightFences[idx]; }
		VkSemaphore GetImageAvailableSemaphores(uint16_t idx) const { return m_ImageAvailableSemaphores[idx]; }
		VkSemaphore GetRenderFinishedSemaphores(uint16_t idx) const { return m_RenderFinishedSemaphores[idx]; }

	private:
		// Private Methods
		//--------------------

		// Creators
		void CreateSwapChain();
		void CleanupSwapChain() const;
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDepthResources();
		void CreateFramebuffers();
		void CreateSyncObjects();

		// Helpers
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		VkFormat FindDepthFormat() const;
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
		                             VkFormatFeatureFlags features) const;


		// Private Members
		//--------------------
		bool m_FramebufferResized;
		VkSwapchainKHR m_SwapChain;
		uint32_t m_ImageCount ;
		uint32_t m_ImageIndex;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;
		VkRenderPass m_RenderPass;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		std::vector < VkSemaphore> m_ImageAvailableSemaphores;
		std::vector < VkSemaphore> m_RenderFinishedSemaphores;
		std::vector < VkFence> m_InFlightFences;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		Device& m_Device;
		GLFWwindow* m_Window;
	};
}
