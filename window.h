#pragma once

#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* initWindow();
void terminateWindow(GLFWwindow* window);
