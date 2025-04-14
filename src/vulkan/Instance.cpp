#pragma once
#include "Instance.h"

#include <stdexcept>
#include <iostream>


// INSTANCE
//----------------
void Instance::Create()
{
	// Validation layer
	if (m_EnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}


	// 1. Create appInfo struct (optional)
	//------------------------------------
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_AppName.c_str();
	appInfo.applicationVersion = (m_AppVersion);
	appInfo.pEngineName = m_EngineName.c_str();
	appInfo.engineVersion = m_EngineVersion;
	appInfo.apiVersion = m_ApiVersion;


	// 2. Create createInfo struct
	//-----------------------------
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Extension support
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
		m_Extensions.push_back(glfwExtensions[i]);


	const auto requiredExtensions = GetRequiredExtensions();
	m_Extensions.insert(m_Extensions.end(), requiredExtensions.begin(), requiredExtensions.end());

	// Message Callback
	createInfo.enabledExtensionCount = static_cast<uint32_t>(m_Extensions.size());
	createInfo.ppEnabledExtensionNames = m_Extensions.data();

	// Validation layers
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (m_EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}


	// 3. Create instance
	//--------------------

	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}


	SetupDebugMessenger();
}

void Instance::Destroy()
{
	vkDestroyInstance(m_Instance, nullptr);
}

void Instance::DestroyMessenger()
{
	if (m_EnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}
}


Instance& Instance::SetAppName(const char* name)
{
	this->m_AppName = name;
	return *this;
}

Instance& Instance::SetAppVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
	this->m_AppVersion = VK_MAKE_VERSION(major, minor, patch);
	return *this;
}

Instance& Instance::SetEngineName(const char* name)
{
	this->m_EngineName = name;
	return *this;
}

Instance& Instance::SetEngineVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
	this->m_EngineVersion = VK_MAKE_VERSION(major, minor, patch);
	return *this;
}

Instance& Instance::SetApiVersion(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch)
{
	this->m_ApiVersion = VK_MAKE_API_VERSION(variant, major, minor, patch);
	return *this;
}



Instance& Instance::AddExtension(const char* extensionName)
{
	this->m_Extensions.push_back(extensionName);
	return *this;
}

Instance& Instance::EnableValidationLayers(bool isEnabled)
{
	this->m_EnableValidationLayers = isEnabled;
	return *this;
}

Instance& Instance::SetValidationLayers(const std::vector<const char*>& layers)
{
	this->m_ValidationLayers = layers;
	return *this;
}




std::vector<const char*> Instance::GetRequiredExtensions()const
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (m_EnableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);    //debug messenger with a callback using the VK_EXT_debug_utils extension
	}

	return extensions;
}

bool Instance::CheckValidationLayerSupport()const
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);   // list all of the available layers

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : m_ValidationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

void Instance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;    //should always return this
}

void Instance::SetupDebugMessenger()
{
	if (!m_EnableValidationLayers) return;

	// 1. Fill in struct with details about the messenger and its callback
	//------------------------------------------------------------------
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	// 2. Create the extension object if it's available
	//-----------------------------------------------
	if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}