#pragma once

#include "Window.h"


static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->SetFrameBufferResized(true);
}

void Window::Initialize(int width, int height, const char* title)
{
    // 1. Initialize the GLFW library.
    //-----
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // 2. Create the window.
    //-----
    m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferResizeCallback);
}

void Window::Destroy()
{
	glfwDestroyWindow(m_pWindow);
}
