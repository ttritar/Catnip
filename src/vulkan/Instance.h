#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string>
#include <vector>


//look up its address ourselves using vkGetInstanceProcAddr     (proxy func)
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

static void DestroyDebugUtilsMessengerEXT( //proxy func
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


class Instance final
{
public:
	Instance() = default;

	void Create();
	void Destroy();
	void DestroyMessenger();	
	VkInstance GetInstance() const { return m_Instance; }

	// SETTERS	
	//----------------

	// appInfo struct (optional)
	Instance& SetAppName(const char* name);
	Instance& SetAppVersion(uint32_t major, uint32_t minor, uint32_t patch);
	Instance& SetEngineName(const char* name);
	Instance& SetEngineVersion(uint32_t major, uint32_t minor, uint32_t patch);
	Instance& SetApiVersion(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch);

	// createInfo struct
	Instance& AddExtension(const char* extensionName);
	Instance& EnableValidationLayers(bool isEnabled);
	Instance& SetValidationLayers(const std::vector<const char*>& layers);


private:
	std::string m_AppName = "VulkanApp";
	uint32_t m_AppVersion = VK_MAKE_VERSION(1, 0, 0);
	std::string m_EngineName = "No Engine";
	uint32_t m_EngineVersion = VK_MAKE_VERSION(1, 0, 0);
	uint32_t m_ApiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> m_Extensions;
	bool m_EnableValidationLayers = false;
	std::vector<const char*> m_ValidationLayers =
	{
	    "VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> GetRequiredExtensions()const;
	bool CheckValidationLayerSupport()const;
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	void SetupDebugMessenger();

	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
};


