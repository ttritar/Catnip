#include "SwapChain.h"

#include <algorithm>
#include <array>
#include <stdexcept>
namespace cat
{
	// CTOR & DTOR
	//--------------------
	SwapChain::SwapChain(Device& device, GLFWwindow* window)
		: m_Device{ device }, m_Window{ window }
	{
		CreateSwapChain();
		//CreateRenderPass();
		CreateDepthResources();
        //CreateFramebuffers();
        CreateSyncObjects();
	}

	SwapChain::~SwapChain()
	{
        CleanupSwapChain();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_Device.GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device.GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_Device.GetDevice(), m_InFlightFences[i], nullptr);
        }

	}



    void SwapChain::RecreateSwapChain()
    {
        // HANDLE MINIMIZATION
        int width = 0, height = 0;
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_Window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_Device.GetDevice());

        CleanupSwapChain();

        CreateSwapChain();
        CreateDepthResources();
    }

 
	// Creators
	//--------------------
    void SwapChain::CreateSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = m_Device.GetSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);


        m_ImageCount = swapChainSupport.capabilities.minImageCount;

        if (swapChainSupport.capabilities.maxImageCount > 0 && m_ImageCount > swapChainSupport.capabilities.maxImageCount)
        {
            m_ImageCount = swapChainSupport.capabilities.maxImageCount;
        }


        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Device.GetSurface();


        createInfo.minImageCount = m_ImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;



        cat::QueueFamilyIndices indices = m_Device.GetPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }


        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;


        if (vkCreateSwapchainKHR(m_Device.GetDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_Device.GetDevice(), m_SwapChain, &m_ImageCount, nullptr);
		std::vector<VkImage> swapChainImages(m_ImageCount);
        vkGetSwapchainImagesKHR(m_Device.GetDevice(), m_SwapChain, &m_ImageCount, swapChainImages.data());

        m_SwapChainExtent = extent;
        m_pSwapChainImages.clear();
		m_pSwapChainImages.resize(m_ImageCount);

        for (int i{}; i < m_ImageCount;i++)
        {
            auto myImg = std::make_unique< Image>(
                m_Device,
                m_SwapChainExtent.width,
                m_SwapChainExtent.height,
                surfaceFormat.format,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY,
                swapChainImages[i]);

            m_pSwapChainImages[i]=std::move(myImg);
        }

        m_SwapChainImageFormat = surfaceFormat.format;
    }

    void SwapChain::CleanupSwapChain()
    {
		m_pSwapChainImages.clear();

		for (size_t i = 0; i < m_pDepthImages.size(); i++)
		{
			delete m_pDepthImages[i];
			m_pDepthImages[i] = nullptr;
		}
        m_pDepthImages.clear();

        vkDestroySwapchainKHR(m_Device.GetDevice(), m_SwapChain, nullptr);
    }

    VkImageView SwapChain::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void SwapChain::CreateDepthResources()
    {
        m_pDepthImages.clear();
        m_swapChainDepthFormat = FindDepthFormat();

        for (size_t i = 0; i < m_ImageCount; i++) 
        {
            auto depthImage = new Image(
                m_Device,
                m_SwapChainExtent.width,
                m_SwapChainExtent.height,
                m_swapChainDepthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY);
            m_pDepthImages.push_back(std::move(depthImage));
        }
        
    }

    void SwapChain::CreateSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(cat::MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(cat::MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(cat::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO; //only required field

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //start signaled

        for (size_t i = 0; i < cat::MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(m_Device.GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_Device.GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_Device.GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    // Helpers
	//--------------------
    VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        //Let's go through the list and see if the preferred combination is available:
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)  //srgb is standard
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        //Now, let's look through the list to see if VK_PRESENT_MODE_MAILBOX_KHR is available:
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }


        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
    {
        if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(m_Window, &width, &height);

            VkExtent2D actualExtent =
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    VkFormat SwapChain::FindDepthFormat() const // select a format with a depth component that supports usage as depth attachment:
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    VkFormat SwapChain::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_Device.GetPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }


}
