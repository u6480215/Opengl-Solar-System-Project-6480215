#pragma once
#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include "utils.h"

// Declare extern variables that are accessed in input handling
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
extern float yaw;
extern float pitch;
extern float lastX;
extern float lastY;
extern bool firstMouse;
extern float fov;

// Function prototypes for input processing
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// Utility function to compile shaders
//unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
GLuint compileShader(GLenum type, const char* source);
GLuint createProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
void printControls();
#endif // INPUT_UTILS_H
