#pragma once
#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>




void Application::Run()
{
    m_pWindow = new Window();
    m_pWindow->Initialize(WIDTH, HEIGHT, "Vulkan");
    initVulkan();
    mainLoop();
    cleanup();
}