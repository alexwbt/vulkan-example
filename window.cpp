#include "window.h"

#include <cstdint>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW to not create an OpenGL context.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable resizing window.

    return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void terminateWindow(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
