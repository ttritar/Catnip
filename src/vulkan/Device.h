#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <optional>
#include <vulkan/vulkan.h>

#include <vma/vk_mem_alloc.h>


namespace cat
{

extern PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
extern PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;


// Validation layers
#ifdef NDEBUG 
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const std::vector<const char*> VALIDATION_LAYERS =
{
	"VK_LAYER_KHRONOS_validation"
};


// Device Extensions
const std::vector<const char*> DEVICE_EXTENSIONS =
{
	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


// Debug Messenger
static VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT( 
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}


// Structs
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}

};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};



// Forward Declarations
	class Buffer;

	class Device final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Device(GLFWwindow* window);
		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(Device&&) = delete;


		// Methods
		//--------------------
		VkCommandBuffer BeginSingleTimeCommands() const;
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, bool mappable, VkBuffer& buffer, VmaAllocation& allocation) const;
		void CopyBuffer(Buffer* srcBuffer, Buffer* destBuffer, VkDeviceSize size) const;

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)const;

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout& oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	

		// Getters & Setters
		VkDevice GetDevice() const { return m_Device; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkInstance GetInstance() const { return m_Instance; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }
		VkCommandPool GetCommandPool() const { return m_CommandPool; } 
		SwapChainSupportDetails GetSwapChainSupport()const { return QuerySwapChainSupport(m_PhysicalDevice); }
		QueueFamilyIndices GetPhysicalQueueFamilies()const { return FindQueueFamilies(m_PhysicalDevice); }
		VmaAllocator GetAllocator() const { return m_Allocator; }
		VkFormatProperties GetFormatProperties(VkFormat format) const;
		VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const { return m_PhysicalDeviceProperties; }

	private:
		// Private Methods
		//--------------------

		// Creators
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();
		void AllocVmaAllocator();

		// Helpers
		static bool CheckValidationLayerSupport();
		static std::vector<const char*> GetRequiredExtensions();
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		bool IsDeviceSuitable(VkPhysicalDevice device) const;
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
		static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

		// Private Members
		//--------------------
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
		VkSurfaceKHR m_Surface;
		VkCommandPool m_CommandPool;
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties{};

		VmaAllocator m_Allocator{};

		GLFWwindow* m_Window;
	};
}
