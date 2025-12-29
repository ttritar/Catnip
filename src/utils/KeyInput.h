#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

bool IsKeyPressedOnce(GLFWwindow* w, int key)
{
    static std::unordered_map<int, bool> last;
    bool cur = glfwGetKey(w, key) == GLFW_PRESS;
    bool pressed = cur && !last[key];
    last[key] = cur;
    return pressed;
}
