#pragma once

#include "Application.h"


void Application::Run()
{
    m_pWindow = new Window();
    m_pWindow->Initialize(WIDTH, HEIGHT, "Vulkan");
    initVulkan();
    mainLoop();
    cleanup();
}