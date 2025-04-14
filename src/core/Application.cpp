#pragma once
#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>




void Application::Run()
{
    m_Window.Initialize(WIDTH, HEIGHT, "Vulkan");
    initVulkan();
    mainLoop();
    cleanup();
}